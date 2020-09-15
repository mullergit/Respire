#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <Ticker.h>
#include <FS.h>
#include "T6615.h"
#include "SparkFun_SCD30_Arduino_Library.h" 
#include "ThingSpeak.h"
//#include "ThingS.ino"
#include "secrets.h"
//#include "handle.h"
//#include "var.h"

#define APSSID "SensorCO2ePM"
#define APPSK  "12345678"
#define NIVEL0  0
#define NIVEL1  40
#define NIVEL2  55
#define NIVEL3  70

const char *myHostname = "prototipo4";//???????
const char *softAP_ssid = APSSID;
const char *softAP_password = APPSK;

Ticker ticker;
WiFiManager wifiManager;
ESP8266WebServer server(80);
const byte DNS_PORT = 53;
DNSServer dnsServer;
WiFiUDP ntpUDP;
int16_t utc = -3; //UTC -3:00 Brazil
uint32_t currentMillis = 0;
uint32_t previousMillis = 0;
NTPClient timeClient(ntpUDP, "a.st1.ntp.br", utc*3600, 60000);
WiFiClient  client;

SoftwareSerial softSerial(12,13); //rx tx
SoftwareSerial softSerialT66(D1,D2); //rx tx

T66 t66;

byte response[132];// = {0,0,0,0,0,0,0};
uint16_t pm1,pm25,pm4,pm10;

bool flag=true, f2=true, flagCO2 = true;
byte count = 0;
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)


unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
unsigned long myChannelNumber2 = SECRET_CH_ID_2;
const char * myWriteAPIKey2 = SECRET_WRITE_APIKEY_2;

unsigned long timer1 = 0, timer2 = 0, timerLCD = 0, timer3;
float erro=0, errorede=0;
float reqs, oldreqs, scd1temp, tempoffset, scd1hum, scd1co2, scd2temp, scd2hum, scd2co2, t66co2;
bool flagSCD1=false;

byte cmdDisable[] = {0x68, 0x01, 0x20, 0x77};//HPMA
byte readPM[]= {0x68, 0x01, 0x04, 0x93};//HPMA 

// Flags:
bool  f_Clone = false, 
      f_AC = false,
      flagReadEEPROM=true,
      f_hasCode = false,
      ACconfig = false,
      nuvem = false;

String sensor_values;

float timerConfig;

void sendThingSpeak();
void openFS();
void createFile();
void handleRoot();
void sendMesures();
void updateGpio();
void initsetup();
void handle_feed();
void sendConfig();
void printStatus();


float vkimo;





void setup() {
  Serial.begin(9600);
  //pinMode(D5,OUTPUT);
  //digitalWrite(D5,HIGH);   
  
  //Serial.print("sizeof(cmdDisable) : ");
  //Serial.println(sizeof(cmdDisable));
  //softSerial.begin(9600);
  //softSerial.write(cmdDisable,sizeof(cmdDisable));
    
  initsetup(); 
  //initMesh();   initServer();
  server.serveStatic("/ics", SPIFFS, "/index.html");
  server.serveStatic("/bonjur", SPIFFS, "/bonjur23.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/css", SPIFFS, "/css");
  server.serveStatic("/img", SPIFFS, "/img");
  server.on("/", handleRoot);
  //server.on("/tabmesures.json", sendTabMesures);
  server.on("/mesures.json", sendMesures);
  server.on("/gpio", updateGpio);
  //server.on("/postplain/", handlePost);
  server.on("/config", sendConfig);
  server.on("/feed", handle_feed);
  server.on("/data/", HTTP_GET, handleSentVar);
  server.begin(); // Web server start
  ThingSpeak.begin(client);  

  timeClient.begin();
  timeClient.update();

 
}
void loop() {
  
  MDNS.update();
  dnsServer.processNextRequest();
  server.handleClient();
  //ArduinoOTA.handle();
  
  /*if(millis() - timer1 > 30000){ 
         int h = timeClient.getHours();
         if(h>4 & h<20){
          nuvem = true;
         }else nuvem = false;   
          if(nuvem)sendThingSpeak();
          timer1 = millis();
          //Serial.println("\nTimer2........");
  }*/
  if(millis()-timer3>20000){
    
          //checkOST();
          //readSCD();
          //readSPS();
          //readT66();   
          //readHPMA();
          //kimo();  
          printStatus();
          timer3=millis();
 
  }
}
