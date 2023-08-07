#include <Wire.h>
#include <ArduinoJson.h>

const int waterPressurePin = A1;
const int soundPin = A2;

const char* nodeSite = "1"; // 노드 구분을 위한 사이트 이름 (1 또는 2)

void setup() {
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  // 가공된 센서값 얻기
  float soundValue = getSoundValue();
  float waterPressureValue = getWaterPressureValue();

  // JSON Data 생성
  StaticJsonDocument<200> jsonData;
  jsonData["type"] = "sensor";
  jsonData["site"] = nodeSite;
  jsonData["vibration"] = 0.0;
  jsonData["sound"] = soundValue;
  jsonData["WaterPressure"] = waterPressureValue;

  // JSON 데이터를 문자열로 변환
  String jsonString;
  serializeJson(jsonData, jsonString);

  // 시리얼 통신으로 데이터 전송
  Serial.println(jsonString);

  delay(1000); // 1초마다 데이터 전송
}


float getSoundValue() {
  
  return analogRead(soundPin);
}

float getWaterPressureValue() {
  float waterPressure = analogRead(waterPressurePin);
  return analogRead(waterPressure);
}
