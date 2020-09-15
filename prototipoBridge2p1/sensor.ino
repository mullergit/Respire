void sendRequest(byte packet[],int arrayLen){//, int respLen){  
   while(!softSerial.available()){ 
     softSerial.write(packet,arrayLen);
     delay(50);
     delay(1000);
   }
   int timeout=0; 
   while(softSerial.available()){// < respLen){  
     timeout++;
     if(timeout > 10) 
     Serial.print("Timeout");
     {
     while(softSerial.available()) 
     Serial.print(softSerial.read(),HEX);
         
     break; 
     }
     delay(50);
   }
  /*for (int i=0; i < respLen; i++) response[i] = softSerial.read();
  Serial.print("\t Sensor response:   ");
   for(int i=0;i<respLen;i++){
    Serial.print(response[i],HEX);
    Serial.print(" ");
   }*/
}
void kimo(){
  int n=50;
      float rawA3=0;
      vkimo = 0;
      for(int i = 0;i<n;i++){ 
          //tkimo = tkimo + analogRead(TempKimoPin);
          rawA3 = rawA3 + analogRead(A0);
          //delay(2);
      } 
      
      rawA3 = rawA3/n;      
      vkimo = (float) (rawA3*5.0)/1024.0;
      Serial.print("\nvkimo ");
      Serial.println(vkimo );
}

void readHPMA(){
  Serial.print("readHPMA ");
  softSerial.flush();
  /*while(!softSerial.available()){ 
      softSerial.write(readPM,sizeof(readPM));
      delay(50);
      //delay(500);
  }*/
  //softSerial.flush();
  
  if(softSerial.available()){
    int i = 0;
    while (softSerial.available() > 0) {
      response[i] = softSerial.read();
      Serial.print(response[i],HEX);
      Serial.print(" ");
      i++;
    }
    Serial.println(" ");
  }
  Serial.print("readHPMA2 ");  
  for(int x=3;x<11;x++){
    Serial.print(response[x],HEX);
      Serial.print(" ");
  }
  if(response[0]==0x42 & response[1]==0x4d){
      Serial.print("readHPMA3 ");
    Serial.println(" ");
    pm1 = (int)response[4]*256+response[5];
    pm25 = response[6]*256+response[7];
    pm4 = response[8]*256+response[9];
    pm10 = response[10]*256+response[11];
    Serial.println("\tHPMA");
    Serial.print(" PM1: ");
    Serial.print(pm1);
    Serial.print("\tPM2.5: ");
    Serial.print(pm25);
    Serial.print("\tPM4: ");
    Serial.print(pm4);
    Serial.print("\tPM10: ");
    Serial.print(pm10);
    Serial.println();
    
  }else{
    Serial.println("\n\tleitura incorreta HPMA ");
  }
  
  
}
/*
void readSPS(){
  Wire.begin(D2,D1);
  do {
    ret = sps30_read_data_ready(&data_ready);
    if (ret < 0) {
      Serial.print("error reading data-ready flag: ");
      Serial.println(ret);
    } else if (!data_ready)
      Serial.print("data not ready, no new measurement available\n");
    else
      break;
    delay(100); 
  } while (1);

  ret = sps30_read_measurement(&m);
  if (ret < 0) {
    Serial.print("error reading measurement\n");
  } else {

    Serial.println("\tSensirion SPS30 Particulate Matter Sensor\n");
    Serial.print("\tMass concentration\n");
    Serial.print("PM 0.3 to 1.0 um: ");
    Serial.print(m.mc_1p0);Serial.print(" ug/m³\n");
    Serial.print("PM 0.3 to 2.5 um: ");
    Serial.print(m.mc_2p5);Serial.print(" ug/m³\n");
    Serial.print("PM 0.3 to 4.0 um: ");
    Serial.print(m.mc_4p0);Serial.print(" ug/m³\n");
    Serial.print("PM 0.3 to 10.0 um: ");
    Serial.print(m.mc_10p0);Serial.print(" ug/m³\n");


    Serial.print("\n\tTotal particles concentration\n");
    Serial.print("NC  0.3 to 0.5 um: ");
    Serial.print(m.nc_0p5);Serial.print(" particles/cm³\n");
    Serial.print("NC  0.3 to 1.0 um: ");
    Serial.print(m.nc_1p0);Serial.print(" particles/cm³\n");
    Serial.print("NC  0.3 to 2.5 um: ");
    Serial.print(m.nc_2p5);Serial.print(" particles/cm³\n");
    Serial.print("NC  0.3 to 4.0 um: ");
    Serial.print(m.nc_4p0);Serial.print(" particles/cm³\n");
    Serial.print("NC  0.3 to 10.0 um: ");
    Serial.print(m.nc_10p0);Serial.print(" particles/cm³\n");
    
    Serial.print("\n\tParticles concentration per size\n");
    Serial.print("NC  0.3 to 0.5 um: ");
    Serial.print(m.nc_0p5);Serial.print(" particles/cm³\n");
    Serial.print("NC  0.5 to 1.0 um: ");
    Serial.print(m.nc_1p0 -m.nc_0p5);Serial.print(" particles/cm³\n");
    Serial.print("NC  1.0 to 2.5 um: ");
    Serial.print(m.nc_2p5-m.nc_1p0);Serial.print(" particles/cm³\n");
    Serial.print("NC  2.5 to 4.0 um: ");
    Serial.print(m.nc_4p0-m.nc_2p5);Serial.print(" particles/cm³\n");
    Serial.print("NC  4.0 to 10.0 um: ");
    Serial.print(m.nc_10p0-m.nc_4p0);Serial.print(" particles/cm³\n");
    
    Serial.print("\nTypical partical size: ");
    Serial.println(m.typical_particle_size);
    Serial.println();
  } 
}
void readSCD(){
  if(true){//flagSCD1){
    Serial.print("readSCD ");
    Wire.begin(D2,D1);
    //Wire.begin(D0,D4);//D4,D8);
    
        if (scd1.dataAvailable()){
              scd1temp = scd1.getTemperature();
              scd1hum = scd1.getHumidity();
              scd1co2 = scd1.getCO2();
              
              Serial.println("SCD1.....");
              Serial.print("co2(ppm):");Serial.print(scd1co2);    
              Serial.print(" temp(C):");Serial.print(scd1temp, 1);    
              Serial.print(" humidity(%):");Serial.print(scd1hum, 1);    
              Serial.println();
              Serial.println("scd1.getTemperatureOffset()");
          Serial.println(scd1.getTemperatureOffset());
            }
            else
              Serial.println("Waiting for new data Wire");    
        
      
    }
    
       
}*/
void readT66(){
  Serial.print("\n readSTATUS()...");
   t66.readSTATUS();
   
   if(t66.response[3]==0){
      Serial.print("\n T66 CO2 ppm:");
      t66.readPPM();
      t66co2 = t66.getPPMorSGPT(t66.response);
      Serial.println(t66co2);
   }
}
