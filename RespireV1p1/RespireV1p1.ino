#include <Arduino.h>
//#include "FS.h"
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
#include "SoftwareSerial.h"
#include <sps30.h>
#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h" 
#include "ThingSpeak.h"
#include "secrets.h"
#include<LiquidCrystal_I2C_Hangul.h>

#define APSSID "Prototipo 1"
#define APPSK  "respireifsc"
const char *myHostname = "prototipo1V1p1";//???????
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
NTPClient timeClient(ntpUDP, "a.st1.ntp.br", utc*3600, 600000);
LiquidCrystal_I2C_Hangul lcd(0x27,16,2);
WiFiClient  client;
SCD30 scd1;

float offset = 3;//6.38;

int16_t ret;
  uint8_t auto_clean_days = 4;
  uint32_t auto_clean;
  struct sps30_measurement m;
  char serial[SPS30_MAX_SERIAL_LEN];
  uint16_t data_ready;

bool flag=true, f2=true, flagCO2 = true;
byte count = 0;
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
//int keyIndex = 0;            // your network key Index number (needed only for WEP)


unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
unsigned long myChannelNumber2 = SECRET_CH_ID_2;
const char * myWriteAPIKey2 = SECRET_WRITE_APIKEY_2;

unsigned long timer1 = 0, timer2 = 0, timerLCD = 6000, timer3;
float erro=0;
float scd1temp, tempoffset, scd1hum, scd1co2, scd2temp, scd2hum, scd2co2, t66co2;
bool flagSCD1=false;

const int ledPin = 15;
String ledState;       

// Flags:
bool  f_Clone = false, 
      f_AC = false,
      flagReadEEPROM=true,
      f_hasCode = false,
      ACconfig = false,
      nuvem = false;
int nuveminterval = 30;

float cm3m3 = 1000000;
void openFS();
void createFile();
void handleRoot();


void setup() {
  Serial.begin(115200);
  pinMode(D6,OUTPUT);
  digitalWrite(D6,HIGH);
  //Wire.begin(D4,D3);
  lcd.begin(16,2);
  lcd.init();
  lcd.backlight();
  
  Wire.begin(D2,D1);//D6,D5);    
  lcd.clear();
  lcd.setCursor(0, 0);    
  lcd.print("Iniciando... ");
        
  delay(2000);
  Wire.begin(D4,D3);
  if (scd1.begin() == false){
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    flag=false;
  }else{
     Serial.println("Air sensor temp offset...");
        Serial.println(scd1.getTemperatureOffset());

    //setTemperatureOffset(float tempOffset);
    scd1.setTemperatureOffset(offset);
    scd1.setMeasurementInterval((uint16_t) 20);
    /*airSensor.setAltitudeCompensation(5);
    delay(2000);
    airSensor.setAmbientPressure(1000);
    delay(2000);
    airSensor.setForcedRecalibrationFactor(787);*/
  }

  
  Wire.begin(D7,D8);
  delay(500);
   sensirion_i2c_init();

  while (sps30_probe() != 0) {
    Serial.print("SPS sensor probing failed\n");
    delay(500);
  }
  Serial.print("SPS sensor probing successful\n");

  ret = sps30_set_fan_auto_cleaning_interval_days(auto_clean_days);
  if (ret) {
    Serial.print("error setting the auto-clean interval: ");
    Serial.println(ret);
  }

  ret = sps30_start_measurement();//sps30_stop_measurement();45-65mA --> ~330uA
  if (ret < 0) {
    Serial.print("error starting measurement\n");
  }else{
    Serial.print("measurements started\n");
  }
    

    delay(1000);
    readSCD();
    readSPS();
    
    ret = sps30_stop_measurement();
    if (ret < 0) {
    Serial.print("error stoping measurement\n");
  }else{
    Serial.print("measurements stoped\n");
  }  
                 
  printStatus();    
  updateLCD();
  
  initsetup();
  ThingSpeak.begin(client);
  
  server.serveStatic("/ics", SPIFFS, "/index.html");
  server.serveStatic("/bonjur", SPIFFS, "/bonjur23.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/css", SPIFFS, "/css");
  server.serveStatic("/img", SPIFFS, "/img");
  server.on("/", handleRoot);
  server.on("/tabmesures.json", sendTabMesures);
  server.on("/mesures.json", sendMesures);
  server.on("/gpio", updateGpio);
  server.begin(); // Web server start  
 
  timeClient.begin();
  timeClient.update();

  ArduinoOTA.setPassword("xxxxxxx");
  ArduinoOTA.setPasswordHash("xxxxxxxxxx");
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
  
  if(millis() - timer1 > nuveminterval*1000){
         // WiFi.forceSleepWake();
         //int minutes = timeClient.getMinutes();
         int h = timeClient.getHours();
         if(h>4 & h<20){
          nuvem = true;
         }else nuvem = false;
          delay(2000);
          Serial.print("\nTimer ThingSpeak..................");   
          Serial.print("Envio nuvem: ");
          Serial.print(nuvem);
          Serial.print("\tIntervalo Envio nuvem: ");
          Serial.println(nuveminterval);
          //if(nuvem)sendThingSpeak();
          timer1 = millis();
          //WiFi.forceSleepBegin();
          
  }
  if(millis()-timer3>25000){
    checkOST();
    Serial.print("Lendo SCD: ");
    readSCD();
    Serial.print("Lendo SPS: ");
    readSPS();   
    printStatus();
    timer3=millis();
  }
   updateLCD();
}
