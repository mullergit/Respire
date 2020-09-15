#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266HTTPClient.h>
#include <DM02A.h>  //Inclui a biblioteca para comunicar com o dimmer DM02A
#include "Irms_Calc.h"

#define MANUALMODE 0
#define AUTOMODE   1

#define NIVEL0  0
#define NIVEL1  40
#define NIVEL2  55
#define NIVEL3  70

const int sensorIn = A0;

//Irms_Calc acs712;
ACS712_Irms acs;
//Cria um novo objeto
DM02A dimmer(D1, D2);//SIG, CH, EN - Se não usar o EN, deixar sem informar

#define APSSID "SensorCO2ePM"
#define APPSK  "12345678"

const char* host = "192.168.4.1"; 
float reqs, erroPost=0, erroGet = 0, errorede=0, nposts = 0, ngets = 0;
unsigned long timer1 = 0, timer2 = 0, timerLCD = 0, timer3;
WiFiClient client;
//WiFiClient clientGet;
const int httpPort = 80;
IPAddress ip(192, 168, 4, 4);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
 
String serverHost = "http://192.168.4.1/feed";
String data;

const char *softAP_ssid = APSSID;
const char *softAP_password = APPSK;

int sensorValue0 = 0;       //sensor value, I'm usingg 0/1 button state
int sensorValue1 = 11;        
int sensorValue2 = 22;        
int sensorValue3 = 33; 

void setup() {
  //ESP.eraseConfig();
  Serial.begin(9600);          //Baud Rate for Communication
  delay(10);                     //Baud rate prper initialization
  //pinMode(13,INPUT);             //Pin D7 on NodeMcu Lua. Button to switch on and off the solenoid.
  WiFi.mode(WIFI_STA);           //NodeMcu esp12E in station mode
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(softAP_ssid,softAP_password);      //Connect to this SSID. In our case esp-01 SSID.  
  //WiFi.begin("ESP_D54736");      //Connect to this SSID. In our case esp-01 SSID. 
  delay(100);
  if(WiFi.status() != WL_CONNECTED){
    connectWifi();
  }

  //Força os dois canais iniciarem desligados
  dimmer.EnviaNivel(0,0);
  dimmer.EnviaNivel(0,1);

  acs.ADCSamples = 1024.0; //1024 samples
  acs.mVperAmp = ACS712_5A;//185;//scaleFactor::ACS712_20A; // use 100 for 20A Module and 66 for 30A Module and 185 for 5A Module
  acs.maxADCVolt = 3.3; //5 Volts
  acs.ADCIn = A0;
  acs.Init();

}

void loop() {
  if(millis()-timer1>10000){
    printStatus();
    timer1=millis();     
  }
  
  if(millis()-timer2>10000){
    Serial.print("\ntimer GetState");
    //buildDataStream();    
    //sendState2();
    //nposts++;
    sendState08_09();
    timer2=millis();     
  }
  
  if(millis()-timer3>7300){
    Serial.print("\ntimer GetConfig");
    getConfig();
    //ngets++;
    timer3=millis();     
  }     
              
}//End Loop

void sendState08_09(){
  WiFiClient client;
  //const char * host = "192.168.4.1";            //default IP address
  //const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  // We now create a URI for the request. Something like /data/?sensor_reading=123
  String url = "/data/";
  url += "?sensor_reading=";
  url +=  "{\"sensor0_reading\":\"sensor0_value\",\"sensor1_reading\":\"sensor1_value\",\"sensor2_reading\":\"sensor2_value\",\"sensor3_reading\":\"sensor3_value\"}";

  url.replace("sensor0_value", String(sensorValue0));
  url.replace("sensor1_value", String(sensorValue1));
  url.replace("sensor2_value", String(sensorValue2));
  url.replace("sensor3_value", String(sensorValue3));


  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
      
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 2000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
}

