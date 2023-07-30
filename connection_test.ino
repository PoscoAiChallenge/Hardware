#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define rxPin 3
#define txPin 2

SoftwareSerial esp01(txPin, rxPin);
const char* ssid = "WiFi_SSID";
const char* password = "Password";
const char* serverUrl = "http://server URL"; // server URL

WiFiUDP udp;
NTPClient ntpClient(udp, "pool.ntp.org");

void setup() {
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

  // 임의의 센서 값을 생성하여 사용
  int SoundValue = random(100, 1000);
  int WaterValue = random(0, 100);
  int VibrationValue = random(0, 100);

  // 사운드 센서 값 (Hz 단위)
  float soundFrequency = SoundValue * 0.1; // 예시: 10 Hz 단위로 변환

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

  delay(5000); // 5초마다 데이터 전송 (테스트용으로 간격을 조정해주세요)
}
