// temperature sensor functions

void tempsBegin(){
  ds18b20.reset_search(); // clear search  
  while (ds18b20.search(addr)) { // go though each valid address
    if ( addr[0] != 0x28) {
      Serial.print("Device is not DS18B20 family device.\n");
      break;
    }
    ds18b20.reset(); // reset one-wire
    ds18b20.select(addr); // select the current device
    ds18b20.write(0x4E);  // write on scratchPad to change config
    ds18b20.write(0x00);         // User byte 0 - Unused
    ds18b20.write(0x00);         // User byte 1 - Unused
    ds18b20.write(0x1F);         // set up en 9 bits (0x1F)
    
    ds18b20.reset(); // reset one-wire
    ds18b20.select(addr); // select the current device
    ds18b20.write(0x44); // start temp conversion
    
    // go back to complete for all addresses
    ds18b20.reset(); // reset one-wire
  }
  ds18b20.reset_search(); // clear search 
}

void tempsRequest(){
  ds18b20.reset_search(); // clear search  
  while (ds18b20.search(addr)) { // go though each valid address
    if ( addr[0] != 0x28) {
      Serial.print("Device is not DS18B20 family device.\n");
      break;
    }
    ds18b20.reset(); // reset one-wire
    ds18b20.select(addr); // select the current device
    ds18b20.write(0x44); // start temp conversion
    
    // go back to complete for all addresses
    ds18b20.reset(); // reset one-wire
  }
  ds18b20.reset_search(); // clear search 
}

void tempsRead(){
  ds18b20.reset_search(); // clear search  
  while (ds18b20.search(addr)) { // go though each valid address
    if ( addr[0] != 0x28) {
      Serial.print("Device is not DS18B20 family device.\n");
      break;
    }
    ds18b20.reset(); // reset one-wire
    ds18b20.select(addr); // select the current device
    ds18b20.write(0xBE);         // Read Scratchpad

    for (byte i = 0; i < 9; i++) {    // we need 9 bytes
      data[i] = ds18b20.read();;
    }

    int thisTemp = ((data[1] << 8) + data[0]) * 0.5 + 0.5; // 0.5 is 9 bit coefficient, +0.5 gives us true rounding
    
    // assign temp to its variable based on addr
    if (addr == t1Addr){ 
      motTemp = thisTemp; 
    }
    else if (addr == t2Addr){
      escTemp = thisTemp;
    }
    else if (addr == t3Addr){
      batTemp = thisTemp;
    }
    
    // go back to complete for all addresses
    ds18b20.reset(); // reset one-wire
  }
  ds18b20.reset_search(); // clear search 
}

