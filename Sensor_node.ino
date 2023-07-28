#include <mpu9250.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define rxPin 3
#define txPin 2
#define sound A0
#define water A1
#define vibration A2

MPU9250 mpu;
SoftwareSerial esp01(txPin, rxPin);
const char* ssid = "Your_WiFi_SSID";
const char* password = "Your_WiFi_Password";
const char* serverUrl = "http://your_server_url/data"; // Replace with your server URL

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
}

void loop() {
  int SoundValue = analogRead(sound);
  int WaterValue = analogRead(water);
  int VibrationValue = analogRead(vibration);

  // MPU-9250 Data
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // JSON Data 생성
  StaticJsonDocument<200> jsonData;
  jsonData["site"] = "Your_Site_Name"; // Replace with your site name
  jsonData["timestamp"] = "2023-07-28T12:34:56"; // Replace with current timestamp in ISO 8601 format
  jsonData["vibration"] = VibrationValue;
  jsonData["sound"] = SoundValue;
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

  delay(1000); // 1초마다 데이터 전송
}
