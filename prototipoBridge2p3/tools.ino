// include plain C library
extern "C" {
#include "user_interface.h"
}

boolean clientewifisi=0; //global var



void clientesreales()
{
struct station_info *station_list = wifi_softap_get_station_info();
boolean haywifi = 0;

while (station_list != NULL)
{
clientewifisi = 1;
haywifi = 1;
station_list = STAILQ_NEXT(station_list, next);
}

if (haywifi == 0)
{
clientewifisi = 0;
}

}
void sendThingSpeak() {
  Serial.println("\t Enviando ThingSpeak canal 2......................................................");
        int tentativas = 0;
        //if(t66co2==0)return;
          if(WiFi.status() != WL_CONNECTED){            
              errorede++;
              if(errorede>10){
                //ESP.reset();     
              }else{
                return;
              }            
          }               
                     
          //ThingSpeak.setField(1, vkimo); 
          //ThingSpeak.setField(2,t66co2 ); 
          //ThingSpeak.setField(3, erro);
          //ThingSpeak.setField(4, errorede);                  
          //ThingSpeak.setField(5,pm1 );
          //ThingSpeak.setField(6,pm25 );
          //ThingSpeak.setField(7,pm4 );
          //ThingSpeak.setField(8,pm10);

          ThingSpeak.setField(5,erro );
          ThingSpeak.setField(6,errorede );
          ThingSpeak.setField(7,reqs );
          //ThingSpeak.setField(8,);
          
          //ThingSpeak.setField(7, vkimo);
                    
          int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
          //x = ThingSpeak.writeField(myChannelNumber2, 1, adc, myWriteAPIKey2);
          if(x == 200){
            Serial.println("Channel 2 update successful.");
          }
          else{
            Serial.println("Problem updating channel 2. HTTP error code " + String(x));
            erro+=1;
          }
                                    
}
/** Is this an IP? */
boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}
void printStatus(){  
    Serial.print("WiFi.SSID(): ");Serial.print(WiFi.SSID());Serial.print("IP STA: ");Serial.println(WiFi.localIP());
    Serial.print("IP softAP: ");Serial.println(WiFi.softAPIP());    
    Serial.print("wifiManager.getConfigPortalSSID(): ");Serial.println(wifiManager.getConfigPortalSSID().c_str());
    Serial.print("Hostname3: ");Serial.println(WiFi.hostname()); 
    Serial.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());  

       Serial.print("pmBaixo: ");Serial.println(pm_baixo);
       Serial.print("pmMedio: ");Serial.println(pm_medio);
       Serial.print("pmAlto: ");Serial.println(pm_alto);
}
void tick(){
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode: configModeCallback()");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  ticker.attach(0.2, tick);
}
void checkOST(void) {  
    unsigned long epochTime = timeClient.getEpochTime();
    Serial.println("Time Epoch: "+ String(epochTime));
    Serial.print("Formatted Time: ");
    Serial.println(timeClient.getFormattedTime());
    Serial.println("Day: "+ String(timeClient.getDay()));
    struct tm *ptm = gmtime ((time_t *)&epochTime); 
    int monthDay = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(monthDay);
    Serial.println("Hours: "+ String(timeClient.getHours()));
    Serial.println("Minutes: "+ String(timeClient.getMinutes()));
}
