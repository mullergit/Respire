#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>
#include <EEPROM.h>
#include "FS.h"
#include <TimeLib.h>
//#include <NtpClientLib.h>
#include <ArduinoJson.h>
//#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <Ticker.h>
#include <FS.h>

#define NLIN 13
#define NCOL 243

#define APSSID "Prototipo 1"
#define APPSK  "12345678"
const char *myHostname = "icscontrol";//???????
const char *softAP_ssid = APSSID;
const char *softAP_password = APPSK;
#include "SoftwareSerial.h"
#include <sps30.h>
#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h" 
#include "ThingSpeak.h"
#include "secrets.h"
#include <ESP8266WiFi.h>
#include<LiquidCrystal_I2C_Hangul.h>

LiquidCrystal_I2C_Hangul lcd(0x27,16,2);

WiFiClient  client;

SCD30 scd1;
float offset = 4;//6.38;
byte response[32];// = {0,0,0,0,0,0,0};
uint16_t pm1,pm25,pm4,pm10;

int16_t ret;
  uint8_t auto_clean_days = 4;
  uint32_t auto_clean;
  struct sps30_measurement m;
  char serial[SPS30_MAX_SERIAL_LEN];
  uint16_t data_ready;
//  int16_t ret;

bool flag=true, f2=true, flagCO2 = true;
byte count = 0;
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)


unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
unsigned long myChannelNumber2 = SECRET_CH_ID_2;
const char * myWriteAPIKey2 = SECRET_WRITE_APIKEY_2;

unsigned long timer1 = 0, timer2 = 0, timerLCD = 8005, timer3;
float erro=0;
float scd1temp, tempoffset, scd1hum, scd1co2, scd2temp, scd2hum, scd2co2, t66co2;
bool flagSCD1=false;
void DisableAutoSend() {
  const char cmd[] = {0x68, 0x01, 0x20, 0x77};
  //SendCmd(cmd, 4);
}

//byte cmdDisable[] = {0x68, 0x01, 0x20, 0x77};//HPMA

//byte readPPM[]= {FLAG, ADDR, 0x02, CMD_READ, CO2PPM};//Telaire

//byte readPM[]= {0x68, 0x01, 0x04, 0x93};//HPMA

Ticker ticker;
WiFiManager wifiManager;
ESP8266WebServer server(80);
const byte DNS_PORT = 53;
DNSServer dnsServer;

//const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
//IRsend irsend(kIrLed);
//const uint16_t kRecvPin = 14;
const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 512;//1024;
const uint8_t kTimeout = 50;
const uint16_t kMinUnknownSize = 12;
#define LEGACY_TIMING_INFO false
//IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
//decode_results results;

const int ledPin = 15;
String ledState;       

uint16_t rawCode[NCOL];
int bufLen = NLIN * NCOL * 2;
int addr = 0;  

// Flags:
bool  f_Clone = false, 
      f_AC = false,
      flagReadEEPROM=true,
      f_hasCode = false,
      ACconfig = false,
      nuvem = true;//false;
int nuveminterval = 30;
//Decode IR:      
int ir_type = 0;
uint8_t initDecodeCounter = 3;
int indexMatriz = 0;
int rawSize = 0;

//MUX:
float data[3];
const byte s0 = 5; // low-order bit
const byte s1 = 12;
const byte mux = A0;
int x;
int analog=1023;
int var1, var2;
float adc, n=10, n2=39;

//Timer:
float timerConfig;

float cm3m3 = 1000000;
void openFS();
void createFile();
void handleRoot();

DynamicJsonDocument jsonBuffer(1024);
JsonObject root = jsonBuffer.to<JsonObject>();

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
enum State_enum {IDL, DECODE, WIFICONFIG};
enum Sensors_enum {NONE, SENSOR_RIGHT, SENSOR_LEFT, BOTH};
uint8_t mstate = IDL;

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
    /*airSensor.setAltitudeCompensation(5);
    delay(2000);
    airSensor.setAmbientPressure(1000);
    delay(2000);
    airSensor.setForcedRecalibrationFactor(787);*/
  }

  
  Serial.print("111........... ");
  Wire.begin(D7,D8);
  delay(500);
   sensirion_i2c_init();

  while (sps30_probe() != 0) {
    Serial.print("SPS sensor probing failed\n");
    delay(500);
  }
Serial.print("222........... ");
  Serial.print("SPS sensor probing successful\n");

  ret = sps30_set_fan_auto_cleaning_interval_days(auto_clean_days);
  if (ret) {
    Serial.print("error setting the auto-clean interval: ");
    Serial.println(ret);
  }

  ret = sps30_start_measurement();
  if (ret < 0) {
    Serial.print("error starting measurement\n");
  }
  Serial.print("measurements started\n");  

    delay(3000);
    readSCD();
    readSPS();             
    printStatus();    
    updateLCD();
    delay(4000);    
  
  initsetup();
  //WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  
  server.serveStatic("/ics", SPIFFS, "/index.html");
  server.serveStatic("/bonjur", SPIFFS, "/bonjur23.html");
  server.serveStatic("/configAC", SPIFFS, "/configAC.html");
  server.serveStatic("/configACmanual", SPIFFS, "/configACmanual.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.on("/", handleRoot);
  //server.on("/", HTTP_GET, handleRoot, processor());

  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/css", SPIFFS, "/css");
  server.serveStatic("/img", SPIFFS, "/img");
  //server.serveStatic("/", SPIFFS, "/index.html");
  server.on("/tabmesures.json", sendTabMesures);
  server.on("/mesures.json", sendMesures);
  server.on("/mstate.json", sendMstate);
  //server.on("/gravar.json", gravar);
  server.on("/gpio", updateGpio);
  //server.on("/initACconfig.json", initDecode);
  server.begin(); // Web server start
}

void sendMstate(){
    Serial.println("sendMstate!");
          DynamicJsonDocument jsonBuffer(1024);
          JsonObject msg = jsonBuffer.to<JsonObject>();
          if(millis()-timerConfig>120000){            
            timer1=millis();
          }else{            
          }
          msg["msg"] = "Aguardando envio dos códigos...";
          msg["mstate"]=mstate;
          String json;
          serializeJson(msg, json);
          server.send(200, "application/json", json);
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
void loop() {
  
  switch(mstate){
    case IDL:
      break;       
    case DECODE:      
          //decodeIr();      
      break; 
    case WIFICONFIG:
      break;
  }
  MDNS.update();
  dnsServer.processNextRequest();
  server.handleClient();
  
  if(millis() - timer1 > nuveminterval*1000){//30000){ 
          Serial.print("\nTimer ThingSpeak..................");   
          Serial.print("Envio nuvem: ");
          Serial.print(nuvem);
          Serial.print("\tIntervalo Envio nuvem: ");
          Serial.println(nuveminterval);
          if(nuvem)sendThingSpeak();
          timer1 = millis();
          
  }
  if(millis()-timer3>10000){
          readSCD();
          readSPS();   
          //readHPMA();  
    printStatus();
    timer3=millis();
  }
   updateLCD();
}
