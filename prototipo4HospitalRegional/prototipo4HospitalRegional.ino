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
#include "secrets.h"

#define APSSID "Sensor CO2 e PM"
#define APPSK  "12345678"

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
float scd1temp, tempoffset, scd1hum, scd1co2, scd2temp, scd2hum, scd2co2, t66co2;
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

//Timer:
float timerConfig;

void openFS();
void createFile();
void handleRoot();
void sendMesures();
void updateGpio();

float vkimo;

void setup() {
  Serial.begin(115200);
  t66.begin(softSerialT66);
  //pinMode(D5,OUTPUT);
  //digitalWrite(D5,HIGH);  
  /*t66.setSGPT(750);
  delay(3000);
  t66.readSTATUS();
  if(t66.response[3]==0){
      Serial.print("\n t66.sgptCALIBRATE();");
      t66.sgptCALIBRATE();
   }
  
  delay(200000);*/
  
  Serial.print("sizeof(cmdDisable) : ");
  Serial.println(sizeof(cmdDisable));
  softSerial.begin(9600);
  softSerial.write(cmdDisable,sizeof(cmdDisable));
    
  initsetup();  
  
  ThingSpeak.begin(client);
  
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
  server.begin(); // Web server start

  timeClient.begin();
  timeClient.update();

  ArduinoOTA.setPassword("respireifsc");
  ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void loop() {
  
  MDNS.update();
  dnsServer.processNextRequest();
  server.handleClient();
  ArduinoOTA.handle();
  
  if(millis() - timer1 > 30000){ 
         int h = timeClient.getHours();
         if(h>4 & h<20){
          nuvem = true;
         }else nuvem = false;   
          if(nuvem)sendThingSpeak();
          timer1 = millis();
          //Serial.println("\nTimer2........");
  }
  if(millis()-timer3>10000){
          checkOST();
          //readSCD();
          //readSPS();
          readT66();   
          readHPMA();
          kimo();  
          printStatus();
          timer3=millis();
  }
   
}
