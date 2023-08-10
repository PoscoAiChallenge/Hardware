//----------- Import Library --------------------------------------
#include <Wire.h>
#include <ArduinoJson.h>

//----------- MPU-6050 Measurement & Filtering Range --------------
#define AFS_SEL 2
#define DLPF_SEL 0

//----------- Varuabkes fir gravity -------------------------------
const int MPU_ADDR = 0x68; //I2C Address of the MPU-6050
int AcX, AcY, AcZ; //Accelerometer values
long Cal_AcX, Cal_AcY, Cal_AcZ; //Calibration Values
float GAcX, GAcY, GAcZ; //Convert accelerometer to gravity value
float Min_GAcX=0, Max_GAcX=0, PtoP_GAcX, Min_GAcY=0, Max_GAcY=0, PtoP_GAcY, Min_GAcZ=0, Max_GAcZ=0, PtoP_GAcZ=0; //Finding Min, Max & Peak 
float Min = 0, Max = 0; //Initial value of Min, Max
int cnt; //Count of calibration process
float Grvt_unit; //Gravity value unit
long period, prev_time; // Period of calculation

//------------ Sensor Pin -----------------------------------------
const int waterPressurePin = A1;
const int soundPin = A2;

const char* nodeSite = "1"; // Node Site

//------------ start Function -------------------------------------
void setup() {
  Serial.begin(115200);
  Wire.begin();
  init_MPU6050();
  Gravity_Range_Option();
  Calib_MPU6050(); //Calculating calibration value
}

//------------ loop Function --------------------------------------
void loop() {
  // sensor data read
  ReadDate_MPU6050();
  Calc_Grvt();
  float soundValue = getSoundValue();
  float waterPressureValue = getWaterPressureValue();
  float vibrationValue = GAcZ;

  // json data generate
  StaticJsonDocument<200> jsonData;
  jsonData["type"] = "sensor";
  jsonData["site"] = nodeSite;
  jsonData["vibration"] = vibrationValue;
  jsonData["sound"] = soundValue;
  jsonData["WaterPressure"] = waterPressureValue;

  // Json data to string
  String jsonString;
  serializeJson(jsonData, jsonString);

  // print json data
  Serial.println(jsonString);

  delay(100); // 0.1초마다 데이터 전송
}

//------------ Analog Sensor Read Function -------------------------------
float getSoundValue() {
  
  return analogRead(soundPin);
}

float getWaterPressureValue() {
  float waterPressure = analogRead(waterPressurePin);
  return analogRead(waterPressure);
}

//------------ MPU6050 Initialize Function ------------------------
void init_MPU6050(){
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); //PWR_MGMT_1 register
  Wire.write(0); //set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  //MPU6050 Clock Type
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); //PWR_MGMT_1 register
  Wire.write(0x03); //Selection Clock 'PLL with Z axis gyroscope reference'
  Wire.endTransmission(true);

  //MPU6050 Accelerometer Configuration Setting
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C); // Accelerometer Configuration register
  if(AFS_SEL == 0) Wire.write(0x00); //AFS_SEL=0, Full Scale Range = +/- 2[g]
  else if(AFS_SEL == 1) Wire.write(0x08); //AFS_SEL=1, Full Scale Range = +/- 4 [g]
  else if(AFS_SEL == 2) Wire.write(0x10); //AFS_SEL=2, Full Scale Range = +/- 8 [g]
  else Wire.write(0x18); //AFS_SEL=3, Full Scale Range = +/- 10 [g]
  Wire.endTransmission(true);

  //MPU6050 DLPF(Digital Low Pass Filter)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1A); //DLPF_CFG register
  if(DLPF_SEL == 0) Wire.write(0x00); //Accel BW 260Hz, Delay 0ms / Gyro BW 256Hz, Delay 0.98ms, Fs 8KHz
  else if(DLPF_SEL == 1) Wire.write(0x01); //Accel BW 184Hz, Delay 2ms / Gyro BW 188Hz, Delay 1.9ms, Fs 1KHz
  else if(DLPF_SEL == 2) Wire.write(0x02); //Accel BW 94Hz, Delay 3ms / Gyro BW 98Hz, Delay 2.8ms, Fs 1KHz
  else if(DLPF_SEL == 3) Wire.write(0x03); //Accel BW 44Hz, Delay 4.9ms / Gyro BW 42Hz, Delay 4.8ms, Fs 1KHz
  else if(DLPF_SEL == 4) Wire.write(0x04); //Accel BW 21Hz, Delay 8.5ms / Gyro BW 20Hz, Delay 8.3ms, Fs 1KHz
  else if(DLPF_SEL == 5) Wire.write(0x05); //Accel BW 10Hz, Delay 13.8ms / Gyro BW 10Hz, Delay 13.4ms, Fs 1KHz 
  else Wire.write(0x06); //Accel BW 5Hz, Delay 19ms / Gyro BW 5Hz, Delay 18.6ms, Fs 1KHz
  Wire.endTransmission(true);
}

//------------ Gravity Range Option Function ----------------------
void Gravity_Range_Option(){
  switch(AFS_SEL) { //Selecting Gravity unit value
    case 0:
      Grvt_unit = 16384;
      break;
    case 1:
      Grvt_unit = 8192;
      break;
    case 2:
      Grvt_unit = 4096;
      break;
    case 3:
      Grvt_unit = 3276.8;
      break;
  }
}

//------------ Calibration Function -------------------------------
void Calib_MPU6050(){
  for(int i = 0 ; i < 2000 ; i++){ //Summing Iteration for finding calibration value
    ReadDate_MPU6050(); //Read Accelerometer data

    delay(10);

    //Sum Data
    Cal_AcX += AcX;
    Cal_AcY += AcY;
    Cal_AcZ += AcZ;
  }
  
  //Average Data
  Cal_AcX /= 2000;
  Cal_AcY /= 2000;
  Cal_AcZ /= 2000;
  
}

void ReadDate_MPU6050() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x38); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  // Wire.requestsFrom(MPU_ADDR,14,true); // request a total fo 14 registers
  Wire.requestFrom(MPU_ADDR, 6, true); // request a total of 6 registers
  AcX=Wire.read()<<8|Wire.read(); //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY=Wire.read()<<8|Wire.read(); //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read(); //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
}

//------------ Calculating Gravity Function -----------------------
void Calc_Grvt() {
  AcX = (AcX - Cal_AcX); // Calibrated Acclerometer value
  AcY = (AcY - Cal_AcY);
  AcZ = (AcZ - Cal_AcZ);

  GAcX = AcX / Grvt_unit; // Converting the Calibrated value to Gravity Value
  GAcY = AcY / Grvt_unit;
  GAcZ = AcZ / Grvt_unit;

  //Calculating Min, Max & Peak to Peak of Gravity 
  
  Min_GAcX = min(Min_GAcX, GAcX);
  Max_GAcX = max(Max_GAcX, GAcX);

  Min_GAcY = min(Min_GAcY, GAcY);
  Max_GAcY = max(Max_GAcY, GAcY);

  Min_GAcZ = min(Min_GAcZ, GAcZ);
  Max_GAcZ = max(Max_GAcZ, GAcZ);

}