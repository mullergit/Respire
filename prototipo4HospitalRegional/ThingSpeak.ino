void sendThingSpeak(){
  Serial.println("\t Enviando ThingSpeak canal 2......................................................");
        int tentativas = 0;
        if(t66co2==0)return;
          if(WiFi.status() != WL_CONNECTED){            
              errorede++;
              if(errorede>10){
                ESP.reset();     
              }else{
                return;
              }            
          }               
                     
          //ThingSpeak.setField(1, vkimo); 
          ThingSpeak.setField(2,t66co2 ); 
          ThingSpeak.setField(3, erro);
          ThingSpeak.setField(4, errorede);                  
          ThingSpeak.setField(5,pm1 );
          ThingSpeak.setField(6,pm25 );
          ThingSpeak.setField(7,pm4 );
          ThingSpeak.setField(8,pm10);
          
          //ThingSpeak.setField(7, vkimo);
                    
          int x = ThingSpeak.writeFields(myChannelNumber2, myWriteAPIKey2);
          //x = ThingSpeak.writeField(myChannelNumber2, 1, adc, myWriteAPIKey2);
          if(x == 200){
            Serial.println("Channel 2 update successful.");
          }
          else{
            Serial.println("Problem updating channel 2. HTTP error code " + String(x));
            erro+=1;
          }
                                    
}
