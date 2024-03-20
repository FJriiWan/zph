

//#include<WiFi.h>
//#include<Firebase_ESP_Client.h>
//#include "addons/TokenHelper.h"
//#include "addons/RTDBHelper.h"

//#define SSID "PC-INSTRUMEN"
//#define PASS "fajrinih"
//#define API_KEY "AIzaSyAT05Bss-PjIKLvUHwe0MW-Gq39q8BnfgQ"
//#define URL "https://zphmonitor-default-rtdb.firebaseio.com/"
//
//FirebaseData fbdo;
//FirebaseAuth auth;
//FirebaseConfig config;
// 
// unsigned long sendDataPrevMillis = 0;
// bool singupOK = false;

// RTC
#include "RTClib.h"
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};
int jam, menit, detik;
int tanggal, bulan, tahun;
String hari;
//---------------------


// ZPHS
//#include "SoftwareSerial.h"
#include <SoftwareSerial.h>
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
//--------------------


void read_winsen() {
  const uint8_t cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  for (int i = 0; i < 9; i++) {
    WinsenSerial.write(cmd[i]);
    //Serial.println (i);
  }
  byte s[26];
  if (WinsenSerial.available()) {
    for (int i = 0; i < 26; i++) {
      s[i] = WinsenSerial.read();
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

  //  Serial.println( "Temperature:" + String(temp));
    Serial.println( "CO2:" + String(co2)); 
    //Serial.println( "VOC:" + String(pm25));

  }
  else if(!WinsenSerial.available()){
    Serial.print("Error");
    delay(500);
  }
}

void read_rtc(){
  DateTime now = rtc.now();
  jam     = now.hour();
  menit   = now.minute();
  detik   = now.second();
  tanggal = now.day();
  bulan   = now.month();
  tahun   = now.year();
  hari    = daysOfTheWeek[now.dayOfTheWeek()];
  Serial.println(String() + hari + ", " + tanggal + "-" + bulan + "-" + tahun);
  Serial.println(String() + jam + "-" + menit + "-" + tahun);
  Serial.println();
  delay(1000);
}


void setup() {
  Serial.begin(115200);

//  //WiFi
//  WiFi.begin(SSID,PASS);
//  while(WiFi.status()  != WL_CONNECTED){
//    Serial.print("."); delay(300);
//  } 
//  Serial.println(WiFi.localIP());
//  config.api_key = API_KEY;
//  config.database_url = DATABASE_URL;
//  if(Firebase.sginUp(&config, &auth, "", "")){
//    Serial.println("signUp OK");
//    signupOK = true;
//  }else {
//    Serial.printf("%s\n", config.signer.signupError.message.c_str());
//  }
//  config.token_status_callback = tokenStatusCallback;
//  Firebase.begin(&config, &auth);
//  Firebase.reconnectWiFi(true);
//  //-------

  //ZPHS
   WinsenSerial.begin(9600);
   while (!WinsenSerial) {
    Serial.println(".");
    delay(100);
    }
  //---------------
  //RTC
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //----------------
}

void loop() {
  // put your main code here, to run repeatedly:
    read_winsen();
    read_rtc();
    delay(2000);
  // if(Firebase.ready() && signupOK && (millis() - sendDAtaPrevMillis > 5000 || sendDataPrevMillis == 0)){
  //   read_winsen();
  //   read_rtc();
  //   delay(1000);
    //    if (Firebase.RTDB.setString(&fbdo, "Sensor/CO2", co2 ))
  // }
}
