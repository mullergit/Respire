#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266HTTPClient.h>
#include <DM02A.h>  //Inclui a biblioteca para comunicar com o dimmer DM02A
#include "Irms_Calc.h"
#include <Ticker.h>
#include <ArduinoJson.h>

#define MANUALMODE 1
#define AUTOMODE   0

#define NIVEL0  0
#define NIVEL1  40
#define NIVEL2  55
#define NIVEL3  70
#define ON LOW
#define OFF HIGH

int niveldimmer = NIVEL0;

const int sensorIn = A0;
const int ledon = 12;
const int ledoff = 2;
const int statepin = D7;
int STATE;// = MANUALMODE;
int nivelCH1;
int nivelCH2;
int set1;
int set2;
bool changeNivel = false;

Ticker ticker;
//Irms_Calc acs712;
ACS712_Irms acs;
//Cria um novo objeto
DM02A dimmer(D1, D2);//SIG, CH, EN - Se não usar o EN, deixar sem informar

#define APSSID "SensorCO2ePM"
#define APPSK  "12345678"

const char* host = "192.168.4.1"; 
float reqs, erroPost=0, erroGet = 0, errorede=0, nposts = 0, ngets = 0;
unsigned long timer1 = 0, timer2 = 0, timerLCD = 0, timer3,timer4;
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

WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
volatile time_t _disconnected_time = 0;
String payloadteste;


void setup() {
  
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ledon, OUTPUT);
  pinMode(ledoff, OUTPUT);
  //pinMode(statepin, OUTPUT);
  //digitalWrite(statepin, LOW);
  pinMode(statepin, INPUT_PULLUP);
  digitalWrite(ledon, OFF);
  digitalWrite(ledoff, ON);

  pinMode(D2, OUTPUT);
  digitalWrite(D2, 1);
  
  delay(3000);
  
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
  acs.mVperAmp = ACS712_30A;//185;//scaleFactor::ACS712_20A; // use 100 for 20A Module and 66 for 30A Module and 185 for 5A Module
  acs.maxADCVolt = 3.3; //5 Volts
  acs.ADCIn = A0;
  acs.Init();

  gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event)  {
    Serial.print("gotIpEventHandler...Station connected, IP: ");
    Serial.println(WiFi.localIP());
    ticker.detach();
    digitalWrite(ledoff, OFF);
    digitalWrite(ledon, ON);
  });

  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event){
    _disconnected_time = millis();
    Serial.println("disconnectedEventHandler...Station disconnected");
    ticker.attach(0.5, tick);
    digitalWrite(ledon, OFF);
  });

  ticker.attach(0.5, tick);
  STATE = digitalRead(statepin);

}

void loop() {
  STATE = digitalRead(statepin);
  if(nivelCH2!=set2)changeNivel=true;

  switch (STATE) {
  case MANUALMODE:
      if(nivelCH2!=0){
        dimmer.EnviaNivel(0,1);
        delay(100);
        nivelCH2= dimmer.feedback(1);
      }
    break;
  case AUTOMODE:
      if(changeNivel){
        dimmer.EnviaNivel(set2,1);
        changeNivel=false;
        delay(100);
        nivelCH2= dimmer.feedback(1);
      }      
    break;
  default:
    break;
}
  if(millis()-timer1>10000){
    //printStatus();
    timer1=millis();     
  }
  
  if(millis()-timer2>10000){
    Serial.print("\ntimer GetState");
    sendState08_09();
    timer2=millis();     
  }
  
  if(millis()-timer3>5300){
    Serial.print("\ntimer GetConfig");
    //getConfig();
    //getConfig25_09();
    get3();
    timer3=millis();     
  }  
  if(millis()-timer4>3000){
    nivelCH2= dimmer.feedback(1);
    //Serial.print("STATE: ");Serial.println(STATE);
    timer4=millis();     
  }   
              
}//End Loop

void tick(){
  int state = digitalRead(ledoff);  
  digitalWrite(ledoff, !state);     
}

void sendState08_09(){
  int ventilationMode = 0;
  double amps = acs.Process();
  nivelCH1= dimmer.feedback(0);//Solicita o nivel atual do canal 1
  nivelCH2= dimmer.feedback(1);//Solicita o nível atual do canal 2
  
  WiFiClient client;
  //const char * host = "192.168.4.1";            //default IP address
  //const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("\nconnection failed");
    return;
  }
  String url = "/data/";
  url += "?sensor_reading=";
  //url +=  "{\"ventilationMode\":\"ventilation_value\",\"amps\":\"amps_value\",\"ch1\":\"ch1_value\",\"ch2\":\"ch2_value\",\"rssi\":\"rssi_value\"}";
  DynamicJsonDocument jsonBuffer(512);
  JsonObject msg = jsonBuffer.to<JsonObject>();

  msg["ventilationMode"] = STATE;
  msg["amps"] = amps ;
  msg["ch1"] = nivelCH1;
  msg["ch2"] = nivelCH2;
  msg["rssi"] = WiFi.RSSI();
  String json;
  serializeJson(msg, json);
  url+= json; 
  
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


