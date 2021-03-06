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
    
    //Serial.print("mstate: ");Serial.println(mstate);
    //if(mstate==1){Serial.print("initDecodeCounter: ");Serial.println(initDecodeCounter);}
    //timer1=millis();
  
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
String getTemperature() {
  float temperature = 35;
  Serial.println(temperature);
  return String(temperature);
}
  
String getHumidity() {
  float humidity = 55;
  Serial.println(humidity);
  return String(humidity);
}

String getPressure() {
  float pressure = 99;
  Serial.println(pressure);
  return String(pressure);
}
void sendTabMesures() {
  double temp = 0;      // Récupère la plus ancienne mesure (temperature) - get oldest record (temperature)
  String json = "[";
  json += "{\"mesure\":\"Température\",\"valeur\":\"" + String(33) + "\",\"unite\":\"°C\",\"glyph\":\"glyphicon-indent-left\",\"precedente\":\"" + String(temp) + "\"},";
  temp = 0;             // Récupère la plus ancienne mesure (humidite) - get oldest record (humidity)
  json += "{\"mesure\":\"Humidité\",\"valeur\":\"" + String(66) + "\",\"unite\":\"%\",\"glyph\":\"glyphicon-tint\",\"precedente\":\"" + String(temp) + "\"},";
  temp = 0;             // Récupère la plus ancienne mesure (pression atmospherique) - get oldest record (Atmospheric Pressure)
  json += "{\"mesure\":\"Pression Atmosphérique\",\"valeur\":\"" + String(99) + "\",\"unite\":\"mbar\",\"glyph\":\"glyphicon-dashboard\",\"precedente\":\"" + String(temp) + "\"}";
  json += "]";
  server.send(200, "application/json", json);
  Serial.println("Tableau mesures envoyees");
}

void updateGpio(){
  
  
  String gpio = server.arg("id");
  String etat = server.arg("etat");
  String success = "1";
  int pin = 15;  
 if ( gpio == "D8" ) {
      if ( etat == "1" ) {
          nuvem = true;
      } else if ( etat == "0" ) {
          nuvem = false;
      }
 }else if ( gpio == "D5" ) {
      if ( etat == "1" ) {
          nuveminterval = 15;
      } else if ( etat == "0" ) {
          nuveminterval = 30;
      }
 }else if ( gpio == "D0" ) {
     wifiManager.resetSettings();
      ESP.reset();
 }else {
    success = "1";
    Serial.println("Err Led Value");
  }  
  String json = "{\"gpio\":\"" + String(gpio) + "\",";
  json += "\"etat\":\"" + String(etat) + "\",";
  json += "\"success\":\"" + String(success) + "\"}";    
  server.send(200, "application/json", json);
  Serial.println("GPIO mis a jour");
}
void sendMesures() {
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();

  msg["t"] = String(tempoffset,1);
  msg["h"] =  String(scd1hum,1);
  msg["co2"] = scd1co2 ;
  
  msg["pm1s"] = String(m.mc_1p0,1);
  msg["pm25s"] = String(m.mc_2p5,1);
  msg["pm4s"] = String(m.mc_4p0,1);
  msg["pm10s"] = String(m.mc_10p0,1);

  msg["n1s"] = String(m.nc_1p0*cm3m3,1);
  msg["n25s"] = String(m.nc_2p5*cm3m3,1);
  msg["n4s"] = String(m.nc_4p0*cm3m3,1);
  msg["n10s"] = String(m.nc_10p0*cm3m3,1);
  if(nuvem){
    msg["nuvem"] = "on";
  }else{
    msg["nuvem"] = "off";
  }
  msg["nuveminterval"] = nuveminterval;
    
        
  String json;
  serializeJson(msg, json);
  server.send(200, "application/json", json);
  Serial.println("Mesures envoyees");
}
void sendMessage() {
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();
  msg["nameId"] = "nameIdjson";
  msg["temp"] = random(20, 29);
  msg["memory"] = ESP.getFreeHeap();
  msg["RSSI"] = WiFi.RSSI();//String(WiFi.RSSI());
  String json;
  serializeJson(msg, json);
  server.send(200, "application/json", json);
          
}
void checkOST(void) {  
    Serial.println("Time Epoch: "+ String(timeClient.getEpochTime()));
    Serial.println(timeClient.getFormattedTime());
    Serial.println("Day: "+ String(timeClient.getDay()));
    Serial.println("Hours: "+ String(timeClient.getHours()));
    Serial.println("Minutes: "+ String(timeClient.getMinutes()));
}
