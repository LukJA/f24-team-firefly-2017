#include <OneWire.h>
#include <Metro.h>
#include <SD.h>
#include <SPI.h>
// -------------------------------------------
// F24 team firefly main ECU control

// OLED dash on SERIAL3 (7,8)
// Accel on WIRE2 (3,4)
// telem on SPI0  (9-12)

// define pins
// OUT ----

#define OLEDSer   Serial3
#define ignitOut  23
#define buzzOut   22
#define hornOut   21

#define DAC0      A21
#define DAC1      A22

// IN ----

#define battVoltTIn 19
#define battVoltLIn 18
#define battAmpsAIn 17  // shunt

#define motorPI   29
#define dAxlePI   30

#define tempsIn  28

#define ignitIn   26  
#define masterIn  25
#define resetIn   24
#define hornIn    27

#define throttleIn 20
#define Toffset   2048 // throttle bottom out

// variables
// -- global

// photo interupt set
volatile int motorRpm, axlespeedKPH = 0; // shunted

// toggle switchs
volatile bool master = 0;
volatile bool ignition = 0;
volatile bool horn = 0;

// ds18b20 set
volatile int batTemp, motTemp, escTemp = 0; // stored as shunt

// battery stats
volatile float batVoltLower, batVoltTotal = 125; // stored as true
volatile int batCurrent = 0; // shunted to whole number

// throttle
volatile int throttle = 0;

String warning = "    ";
String warningCleared = "    ";

// -- system
elapsedMillis sinceBoot;

// intervals
Metro X1millis = Metro(1);  // Instantiate an instance
Metro X10millis = Metro(10);  // Instantiate an instance
Metro X100millis = Metro(100);  // Instantiate an instance
Metro X500millis = Metro(500);  // Instantiate an instance

// black box SD init
File BlackBox;
volatile bool SDEN = 0;
// Teensy 3.5 & 3.6 on-board: BUILTIN_SDCARD
const int chipSelect = BUILTIN_SDCARD; 

// temp sense
OneWire ds18b20(tempsIn); //create instance
byte addr[8];
byte data[12];
const byte t1Addr[8] = {0x28, 0x2B, 0x8A, 0x63, 0x08, 0x00, 0x00, 0x1B}; //motor
const byte t2Addr[8] = {0x28, 0x34, 0xF6, 0x64, 0x08, 0x00, 0x00, 0xAE}; //esc
const byte t3Addr[8] = {0x28, 0xFE, 0x0A, 0x64, 0x08, 0x00, 0x00, 0x79}; //bat

void setup() {
  // prevent interupting setup
  noInterrupts();

  Serial.begin(9600); // debug serial

  // led
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  // set outputs
  pinMode(ignitOut, OUTPUT);
  pinMode(buzzOut, OUTPUT);
  pinMode(hornOut, OUTPUT);
  
  // PhotoInterupts handler attachment
  // all methods are handled in "photoInter.c"
  pinMode(motorPI, INPUT);
  attachInterrupt(motorPI, motorPhotoI, RISING);
  pinMode(dAxlePI, INPUT);
  attachInterrupt(dAxlePI, dAxlePhotoI, RISING);

  // UI interupts
  // methods handled in UI
  pinMode(ignitIn, INPUT_PULLDOWN);
  attachInterrupt(ignitIn, ignitionISR, CHANGE);
  pinMode(masterIn, INPUT_PULLDOWN);
  attachInterrupt(masterIn, masterISR, CHANGE);
  pinMode(resetIn, INPUT_PULLDOWN);
  //attachInterrupt(resetIn, rsetISR, CHANGE);
  pinMode(hornIn, INPUT_PULLDOWN);
  attachInterrupt(hornIn, hornISR, CHANGE);

  // Enable 13bit ADC
  pinMode(throttleIn, INPUT_PULLDOWN);
  pinMode(battVoltTIn, INPUT);
  pinMode(battVoltLIn, INPUT);
  //analogReadResolution(13);

  // Enable 12bit DAC
  pinMode(DAC1, OUTPUT);
  pinMode(DAC0, OUTPUT);
  analogWriteResolution(12);  
  
  // Write 1V to DAC1 (1-4v)
  analogWrite(DAC1, 1241); // 1V

  // Setup OLED Serial Display
  OLEDSer.begin(115200); // begin serial control of OLED

  // Setup Temperature Sensors (request a convert and set settings)
  //tempsBegin();
  delay(100); // allow first conversion
  // activate interupts
  interrupts();
}

