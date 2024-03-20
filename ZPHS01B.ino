#include "SoftwareSerial.h"
#include <Wire.h>

#define TX 17
#define RX 16

SoftwareSerial WinsenSerial(RX, TX);

int pm1;
int pm25;
int pm10;
int co2;
int voc;
float temp;
float rh;
float ch2o;
float co;
float o3;
float no2;
int chk;

float a;

void read_winsen() {
  const uint8_t cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  for (int i = 0; i < 9; i++) {
    WinsenSerial.write(cmd[i]);
  }

  byte s[26];
  if (WinsenSerial.available()) {
    for (int i = 0; i < 26; i++) {
      s[i] = WinsenSerial.read();
      //Serial.println(i); 
    }

    pm1 = s[2] * 256 + s[3];
    pm25 = s[4] * 256 + s[5];
    pm10 = s[6] * 256 + s[7];
    co2 = s[8] * 256 + s[9];
    voc = s[10];

    temp = ((s[11] * 256 + s[12]) - 435) * 0.1;
    rh = s[13] * 256 + s[14];
    ch2o = (s[15] * 256 + s[16]) * 0.001;
    co = (s[17] * 256 + s[18]) * 0.1;
    o3 = (s[19] * 256 + s[20]) * 0.01;
    no2 = (s[21] * 256 + s[22]) * 0.01;

    chk = s[25];

    Serial.println(temp);
     
    //  Serial.println( "CO2:" + String(co2)); 
    //  Serial.println( "VOC:" + String(voc));
     
  
  }
  else if(!WinsenSerial.available()){
    Serial.print(".");
    delay(500);
  }
  
}


void setup() {
  // put your setup code here, to run once:
   Serial.begin(115200);
   WinsenSerial.begin(9600);
  while (!WinsenSerial) {
    Serial.println(".");
    delay(100);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  read_winsen();

  delay(5000);
}
