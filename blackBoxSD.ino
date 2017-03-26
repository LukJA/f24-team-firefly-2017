// SD card definitions
// black box definitions

void blackBoxInit(){
  // if sd fails deacivate writing
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    SDEN = 0;
    return;
  }
  // open file
  BlackBox = SD.open("test.txt", FILE_WRITE);
  // if it didnt open deactivate writing
  if (!BlackBox){
    SDEN = 0;
    return;
  }
  // else return active
  SDEN = 1;
}

