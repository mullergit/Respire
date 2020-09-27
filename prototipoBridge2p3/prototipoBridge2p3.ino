#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <stdio.h>
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
#include<LiquidCrystal_I2C_Hangul.h>
#include <SPI.h>
#include <TFT_eSPI.h>      // Hardware-specific library
#include "Free_Fonts.h"
#include <SD.h>
#include "SoftwareSerial.h"

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
NTPClient timeClient(ntpUDP, "c.ntp.br", utc*3600, 60000);
//NTPClient timeClient(ntpUDP, "a.st1.ntp.br", utc*3600, 60000);
WiFiClient  client;
SCD30 scd1;
T66 t66;
//SoftwareSerial softSerial(15,13);//(13,15);//<--d7,d8 (12,13)-->d6,d7; //rx tx
SoftwareSerial softSerial(12,13);//-->d6,d7; //rx tx
//SoftwareSerial softSerialT66(D1,D2); //rx tx

LiquidCrystal_I2C_Hangul lcd(0x27,16,2);
TFT_eSPI tft = TFT_eSPI();
// The SPIFFS file name must start with "/".
#define CALIBRATION_FILE "/TouchCalData1"
#define REPEAT_CAL false
// Keypad start position, key sizes and spacing
#define KEY_X 40 // Centre of key
#define KEY_Y 96
#define KEY_W 62 // Width and height
#define KEY_H 30
#define KEY_SPACING_X 18 // X and Y gap
#define KEY_SPACING_Y 20
#define KEY_TEXTSIZE 1   // Font size multiplier

// Using two fonts since numbers are nice when bold
#define LABEL1_FONT &FreeSansOblique12pt7b // Key label font 1
#define LABEL2_FONT &FreeSansBold12pt7b    // Key label font 2

// Numeric display box size and location
#define DISP_X 1
#define DISP_Y 10
#define DISP_W 238
#define DISP_H 50
#define DISP_TSIZE 3
#define DISP_TCOLOR TFT_CYAN

// Number length, buffer for storing it and character index
#define NUM_LEN 12
char numberBuffer[NUM_LEN + 1] = "";
uint8_t numberIndex = 0;

// We have a status line for messages
#define STATUS_X 120 // Centred on this
#define STATUS_Y 65

// Create 15 keys for the keypad
char keyLabel[15][5] = {"New", "Del", "Send", "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "0", "#" };
uint16_t keyColor[15] = {TFT_RED, TFT_DARKGREY, TFT_DARKGREEN,
                         TFT_BLUE, TFT_BLUE, TFT_BLUE,
                         TFT_BLUE, TFT_BLUE, TFT_BLUE,
                         TFT_BLUE, TFT_BLUE, TFT_BLUE,
                         TFT_BLUE, TFT_BLUE, TFT_BLUE
                        };

// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[15];



byte response[132];// = {0,0,0,0,0,0,0};
uint16_t pm1,pm25,pm4,pm10;
uint16_t vpm[7];
uint8_t indexvpm=0;
bool flagvpm = false;
double meanpm;

bool flag=true, f2=true, flagCO2 = true;
byte count = 0;
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)


unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
unsigned long myChannelNumber2 = SECRET_CH_ID_2;
const char * myWriteAPIKey2 = SECRET_WRITE_APIKEY_2;

unsigned long timer1 = 0, timer2 = 0, timerLCD = 0, timer3, timer4;
float erro=0, errorede=0;
float reqs, oldreqs, scd1temp, tempoffset, scd1hum, scd1co2, scd2temp, scd2hum, scd2co2, t66co2;
bool flagSCD1=false;
float offset = 2.2;//6.38;
int contador = 0;
String dataString = "";

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

int ventilationMode =1;
double amps;
int nivelCH1;
int nivelCH2;
int rssi;

int setch1 = 0;
int setch2 = 0;
double pm_baixo;
double pm_medio;
double pm_alto;
char mqtt_server[40];
char mqtt_port[40] = "8080";
char blynk_token[40] = "YOUR_BLYNK_TOKEN";

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

WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
WiFiEventHandler probeRequestPrintHandler;
WiFiEventHandler probeRequestBlinkHandler;
WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;

float vkimo;
bool blinkFlag;
bool ledState;
volatile time_t _disconnected_time = 0;

String macToString(const unsigned char* mac) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
  Serial.print("Station connected: ");
  Serial.println(macToString(evt.mac));
  //Serial.print(evt.mac.c_str());
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
  Serial.print("Station disconnected: ");
  Serial.println(macToString(evt.mac));
  //Serial.print(evt.mac.c_str());
}

void onProbeRequestPrint(const WiFiEventSoftAPModeProbeRequestReceived& evt) {
  Serial.print("Probe request from: ");
  Serial.print(macToString(evt.mac));
  //Serial.print("IP: ");
  //Serial.print(macToString(evt.remoteIP));
  //Serial.print(evt.mac.c_str());
  Serial.print(" RSSI: ");
  Serial.println(evt.rssi);
}