void connectWifi(){
  Serial.println("Conectando..........");
  int attempts = 0;
  if (WiFi.status() != WL_CONNECTED) { 
    WiFi.begin(softAP_ssid,softAP_password);
    attempts++;
    if(attempts>10){
      Serial.println("Não foi possível conectar!");
      //return;     
    }
    delay(2000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}


void buildDataStream() {//msg["RSSI"] = WiFi.RSSI()
  int exaustorMode = 0;
  double amps = acs.Process();
  int nivelCH1= dimmer.feedback(0);//Solicita o nivel atual do canal 1
  int nivelCH2= dimmer.feedback(1);//Solicita o nível atual do canal 2
  
  data = "mod=";
  data += String(exaustorMode);
  data += "&amps=";
  data += String(amps);
  data += "&ch1=";
  data += String(nivelCH1);
  data += "&ch2=";
  data += String(nivelCH2);
  data += "&rssi=";
  data += String(WiFi.RSSI());
  Serial.println("- data stream: "+data);
}
void getConfig(){
 Serial.print("\nconnecting to ");Serial.println(host);
 Serial.print("\nWiFi.status() ");Serial.println(WiFi.status());
      if(WiFi.status() != WL_CONNECTED){ 
            //WiFi.begin("ESP_D54736"); 
              WiFi.begin(softAP_ssid,softAP_password);      //Connect to this SSID. In our case esp-01 SSID.  
          
            errorede++;
            if(errorede>10){
              Serial.print("\nReset......... ");
              delay(100);
                 //ESP.reset();     
            }else{
                 return;
            }            
      }              
      //clientGet.setNoDelay();        
      //clientGet.flush();
      if (!client.connect("192.168.4.1", httpPort)) {
            Serial.println("connection failed");
            erroGet++;
            return;
      }
      client.print(String("GET ") +"/config"+" HTTP/1.1\r\n" + 
                           "Host: " + host + "\r\n"+ 
                           "Connection: close\r\n\r\n");         
      delay(10);
      while(client.available()){
            String line = client.readStringUntil('\r');
            Serial.print(line);
      }
      //clientGet.flush();
      Serial.println("\n closing connection");       
      Serial.print("\nRequests: ");
      reqs++;
      Serial.println(reqs);
      client.stop();
  
}
void sendState() {
  Serial.println("\n Enviando POST");
  HTTPClient http;
  http.begin(serverHost);
  //http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Content-Type", "text/plain");
  int httpCode = http.POST(data);
  //int httpCode = http.POST("Hello from esp8266");
    //int httpCode = http.POST("");
    Serial.println("HTTP Code: " + String(httpCode));
    if (httpCode != 200){
      Serial.println("Couldn't send the request, got code: " + httpCode);
      erroPost++;
    } else {
      
      Serial.println("Request was sent successfully");
    }
    //String payload = http.getString();
    //Serial.println(payload);
  //http.writeToStream(&Serial);
  http.end();
}
void sendState2() {
  Serial.println("\n Enviando POST");
    WiFiClient client; 
    if (!client.connect("192.168.4.1", httpPort))     { 
      Serial.println("connection failed");
      erroPost++; 
      return; 
    } 
    HTTPClient http; 
    http.begin(serverHost); 
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); 
    int httpCode = http.POST(data); 
    Serial.println("HTTP Code: " + String(httpCode));
    if (httpCode != 200){
      Serial.println("Couldn't send the request, got code: " + httpCode);
      erroPost++;
    } else {      
      Serial.println("Request was sent successfully");
    } 
    http.end(); //Close connection Serial.println(); 
    Serial.println("closing connection"); 
 
}
void sendState3(){
 Serial.print("\nconnecting to ");Serial.println(host);
 Serial.print("\nWiFi.status() ");Serial.println(WiFi.status());
      if(WiFi.status() != WL_CONNECTED){ 
              WiFi.begin(softAP_ssid,softAP_password);             
            errorede++;
            if(errorede>10){
              //Serial.print("\nReset......... ");
              delay(100);
                 //ESP.reset();     
            }else{
                 return;
            }            
      }                      
      if (!client.connect("192.168.4.1", httpPort)) {
            Serial.println("connection failed");
            erroPost++;
            return;
      }
      //ngets++;    
      client.print("/feed?"+data+" HTTP/1.1\r\n" + 
                           "Host: " + host + "\r\n" + 
                           "Connection: close\r\n\r\n");         
      delay(10);
      while(client.available()){
            String line = client.readStringUntil('\r');
            Serial.print(line);
      }
      Serial.println("\n closing connection");       
      //client.stop();
  
}
void printStatus(){  
    Serial.print("WiFi.SSID(): ");Serial.print(WiFi.SSID());Serial.print("IP STA: ");Serial.println(WiFi.localIP());
    Serial.print("IP softAP: ");Serial.println(WiFi.softAPIP());    
    //Serial.print("wifiManager.getConfigPortalSSID(): ");Serial.println(wifiManager.getConfigPortalSSID().c_str());
    Serial.print("Hostname3: ");Serial.println(WiFi.hostname());
    Serial.print("nposts:");Serial.println(nposts);    
    Serial.print("erroPosts:");Serial.println(erroPost);
    Serial.print("ngets: ");Serial.println(ngets);
    Serial.print("erroGets: ");Serial.println(erroGet);      
}
