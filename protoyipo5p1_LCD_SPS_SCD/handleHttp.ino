
void handleICS() {
  /*if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }*/
  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<!DOCTYPE html><html lang='en'><head>"
            "<meta name='viewport' content='width=device-width'>"
            "<title>CaptivePortal</title></head><body>"
            "<h1>HELLO ICS Controler!!</h1>");
  if (server.client().localIP() == IPAddress(10,0,1,1)) {
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + WiFi.SSID() + F("</p>");
  }
  Page += F(
            "<p>You may want to <a href='/bonjur'>config the wifi connection</a>.</p>"
            "</body></html>");

  server.send(200, "text/html", Page);
  server.client().stop();
}
void handleRoot() {

  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Serial.print("WiFi.SSID(): ");Serial.print(WiFi.SSID());Serial.print("IP STA: ");Serial.println(WiFi.localIP());
    Serial.print("IP softAP: ");Serial.println(WiFi.softAPIP());

    IPAddress ip = WiFi.localIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    IPAddress ips = WiFi.softAPIP();
    String ipS = String(ips[0]) + '.' + String(ips[1]) + '.' + String(ips[2]) + '.' + String(ips[3]);
  
  Page += F(
            "<!DOCTYPE html><html lang='en'><head>"
            "<meta name='viewport' content='width=device-width'>"
            "<meta charset='utf-8'>"
            "<title>CaptivePortal</title></head><body>"
            "<h1>Sensor WiFi de CO2 e Material Particulado</h1>");
  if (server.client().localIP() == ips) {
    Page += String(F("<p>Você está conectado no soft AP ")) + softAP_ssid +String(F(".  O ip do sensor nessa rede é "))+ipS +String(F("<p>  O ip do sensor na rede WiFi  "))+ WiFi.SSID()+String(F(" é  "))+ipStr+F("</p>");
  } else {
    Page += String(F("<p>Você está conectado na rede WiFi ")) + WiFi.SSID() +String(F(".  O ip do sensor nessa rede é "))+ipStr +String(F("<p>  O ip do sensor como softAP é "))+ipS +F("</p>");
  }
  Page += F(
            "<p>Você pode ir para a <a href='/bonjur'> página de monitoramento</a>.</p>"
            "</body></html>");

  server.send(200, "text/html", Page);
  server.client().stop();
  
    /*ACconfig = true;
  if (ACconfig){ // If caprive portal redirect instead of displaying the page.    
   server.sendHeader("Location", "/bonjur");
   server.send(303);   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    //server.client().stop();
  }else{
    server.sendHeader("Location", "/configAC");
    server.send(303);
  }*/
  
}
/*
boolean captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

void handleWifi() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<!DOCTYPE html><html lang='en'><head>"
            "<meta name='viewport' content='width=device-width'>"
            "<title>CaptivePortal</title></head><body>"
            "<h1>Wifi config</h1>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
  Page +=
    String(F(
             "\r\n<br />"
             "<table><tr><th align='left'>SoftAP config</th></tr>"
             "<tr><td>SSID ")) +
    String(softAP_ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.softAPIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN config</th></tr>"
      "<tr><td>SSID ") +
    String(ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.localIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"      
      "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>");
  //Serial.println("scan start");
  int n = WiFi.scanNetworks();
  //Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</td></tr>");
    }
  } else {
    Page += F("<tr><td>No WLAN found</td></tr>");
  }
  Page += F(
            "</table>"
            "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
            "<input type='text' placeholder='network' name='n'/>"
            "<br /><input type='password' placeholder='password' name='p'/>"
            "<br /><input type='submit' value='Connect/Disconnect'/></form>"
            "<p>You may want to <a href='/'>return to the home page</a>.</p>"
            "</body></html>");
  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content length
}

void handleWifiSave() {
  Serial.println("wifi save");
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.sendHeader("Location", "wifi", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}

void handleNotFound() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++) {
    message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", message);
}*/