void onProbeRequestBlink(const WiFiEventSoftAPModeProbeRequestReceived&) {
  // We can't use "delay" or other blocking functions in the event handler.
  // Therefore we set a flag here and then check it inside "loop" function.
  blinkFlag = true;
}

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";
const char* PARAM_FLOAT = "inputFloat";


void setup() {
  Serial.begin(115200);
  
  Serial.print("sizeof(cmdDisable) : ");
  Serial.println(sizeof(cmdDisable));
  softSerial.begin(9600);
  softSerial.write(cmdDisable,sizeof(cmdDisable));
  
  //pinMode(D5,OUTPUT);
  //digitalWrite(D5,HIGH); 

  Wire.begin(D2,D1);  //SDA,SCL  
  if (scd1.begin() == false){
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    flag=false;
  }else{
     Serial.println("Air sensor temp offset...");
        Serial.println(scd1.getTemperatureOffset());

    //setTemperatureOffset(float tempOffset);
    scd1.setTemperatureOffset(offset);
    scd1.setMeasurementInterval((uint16_t) 5);
    /*airSensor.setAltitudeCompensation(5);
    delay(2000);
    airSensor.setAmbientPressure(1000);
    delay(2000);
    airSensor.setForcedRecalibrationFactor(787);*/
  }
    
  //Serial.print("sizeof(cmdDisable) : ");
  //Serial.println(sizeof(cmdDisable));
  //softSerial.begin(9600);
  //softSerial.write(cmdDisable,sizeof(cmdDisable));
    
  initsetup();
  initTFT();
  
  Serial.setDebugOutput(true);
  
  server.serveStatic("/ics", SPIFFS, "/index.html");
  server.serveStatic("/bonjur", SPIFFS, "/bonjur23.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/css", SPIFFS, "/css");
  server.serveStatic("/img", SPIFFS, "/img");
  server.on("/", handleRoot);  
  server.on("/mesures.json", sendMesures);
  server.on("/gpio", updateGpio);
  server.on("/config", sendConfig);
  server.on("/data/", HTTP_GET, handleSentVar);
  //server.on("/niveis",  handleNiveis);
  server.on("/updateNivel", updateNivel); 
  server.on("/niveis.json", sendNiveis);

  //server.on("/tabmesures.json", sendTabMesures);
  //server.on("/postplain/", handlePost);
  //server.on("/feed", handle_feed);
  
  server.begin(); // Web server start
  ThingSpeak.begin(client);  

  timeClient.begin();
  timeClient.update();

  stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);
  stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);
  probeRequestPrintHandler = WiFi.onSoftAPModeProbeRequestReceived(&onProbeRequestPrint);
  probeRequestBlinkHandler = WiFi.onSoftAPModeProbeRequestReceived(&onProbeRequestBlink);

  gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event)  {
    Serial.print("gotIpEventHandler...Station connected, IP: ");
    Serial.println(WiFi.localIP());
  });

  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event){
    _disconnected_time = millis();
    Serial.println("disconnectedEventHandler...Station disconnected");
  });

  //static auto _onDisconnected = WiFi.onStationModeDisconnected([](...) { _disconnected_time = millis(); });

  pm_baixo = readFile2(SPIFFS, "/nivelBaixo.txt").toDouble();
  pm_medio = readFile2(SPIFFS, "/nivelMedio.txt").toDouble();
  pm_alto  = readFile2(SPIFFS, "/nivelAlto.txt").toDouble();
 
}
uint8_t i;
int soma = 0;
void loop() {

  handleTFT();
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
  if(millis()-timer2>1000){   
          if(readHPMA()){
            Serial.println(" ");
          //Serial.print(indexvpm);Serial.println(" ");
          vpm[indexvpm] = pm10-pm1;
          //Serial.print(vpm[indexvpm]);Serial.println(" ");
          indexvpm++;
          
          //Serial.print("flagvpm: ");Serial.println(flagvpm);
          if(flagvpm){
            i=0;            
            while (i < 10) {
              Serial.print(vpm[i]);Serial.print(" ");
                soma += vpm[i];
                i++;
            } 
            meanpm = soma/10.0;
            Serial.println(meanpm);
            soma=0;

            handleFAN();
          }
          if(indexvpm==10){
            indexvpm=0;
            flagvpm=true;
          }
            
          }
          
         
          timer2=millis(); 
  }
  if(millis()-timer3>10000){
          //timeClient.update();    
          //checkOST();
          readSCD();
          //readSPS();
          //readT66();   
          //readHPMA();
          //kimo();  
          printStatus();
          timer3=millis(); 
  }
  if(millis()-timer4>5000){
          handleTFT2();
          timer4=millis(); 
  }

  if ((WiFi.status() != WL_CONNECTED) && millis() - _disconnected_time > 120 * 1000)ESP.restart();
}
void handleFAN(){
  if(meanpm>=pm_alto){
    setch2 = 70;
  }else if(meanpm>=pm_medio){
    setch2 = 60;
  }else if(meanpm>=pm_baixo){
    setch2 = 50;
  }else{
    setch2 = 0;
  }  
}
