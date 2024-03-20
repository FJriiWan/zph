#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER 256

//----------
#include <TinyGsmClient.h> //https://github.com/vshymanskyy/TinyGSM
#include <ArduinoHttpClient.h> //https://github.com/arduino-libraries/ArduinoHttpClient
//----------

//----------
//const char FIREBASE_HOST[]  = "zphmonitor-default-rtdb.firebaseio.com";
//const String FIREBASE_AUTH  = "1hPFDRyQVf4euPId69zKGsy8WiMwAeWFeaQFDUW2";
const char FIREBASE_HOST[]  = "weather-station-bce94-default-rtdb.firebaseio.com";
const String FIREBASE_AUTH  = "KKUvxyGVgzUJ2jXbI7gxBNRfTUjqr9xwSvSqF9CY";
const String Waktu  = "Waktu/";
const String Sensor = "Sensor/";
const int SSL_PORT          = 443;
//----------


char apn[]  = "internet";
char user[] = "wpa";
char pass[] = "wpa123";


#define rxPin 4
#define txPin 2
HardwareSerial sim800(1);
TinyGsm modem(sim800);

//ZPH
#define RXD2 16
#define TXD2 17
HardwareSerial WinsenSerial(2);
#include <Wire.h>
int pm1,pm25,pm10,voc,chk;
float temp,rh,ch2o,co,o3,no2,co2;
String PM1,PM25,PM10,CO2,VOC,CHK,TEMP,RH,CH2O,CO,O3,NO2;
String sensor;


// RTC
#include "RTClib.h"
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};
String jam, menit, detik;
String tanggal, bulan, tahun,hari;
String Hari;
String waktu;
//---------------------

void rtc_loop()
{
  DateTime now = rtc.now();
  jam     = String(now.hour());
  menit   = String(now.minute());
  detik   = String(now.second());
  tanggal = String(now.day());
  bulan   = String(now.month());
  tahun   = String(now.year());
  hari    = String(daysOfTheWeek[now.dayOfTheWeek()]);
  Hari = String(hari);

      waktu = "{";
      waktu += "\"Tahun\":" + tahun + ","; 
      waktu += "\"Bulan\":" + bulan + ",";
      waktu += "\"Tanggal\":" + tanggal + ",";
      waktu += "\"Jam\":" + jam + ",";
      waktu += "\"Menit\":" + menit + ",";
      waktu += "\"Detik\":" + detik + "";
      waktu += "}";
      
}

void zph_loop()
{
  const uint8_t cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  for (int i = 0; i < 9; i++) {
    WinsenSerial.write(cmd[i]);
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
    PM1 = String(pm1);  PM25 = String(pm25);  PM10 = String(pm10);
    CO2 = String(co2);  VOC = String(voc);    TEMP = String(temp);
    RH = String(rh);    CH2O = String (ch2o); CO = String(co);
    O3 = String(o3);    NO2 = String (no2);   CHK = String(chk);
  }
   
}

TinyGsmClientSecure gsm_client_secure_modem(modem, 0);
HttpClient http_client = HttpClient(gsm_client_secure_modem, FIREBASE_HOST, SSL_PORT);

unsigned long previousMillis = 0;
long interval = 10000;

void setup() {
  Serial.begin(115200);
  sim800.begin(9600, SERIAL_8N1, rxPin, txPin);
  WinsenSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(3000);

  rtc.begin();
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //----------------
   
//  Serial.println("Initializing modem...");
  modem.restart();
  String modemInfo = modem.getModemInfo();
//  Serial.print("Modem: ");
//  Serial.println(modemInfo);
  
  http_client.setHttpResponseTimeout(90 * 1000); //^0 secs timeout
}

void loop() {
  modem.gprsConnect(apn, user, pass);
  http_client.connect(FIREBASE_HOST, SSL_PORT);
  
    if (http_client.connected() && (millis()- previousMillis > 5000 || previousMillis == 0 )) {
      rtc_loop();
      zph_loop();
      PostToFirebase("PATCH", Waktu, waktu, &http_client);
      sensor = "{";
      sensor += "\"PM1\":" + PM1 + ","; 
      sensor += "\"PM10\":" + PM10 + ",";
      sensor += "\"PM25\":" + PM25 + ",";
      sensor += "\"CO2\":" + CO2 + ",";  
      sensor += "\"VOC\":" + VOC + ",";
      sensor += "\"RH\":" + RH + ",";
      sensor += "\"CH2O\":" + CH2O + ",";
      sensor += "\"CO\":" + CO + ",";
      sensor += "\"O3\":" + O3 + ",";
      sensor += "\"NO2\":" + NO2 + ",";
      sensor += "\"CHK\":" + CHK + ",";
      sensor += "\"Temperature\":" + TEMP + "";
      
      sensor += "}";
      PostToFirebase("PATCH", Sensor, sensor, &http_client);
    }
    else{
      http_client.stop();// Shutdown
    }
}

void PostToFirebase(const char* method, const String & path , const String & data, HttpClient* http) {
  String response;
  int statusCode = 0;
  http->connectionKeepAlive(); // Currently, this is needed for HTTPS
  String url;
  if (path[0] != '/') {
    url = "/";
  }
  url += path + ".json";
  url += "?auth=" + FIREBASE_AUTH;
  String contentType = "application/json";
  http->put(url, contentType, data);

  statusCode = http->responseStatusCode();
  response = http->responseBody();
  if (!http->connected()) {
    http->stop();// Shutdown
  }

}