void getConfig(){
 Serial.print("\nconnecting to ");Serial.println(host);
 Serial.print("\nWiFi.status() ");Serial.println(WiFi.status());
      if(WiFi.status() != WL_CONNECTED){ 
            //WiFi.begin("ESP_D54736"); 
              WiFi.begin(softAP_ssid,softAP_password);      //Connect to this SSID. In our case esp-01 SSID.  
          
            errorede++;
            if(errorede>20){
              Serial.print("\nReset......... ");
              delay(100);
                 ESP.reset();     
            }else{
                 return;
            }            
      }              
      if (!client.connect("192.168.4.1", httpPort)) {
            Serial.println("\nconnection failed!!!");
            erroGet++;
            return;
      }
      client.print(String("GET ") +"/config"+" HTTP/1.1\r\n" + 
                           "Host: " + host + "\r\n"+ 
                           "Connection: close\r\n\r\n");         
      //delay(10);
      String line;
      while(client.available()){
            line = client.readStringUntil('\r');
            //line = client.readString();
            //Serial.print(line);
      }
      Serial.println(line);
      Serial.print("length():");
    Serial.println(line.length());

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, line);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    //const char* timestamp = doc["time"];
    long timestamp = doc["time"];
    set1 = doc["set1"];
    set2 = doc["set2"];
  
    Serial.println(timestamp);
    Serial.println(set1);
    Serial.println(set2);
    
      //clientGet.flush();
      Serial.println("\n closing connection");       
      Serial.print("\nRequests: ");
      reqs++;
      Serial.println(reqs);
      client.stop();
  
}
void getConfig25_09(){
    HTTPClient http;  //Object of class HTTPClient
    http.useHTTP10(true);
    http.begin("http://192.168.4.1:80/config");
    int httpCode = http.GET();
    //Check the returning code                                                                  
    if (httpCode > 0) {
      // Get the request response payload
      //payloadteste = http.readStringUntil('\r');//getString();
      const String&  payload = http.getString();
      Serial.println(payload);
      //yield();
      // TODO: Parsing
    }
    //http.end();   //Close connection
  
}
void get3(){
  WiFiClient client;
    HTTPClient http; //must be declared after WiFiClient for correct destruction order, because used by http.begin(client,...)
    http.begin(client, "http://192.168.4.1:80/config");
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        int len = http.getSize();
        uint8_t buff[128] = { 0 };

#if 1
        payloadteste = http.getString();
        Serial.println(payloadteste);
        //Serial.println("zero?????");
        
        StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payloadteste);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    long timestamp = doc["time"];
    set1 = doc["set1"];
    set2 = doc["set2"];
  
    Serial.println(timestamp);
    Serial.println(set1);
    Serial.println(set2);
#else
        // or "by hand"
        Serial.println("by hand??????????");

        // get tcp stream
        WiFiClient * stream = &client;

        // read all data from server
        while (http.connected() && (len > 0 || len == -1)) {
          // read up to 128 byte
          int c = stream->readBytes(buff, std::min((size_t)len, sizeof(buff)));
          Serial.printf("readBytes: %d\n", c);
          if (!c) {
            Serial.println("read timeout");
          }

          // write it to Serial
          Serial.write(buff, c);

          if (len > 0) {
            len -= c;
          }
        }
#endif

        Serial.println();
        Serial.print("[HTTP] connection closed or file end.\n");
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}
void printStatus(){  
    Serial.print("WiFi.SSID(): ");Serial.print(WiFi.SSID());Serial.print("IP STA: ");Serial.println(WiFi.localIP());
    Serial.print("IP softAP: ");Serial.println(WiFi.softAPIP());    
    //Serial.print("wifiManager.getConfigPortalSSID(): ");Serial.println(wifiManager.getConfigPortalSSID().c_str());
    Serial.print("Hostname3: ");Serial.println(WiFi.hostname());
    //Serial.print("nposts:");Serial.println(nposts);    
    //Serial.print("erroPosts:");Serial.println(erroPost);
    //Serial.print("ngets: ");Serial.println(ngets);
    //Serial.print("erroGets: ");Serial.println(erroGet);      
    //Serial.print("STATE: ");Serial.println(STATE);
}
