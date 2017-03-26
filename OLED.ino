// send the list of data over the port
// RPM SPEED V1 V2 AT W#(/10) T1 T2 T3 IG THROT(128) WARN(4)
// eg 1337 33 125 126 20 24 26 27 28 1 100 HELP

void OLEDData(int rpm,int spd,int v1,int v2,int at,int wt,int t1,int t2,int t3,int throt,int ignit,char text[5]){
  int list[11] = {rpm,spd,v1,v2,at,wt,t1,t2,t3,throt,ignit};
  for (int i = 0; i <11; i++){
	  
	// fix digit widths
    if (i == 0){ // 4 digit wide
      if (list[i] < 10){
        OLEDSer.print("000");
      }
      else if (list[i] < 100){
        OLEDSer.print("00");
      }
      else if (list[i] < 1000){
        OLEDSer.print("0");
      }
    }
    if (i == 2 || i == 3 || i == 10){ // 3 digits wide
      if (list[i] < 10){
        OLEDSer.print("00");
      }
      else if (list[i] < 100){
        OLEDSer.print("0");
      }
    }
    if (i == 1 || i == 4 || i == 5 || i == 6 || i == 7 || i == 8){ // 2 digits wide
      if (list[i] < 10){
        OLEDSer.print("0");
      }
    }
	
    OLEDSer.print(list[i]);
    OLEDSer.print(" ");
    //Serial.print(list[i]);
    //Serial.print(" ");
  }
  OLEDSer.print(text);
  OLEDSer.println("");
  //Serial.print(text);
  //Serial.println("");
}


