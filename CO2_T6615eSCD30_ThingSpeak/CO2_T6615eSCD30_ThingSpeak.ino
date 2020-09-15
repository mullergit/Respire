#include "T6615.h"
#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h" 
#include "ThingSpeak.h"
#include "secrets.h"
#include <ESP8266WiFi.h>

#define SDA_2 D5
#define SCL_2 D6

SoftwareSerial softSerial(13,15); 

T66 t66;
SCD30 scd1, scd2;
//SCD30 as2;

bool flagSCD1=true, flagSCD2=true;
byte count = 0;
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
unsigned long timer1 = 0, timer2 = 0;
float erro=0;
float scd1temp, scd1hum, scd1co2, scd2temp, scd2hum, scd2co2, t66co2;

void readSCD(){
  if(flagSCD1){
    Wire.begin(D2,D1);
    if (scd1.dataAvailable()){
        scd1temp = scd1.getTemperature();
        scd1hum = scd1.getHumidity();
        scd1co2 = scd1.getCO2();
        
        Serial.println("SCD1.....");
        Serial.print("co2(ppm):");Serial.print(scd1co2);    
        Serial.print(" temp(C):");Serial.print(scd1temp, 1);    
        Serial.print(" humidity(%):");Serial.print(scd1hum, 1);    
        Serial.println();
        Serial.println("scd1.getTemperatureOffset()");
    Serial.println(scd1.getTemperatureOffset());
      }
      else
        Serial.println("Waiting for new data Wire");    
  }
  if(flagSCD2){
    Wire.begin(D5,D6);
    if (scd2.dataAvailable()){
        scd2temp = scd2.getTemperature();
        scd2hum = scd2.getHumidity();
        scd2co2 = scd2.getCO2();
        
        Serial.println("SCD2.....");
        Serial.print("co2(ppm):");Serial.print(scd2co2);    
        Serial.print(" temp(C):");Serial.print(scd2temp, 1);    
        Serial.print(" humidity(%):");Serial.print(scd2hum, 1);    
        Serial.println();
        Serial.println("scd2.getTemperatureOffset()");
    Serial.println(scd2.getTemperatureOffset());
      }
      else
        Serial.println("Waiting for new dataTwoWire");    
  }
}
void readT66(){
  //Serial.print("\n readSTATUS()...");
   t66.readSTATUS();
   
   if(t66.response[3]==0){
      Serial.print("\n T66 CO2 ppm:");
      t66.readPPM();
      t66co2 = t66.getPPMorSGPT(t66.response);
      Serial.println(t66co2);
   }
}
void sendThingSpeak(){
  Serial.println("\t Enviando ThingSpeak......................................................");

          if(WiFi.status() != WL_CONNECTED){
            Serial.print("Attempting to connect to SSID: ");
            Serial.println(SECRET_SSID);
            while(WiFi.status() != WL_CONNECTED){
              WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
              Serial.print(".");
              delay(5000);     
            } 
            Serial.println("\nConnected.");
          }               
          
          ThingSpeak.setField(1, scd1co2);
          ThingSpeak.setField(2, scd2co2);
          ThingSpeak.setField(3, t66co2);
          ThingSpeak.setField(4, scd1temp);
          ThingSpeak.setField(5,scd2temp );
          ThingSpeak.setField(6,scd1hum );
          ThingSpeak.setField(7,scd2hum );
          ThingSpeak.setField(8,erro );
          
          int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
          if(x == 200){
            Serial.println("Channel update successful.");
          }
          else{
            Serial.println("Problem updating channel. HTTP error code " + String(x));
            erro++;
          }                             
}

void setup(){
  Serial.begin(115200);
  t66.begin(softSerial);
  Wire.begin(D2,D1);
  if (scd1.begin() == false){
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    flagSCD1=false;
  }else{
    Serial.println("scd1.getTemperatureOffset()");
    Serial.println(scd1.getTemperatureOffset());
    scd1.setTemperatureOffset((float) 2.6);
    Serial.println("scd1.getTemperatureOffset()");
    Serial.println(scd1.getTemperatureOffset());
    //scd1.setForcedRecalibrationFactor(820);
  }
  Wire.begin(D5,D6);
    if (scd2.begin() == false){
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    flagSCD2=false;
  }else{
    Serial.println("scd2.getTemperatureOffset()");
    Serial.println(scd2.getTemperatureOffset());
    scd2.setTemperatureOffset((float) 1.4);
    Serial.println("scd2.getTemperatureOffset()");
    Serial.println(scd2.getTemperatureOffset());
    //scd2.setForcedRecalibrationFactor(810);
  }

  
  WiFi.mode(WIFI_STA); 
    ThingSpeak.begin(client);

    delay(3000);
    readSCD();
    readT66();
    //Serial.println("Enviando 1° pacote para ThingSpeak...");
    //sendThingSpeak();
}

void loop(){
  
   if(millis() - timer1 > 10000){  
          readSCD();          
          readT66();
          timer1 = millis();
    }
  
  if(millis() - timer2 > 600000){    
          sendThingSpeak();
          timer2 = millis();
    }
    
  delay(10000);
}
