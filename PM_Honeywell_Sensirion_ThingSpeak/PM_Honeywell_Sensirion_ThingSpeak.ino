#include "SoftwareSerial.h"
#include <sps30.h>
#include "T6615.h"
#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h" 
#include "ThingSpeak.h"
#include "secrets.h"
#include <ESP8266WiFi.h>

#define FLAG          0xFF
#define ADDR          0xFE

#define CMD_WARM      0x84
#define CMD_SKIP_WARMUP 0x91
#define CMD_HALT      0x95
#define CMD_READ      0X02
#define SERIAL_NUMBER 0x01
#define CO2PPM        0x03
#define ELEVATION     0x0F

#define STATUSS        0xB6

#define CMD_SGPT_CALIBRATE 0x9B
#define CMD_SET       0X03
#define CMD_VFY       0x02
#define SGPTPPM       0X11

#define CMD_ABC       0xB7
#define ABC_VFY       0x00
#define ABC_ON        0x01
#define ABC_RESET     0x03
#define ABC_OFF       0x02

SoftwareSerial softSerial(12,13); //rx tx
//SoftwareSerial softSerial(5,4); //rx tx
//SoftwareSerial softSerial(13,15); //rx tx
WiFiClient  client;

byte response[82];// = {0,0,0,0,0,0,0};
//uint16_t pm1,pm25,pm4,pm10;

float pm1,pm25,pm4,pm10;

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
unsigned long timer1 = 0, timer2 = 0, timerLCD = 0;;
float erro=0;
float scd1temp, tempoffset, scd1hum, scd1co2, scd2temp, scd2hum, scd2co2, t66co2;

void DisableAutoSend() {
  const char cmd[] = {0x68, 0x01, 0x20, 0x77};
  //SendCmd(cmd, 4);
}

byte cmdDisable[] = {0x68, 0x01, 0x20, 0x77};//HPMA

byte readPPM[]= {FLAG, ADDR, 0x02, CMD_READ, CO2PPM};//Telaire

byte readPM[]= {0x68, 0x01, 0x04, 0x93};//HPMA

void sendRequest(byte packet[],int arrayLen){//, int respLen){  
   while(!softSerial.available()){ 
     softSerial.write(packet,arrayLen);
     delay(50);
     delay(1000);
   }
   int timeout=0; 
   while(softSerial.available()){// < respLen){  
     timeout++;
     if(timeout > 10) 
     Serial.print("Timeout");
     {
     while(softSerial.available()) 
     Serial.print(softSerial.read(),HEX);
         
     break; 
     }
     delay(50);
   }
  /*for (int i=0; i < respLen; i++) response[i] = softSerial.read();
  Serial.print("\t Sensor response:   ");
   for(int i=0;i<respLen;i++){
    Serial.print(response[i],HEX);
    Serial.print(" ");
   }*/
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
          /*
          ThingSpeak.setField(1, m.mc_1p0);
          ThingSpeak.setField(2, m.mc_2p5);
          ThingSpeak.setField(3, m.mc_4p0);
          ThingSpeak.setField(4,m.mc_10p0 );
          */
          ThingSpeak.setField(1, (int) 1);
          ThingSpeak.setField(2, (int) 2);
          ThingSpeak.setField(3, (int) 4);
          ThingSpeak.setField(4,(int) 10 );
          
          ThingSpeak.setField(5,pm1 );
          ThingSpeak.setField(6,pm25 );
          ThingSpeak.setField(7,pm4 );
          ThingSpeak.setField(8,pm10);
          
          int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
          if(x == 200){
            Serial.println("Channel update successful.");
          }
          else{
            Serial.println("Problem updating channel. HTTP error code " + String(x));
            erro++;
          }                             
}

