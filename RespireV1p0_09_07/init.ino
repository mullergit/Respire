void initsetup(){
  pinMode(BUILTIN_LED, OUTPUT);
  ticker.attach(0.6, tick);  
  Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);
  //while (!Serial)  // Wait for the serial connection to be establised.
    //delay(50);
    delay(2000);
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  //SPIFFS.format();
  openFS();
  createFile();
  
  //wifiManager.resetSettings();
  WiFi.hostname(myHostname);
  Wire.begin(D2,D1);//D6,D5);    
  lcd.clear();
  lcd.setCursor(0, 0);    
  lcd.print("Conectando... ");
  
  wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  wifiManager.setAPCallback(configModeCallback);

  //wifiManager.setTimeout(60);
  if (!wifiManager.autoConnect(softAP_ssid, softAP_password)) {
    Serial.println("failed to connect and hit timeout");
    ESP.reset();
    delay(1000);
  }
  Serial.println("connected...yeey :)");
  ticker.detach();
  //digitalWrite(BUILTIN_LED, LOW);
  
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", IPAddress(10,0,1,1));
  
  if (MDNS.begin(myHostname)) {
    MDNS.addService("http", "tcp", 80);
    Serial.print(F("Open http://"));
    Serial.print(myHostname);
    Serial.println(F(".local/edit to open the FileSystem Browser"));
  }
  else{
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
}
