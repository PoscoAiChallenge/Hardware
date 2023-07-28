#include <mpu9250.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define rxPin 3
#define txPin 2
#define sound A0
#define water A1
#define vibration A2

MPU9250 mpu;
SoftwareSerial esp01(txPin, rxPin);
const char* ssid = "WiFi_SSID";
const char* password = "Password";
const char* serverUrl = "http://api.ye0ngjae.com/data"; // server URL

WiFiUDP udp;
NTPClient ntpClient(udp, "pool.ntp.org");

void setup() {
  Wire.begin();
  Serial.begin(9600);
  esp01.begin(9600);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // NTP 시간 동기화
  ntpClient.begin();
  ntpClient.update();
}

void loop() {
  // 현재 시간 정보 업데이트
  ntpClient.update();
  String currentTimestamp = ntpClient.getFormattedTime();

  int SoundValue = analogRead(sound);
  int WaterValue = analogRead(water);
  int VibrationValue = analogRead(vibration);

  // 사운드 센서 값 (Hz 단위)
  float soundFrequency = SoundValue * 0.1; // 예시: 10 Hz 단위로 변환

  // MPU-9250 Data
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // JSON Data 생성
  StaticJsonDocument<200> jsonData;
  jsonData["site"] = "Site_Name"; // site name
  jsonData["timestamp"] = currentTimestamp; // 현재 시간 정보
  jsonData["vibration"] = VibrationValue;
  jsonData["sound"] = soundFrequency; // 사운드 센서 값 (Hz 단위)
  jsonData["WaterPressure"] = WaterValue;

  // JSON 데이터를 문자열로 변환
  String jsonString;
  serializeJson(jsonData, jsonString);
  Serial.println(jsonString);

  // HTTP POST 요청을 통해 데이터 전송
  WiFiClient client;
  HTTPClient http;

  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error in HTTP request: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  delay(500); // 0.5초마다 데이터 전송
}