void readHPMA(){
  while(!softSerial.available()){ 
      softSerial.write(readPM,sizeof(readPM));
      delay(50);
      delay(1000);
  }
  
  if(softSerial.available()){
    int i = 0;
    while (softSerial.available() > 0) {
      response[i] = softSerial.read();
      Serial.print(response[i],HEX);
      Serial.print(" ");
      i++;
    }
    Serial.println(" ");
  }  
  for(int x=3;x<11;x++){
    Serial.print(response[x],HEX);
      Serial.print(" ");
  }
  Serial.println(" ");
  pm1 = response[3]*256+response[4];
  pm25 = response[5]*256+response[6];
  pm4 = response[7]*256+response[8];
  pm10 = response[9]*256+response[10];
  Serial.println("\tHPMA");
  Serial.print(" PM1: ");
  Serial.print(pm1);
  Serial.print("\tPM2.5: ");
  Serial.print(pm25);
  Serial.print("\tPM4: ");
  Serial.print(pm4);
  Serial.print("\tPM10: ");
  Serial.print(pm10);
  Serial.println();
  
}
void readHPMA2(){
  Serial.print("readHPMA init ");
  softSerial.flush();
  /*while(!softSerial.available()){ 
      softSerial.write(readPM,sizeof(readPM));
      delay(50);
      //delay(500);
  }*/
  //softSerial.flush();
  
  if(softSerial.available()){
    int i = 0;
    while (softSerial.available() > 0) {
      response[i] = softSerial.read();
      Serial.print(response[i],HEX);
      Serial.print(" ");
      i++;
    }
    Serial.println(" ");
  }
  Serial.print("readHPMA2 ");  
  for(int x=3;x<11;x++){
    Serial.print(response[x],HEX);
      Serial.print(" ");
  }
  Serial.print("\nreadHPMA3 ");
  Serial.println(" ");
  pm1 =  response[4]*256+response[5];
  pm25 = response[6]*256+response[7];
  pm4 = response[8]*256+response[9];
  pm10 = response[10]*256+response[11];
  Serial.println("\tHPMA");
  Serial.print(" PM1: ");
  Serial.print(pm1);
  Serial.print("\tPM2.5: ");
  Serial.print(pm25);
  Serial.print("\tPM4: ");
  Serial.print(pm4);
  Serial.print("\tPM10: ");
  Serial.print(pm10);
  Serial.println();
  
}
void readSPS(){
  Wire.begin(D2,D1);
  do {
    ret = sps30_read_data_ready(&data_ready);
    if (ret < 0) {
      Serial.print("error reading data-ready flag: ");
      Serial.println(ret);
    } else if (!data_ready)
      Serial.print("data not ready, no new measurement available\n");
    else
      break;
    delay(100); /* retry in 100ms */
  } while (1);

  ret = sps30_read_measurement(&m);
  if (ret < 0) {
    Serial.print("error reading measurement\n");
  } else {

    Serial.println("\tSensirion SPS30 Particulate Matter Sensor\n");
    Serial.print("\tMass concentration\n");
    Serial.print("PM 0.3 to 1.0 um: ");
    Serial.print(m.mc_1p0);Serial.print(" ug/m³\n");
    Serial.print("PM 0.3 to 2.5 um: ");
    Serial.print(m.mc_2p5);Serial.print(" ug/m³\n");
    Serial.print("PM 0.3 to 4.0 um: ");
    Serial.print(m.mc_4p0);Serial.print(" ug/m³\n");
    Serial.print("PM 0.3 to 10.0 um: ");
    Serial.print(m.mc_10p0);Serial.print(" ug/m³\n");


    Serial.print("\n\tTotal particles concentration\n");
    Serial.print("NC  0.3 to 0.5 um: ");
    Serial.print(m.nc_0p5);Serial.print(" particles/cm³\n");
    Serial.print("NC  0.3 to 1.0 um: ");
    Serial.print(m.nc_1p0);Serial.print(" particles/cm³\n");
    Serial.print("NC  0.3 to 2.5 um: ");
    Serial.print(m.nc_2p5);Serial.print(" particles/cm³\n");
    Serial.print("NC  0.3 to 4.0 um: ");
    Serial.print(m.nc_4p0);Serial.print(" particles/cm³\n");
    Serial.print("NC  0.3 to 10.0 um: ");
    Serial.print(m.nc_10p0);Serial.print(" particles/cm³\n");

    Serial.print("\n\tParticles concentration per size\n");
    Serial.print("NC  0.3 to 0.5 um: ");
    Serial.print(m.nc_0p5);Serial.print(" particles/cm³\n");
    Serial.print("NC  0.5 to 1.0 um: ");
    Serial.print(m.nc_1p0 -m.nc_0p5);Serial.print(" particles/cm³\n");
    Serial.print("NC  1.0 to 2.5 um: ");
    Serial.print(m.nc_2p5-m.nc_1p0);Serial.print(" particles/cm³\n");
    Serial.print("NC  2.5 to 4.0 um: ");
    Serial.print(m.nc_4p0-m.nc_2p5);Serial.print(" particles/cm³\n");
    Serial.print("NC  4.0 to 10.0 um: ");
    Serial.print(m.nc_10p0-m.nc_4p0);Serial.print(" particles/cm³\n");

    Serial.print("\nTypical partical size: ");
    Serial.println(m.typical_particle_size);
    Serial.println();
  } 
}

void setup() {
  Serial.begin(115200);

  Serial.print("sizeof(cmdDisable) : ");
  Serial.println(sizeof(cmdDisable));
  softSerial.begin(9600);
  softSerial.write(cmdDisable,sizeof(cmdDisable));

  Serial.print("111........... ");
  /*
  Wire.begin(D2,D1);
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
*/
 
  WiFi.mode(WIFI_STA); 
    ThingSpeak.begin(client);

}

void loop() {

  if(millis() - timer1 > 15000){    
          sendThingSpeak();
          timer1 = millis();
          //Serial.println("\nTimer2........");
  }  
   

  if(millis() - timer2 > 5000){ 
          //readSPS();   
          readHPMA2();
          timer2 = millis();
          //Serial.println("\nTimer2........");
  }

}
