void sendThingSpeak(){
   Serial.println("\t Enviando ThingSpeak canal 1......................................................");
      int tentativas = 0;
          if(WiFi.status() != WL_CONNECTED){
            Serial.print("Attempting to connect to SSID: ");
            Serial.println(SECRET_SSID);
            while(WiFi.status() != WL_CONNECTED){
              WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
              Serial.print(".");
              tentativas++;
              if(tentativas>2)return;
              delay(3000);     
            } 
            Serial.println("\nConnected.");
          }               

          ThingSpeak.setField(1, m.mc_1p0);
          ThingSpeak.setField(2, m.mc_2p5);
          ThingSpeak.setField(3, m.mc_4p0);
          ThingSpeak.setField(4, m.mc_10p0 );
          
          ThingSpeak.setField(5, m.nc_1p0*cm3m3);
          ThingSpeak.setField(6, m.nc_2p5*cm3m3);
          ThingSpeak.setField(7, m.nc_4p0*cm3m3);
          ThingSpeak.setField(8, m.nc_10p0*cm3m3 );
          
          
          
          int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
          if(x == 200){
            Serial.println("Channel update successful.");
          }
          else{
            Serial.println("Problem updating channel. HTTP error code " + String(x));
            erro++;
          }

          Serial.println("\t Enviando ThingSpeak Canal 2................................................");
          ThingSpeak.setField(1,scd1temp);
          ThingSpeak.setField(2, tempoffset);
          ThingSpeak.setField(3, scd1hum);
          ThingSpeak.setField(4, scd1co2);
          
          ThingSpeak.setField(5, m.typical_particle_size);
          ThingSpeak.setField(6,m.nc_0p5*cm3m3 );
          ThingSpeak.setField(7,erro );                   
          //ThingSpeak.setField(8,erro );
          
                    
          x = ThingSpeak.writeFields(myChannelNumber2, myWriteAPIKey2);
          //x = ThingSpeak.writeField(myChannelNumber2, 1, adc, myWriteAPIKey2);
          if(x == 200){
            Serial.println("Channel 2 update successful.");
          }
          else{
            Serial.println("Problem updating channel 2. HTTP error code " + String(x));
            erro+=1000;
          }
                                    
}
