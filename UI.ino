// UI element handling
// register definitions
#define NVIC_AIRCR_VECTKEY		(0x5FA << 16)
#define NVIC_SYSRESETREQ		2

long int resetPress = 0;;

void ignitionISR(){
  cli();
  ignition = digitalRead(ignitIn);
  sei();
}

void masterISR(){
  cli();
  master = digitalRead(masterIn);
  sei();
}

void rsetISR(){
  cli();
  delay(10);
  Serial.println("ISR");
  if (!digitalRead(resetIn)){
    resetPress = millis();
    Serial.println("A");
  } else if ((millis() - resetPress) > 3000){ // pressed for 3 seconds
    Serial.println("B");
	  // write vectkey and sysreset
    // SCB_AIRCR = (NVIC_AIRCR_VECTKEY | (SCB_AIRCR & (0x700)) | (1 << NVIC_SYSRESETREQ)); // write vectkey to gain access
	  //wait 
	  //while(1);
  } else {
    // set the cleared warning
    Serial.println("C");
    warningCleared = warning;
    warning = "    "; // -------------- what
    Serial.println(warningCleared);
    delay(10);
  }
  
  sei();
}

void hornISR(){
  cli();
  if(!digitalRead(hornIn)){
	  digitalWrite(hornOut, HIGH);
  }
  else {
	  digitalWrite(hornOut, LOW);
  }
  sei();
}

