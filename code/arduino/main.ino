#include <Wire.h>
#include "MPU9250.h"
#include <ArduinoJson.h>

MPU9250 mpu;

const int vibrationPin = A4;
const int soundPin = A1;
const int waterPressurePin = A0;


const char* nodeSite = "1"; // 노드 구분을 위한 사이트 이름 (1 또는 2)

const int numSamples = 10; //정확도 향상을 위한 샘플 갯수

void setup() {
  Wire.begin();
  Serial.begin(9600);
}

void loop() {
  // MPU-9250 Data
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // 가공된 센서값 얻기
  float vibrationValue = getVibrationValue();
  float soundValue = getSoundValue();
  float waterPressureValue = getWaterPressureValue();

  // JSON Data 생성
  StaticJsonDocument<200> jsonData;
  jsonData["type"] = "sensor";
  jsonData["site"] = nodeSite;
  jsonData["timestamp"] = getCurrentDatetime();
  jsonData["vibration"] = vibrationValue;
  jsonData["sound"] = soundValue;
  jsonData["WaterPressure"] = waterPressureValue;

  // JSON 데이터를 문자열로 변환
  String jsonString;
  serializeJson(jsonData, jsonString);

  // 시리얼 통신으로 데이터 전송
  Serial.println(jsonString);

  delay(1000); // 1초마다 데이터 전송
}

float getVibrationValue() {
  // 진동 값을 측정하여 배열에 저장
  int samples[numSamples];
  for (int i = 0; i < numSamples; i++) {
    // 가속도 센서 값 읽기
    int16_t ax, ay, az;
    accelgyro.getAcceleration(&ax, &ay, &az);

    // 가속도 값의 크기 계산 (피타고라스 정리)
    float magnitude = sqrt(ax * ax + ay * ay + az * az);

    // 진동 값으로 사용할 값으로 변환 (여러 방법 중 선택 가능)
    samples[i] = magnitude;

    delay(10); // 측정 간격을 조절하여 샘플 값을 얻습니다.
  }

  // 배열에 저장된 샘플 값을 정렬 (오름차순)
  for (int i = 0; i < numSamples - 1; i++) {
    for (int j = i + 1; j < numSamples; j++) {
      if (samples[i] > samples[j]) {
        int temp = samples[i];
        samples[i] = samples[j];
        samples[j] = temp;
      }
    }
  }

  // 배열에서 최소값과 최대값을 제외한 나머지 값들의 평균을 구함
  int sum = 0;
  for (int i = 1; i < numSamples - 1; i++) {
    sum += samples[i];
  }
  float averageValue = (float) sum / (numSamples - 2);

  // TODO: 측정 값에 대한 민감도 향상 등의 추가 가공 로직

  return averageValue;
}


float getSoundValue() {
  // TODO: 사운드 센서 값 가공 로직 구현
  // 민감도 향상 및 노이즈 제거를 위한 처리
  return analogRead(soundPin);
}

float getWaterPressureValue() {
  float waterPressure = analogRead(waterPressurePin);
  return analogRead(waterPressure);
}

String getCurrentDatetime() {
  // TODO: 현재 시간 정보를 문자열로 변환하여 반환
  // 라이브러리를 사용하거나 시간 정보를 직접 구하는 로직 등으로 구현 가능
  return "2023-07-30 14:30:23"; // 예시: yyyy-mm-dd hh:mm:ss 형식의 문자열
}
