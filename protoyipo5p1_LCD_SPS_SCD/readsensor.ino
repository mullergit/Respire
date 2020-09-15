void readSPS(){
  //Wire.begin(D7,D8);
  /*ret = sps30_start_measurement();//sps30_stop_measurement();45-65mA --> ~330uA
  if (ret < 0) {
    Serial.print("error starting measurement\n");
  }else{
    Serial.print("measurements started\n");
  }
  delay(1000);*/
  
  do {
    ret = sps30_read_data_ready(&data_ready);
    if (ret < 0) {
      Serial.print("error reading data-ready flag: ");
      Serial.println(ret);
    } else if (!data_ready)
      Serial.print("data not ready, no new measurement available\n");
    else
      break;
    delay(100); /* retry in 100ms */
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
    /*
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
    */
    Serial.print("\nTypical partical size: ");
    Serial.println(m.typical_particle_size);
    Serial.println();
  } 
  /*ret = sps30_stop_measurement();
    if (ret < 0) {
    Serial.print("error stoping measurement\n");
  }else{
    Serial.print("measurements stoped\n");
  }*/
}
void readSCD(){
  //if(flagSCD1){
    //Wire.begin(D4,D3);//SDA, SCL
    if (scd1.dataAvailable()){
        scd1temp = scd1.getTemperature();
        scd1hum = scd1.getHumidity();
        scd1co2 = scd1.getCO2();
        tempoffset = scd1temp - 1.0;
        
        Serial.println("SCD1.....");
        Serial.print("co2(ppm):");Serial.print(scd1co2);    
        Serial.print(" temp(C):");Serial.print(scd1temp, 1); Serial.print(" tempoffset(C):");Serial.print(tempoffset, 1);   
        Serial.print(" humidity(%):");Serial.print(scd1hum, 1);    
        Serial.println();
        Serial.println("scd1.getTemperatureOffset()");
    Serial.println(scd1.getTemperatureOffset());
      }
      else
        Serial.println("Waiting for new data Wire");    
  //}  
}
void PM10fs4408(){
  int n=50;
      float rawA3=0;
      pm10fs = 0;
      for(int i = 0;i<n;i++){ 
          //tkimo = tkimo + analogRead(TempKimoPin);
          rawA3 = rawA3 + analogRead(A0);
          //delay(2);
      } 
      
      rawA3 = rawA3/n;      
      pm10fs = (float) (rawA3*3.3)/1024.0;
      pm10fs = (pm10fs+0.03)*330;
      Serial.print("\npm10fs ");
      Serial.println(pm10fs );
}
void updateLCD(){

  if(millis() - timerLCD > 6000){
    Wire.begin(D2,D1);//D6,D5); 
    delay(10);
    lcd.clear();
    lcd.setCursor(0, 0);
    
    if(flagCO2){
        lcd.print("Te ");
        lcd.print(tempoffset,1);
        lcd.print("C ");
        
        lcd.print("Umi ");
        lcd.print(String((int)scd1hum));
        lcd.print("%"); 
        lcd.setCursor(0, 1);        
        lcd.print("CO2  ");
        lcd.print(String((int)scd1co2));
        lcd.print(" ppm ");
        if(nuvem){
          lcd.print("on");
        }else{
          lcd.print("of");
        }
        
        
        flagCO2=false;
        
    }else{
        
        lcd.print("PM 2.5  ");
        lcd.print(String((int)m.mc_2p5));
        lcd.print(" ug/m3   ");
        lcd.setCursor(0, 1);
        lcd.print("PM 10   ");
        lcd.print(String((int)m.mc_10p0));
        lcd.print(" ug/m3   ");
      
        flagCO2 = true;
    }
       timerLCD = millis();          
                  
    }
    
}