void loop() {
  // runs a certain routine every X milliseconds through metro library
  //every 1    - 
  //every 10   - read and write throttle, ignition
  //every 100  - send data to oled, 
  //every 500 - read temps + request next, motor current, motor voltage into vars, update boot time, check warnings
  
  digitalWrite(13, HIGH); // status is GO

  if (!master){
    ignition = 0;  // deactive motor controls 
    throttle = 0;
    Serial.println(master); // notify debug
    digitalWrite(13, LOW);  // turn of status led
	
    delay(200); //wait a bit to give OLED time to catch-up
	  OLEDSer.println("M");
    // system disbaled if master is deactivated
    // Disable interupts??? reset wont work...
  }
  while(!master){
	  // wait until switch is flicked, handled in interupt
  }
  
  if (X1millis.check()){ 
	// nothing yet - may remove
  }
  
  if (X10millis.check()){
    // read throttle (13bit)
    throttle = (analogRead(throttleIn));
    Serial.println(throttle);
    throttle = map(throttle, 30, 300, 0, 4096);
    analogWrite(DAC0, throttle); // write throttle, its 13bit and output is 12 so shift over one

    //write ignition 
    if (ignition){
      digitalWrite(ignitOut, HIGH);
    }
    else{
      digitalWrite(ignitOut, LOW);
    }
	
  }
  
  if (X100millis.check()){
    // send OLED data - in OLED.ino
    // RPM SPEED V1 V2 AT W#(/10) T1 T2 T3 IG THROT(128) WARN(4)
    // eg 1337 33 125 126 20 24 26 27 28 1 100 HELP
    
    // convert warning
    char warn[10];
    warning.toCharArray(warn, 10);
    
    //OLEDData(1337, 33, 125, 126, 20, 24, 26, 27, 28, 1, 100, "HELP");
    int draw = map(throttle, 0, 4096, 0, 128);
    OLEDData(motorRpm, axlespeedKPH, 127, 126, batCurrent, batCurrent*batVoltTotal/10, batTemp, motTemp, escTemp, ignition, draw, warn);
    
    //reset the alarm
    digitalWrite(buzzOut, LOW);
  }
  
  if (X500millis.check()){
    sinceBoot = millis(); // update boot time
    Serial.println(sinceBoot);

    // voltages + warning
    batVoltLower = (analogRead(battVoltLIn) * 3.3 * 5) / 1023; // divider is 1/5 ref is 3.3
    batVoltTotal = (analogRead(battVoltTIn) * 3.3 * 11) / 1023; // divider is 1/11 ref is 3.3
    //Serial.print(batVoltLower);
    //Serial.print(batVoltTotal);
    /*
    if ((batVoltLower < 10.6 || batVoltTotal-batVoltLower < 10.6) && (warningCleared != "VOLT")){
      // set the warning
      digitalWrite(buzzOut, HIGH);
      warning = "VOLT";
    }*/

    // current -- 0.00075 ohm resistor I = v/r
    // calculates current in A and shunts it to int
    batCurrent = (int)(analogRead(battAmpsAIn) / 8192 * 3.3 /0.00075);
    /*
    if ((batCurrent > 50) && (warningCleared != "AMPS")){
      // set the warning
      digitalWrite(buzzOut, HIGH);
      warning = "AMPS";
    }*/

    // temps
    //tempsRead(); // read current (auto updates vars)
    //tempsRequest(); // request a set for the next loop round  
	  /*
    if ((batTemp > 40 || motTemp > 60 || escTemp > 60) && (warningCleared != "TEMP")){
      // set the warning
      digitalWrite(buzzOut, HIGH);
      warning = "TEMP";
    }*/
    
  }
}
