// Photo interupt routines
// vars

volatile long int lastMotor,lastAxle;
const float distancePerRot = 20 * 2.54 * 3.14 / 100; // meters

// direct motor interupt, once per revolution
void motorPhotoI(){
  cli();
  // calculate rotation interval in milliseconds
  long int interV = millis() - lastMotor;
  lastMotor = millis(); 

  // calculate and set motor RPM
  // globally set in f24.ino
  motorRpm = 60000 / interV;
  sei();
}

// output shaft interupt, once per revolution
void dAxlePhotoI(){
  cli();
  // calculate rotation interval in milliseconds
  long int interV = millis() - lastAxle;
  lastAxle = millis();

  // calculate velocity
  axlespeedKPH = distancePerRot * (360000 / interV) / 1000;
  sei();
}

