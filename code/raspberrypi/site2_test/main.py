import serial
import requests
from datetime import datetime
import json

SERIAL_PORT = '/dev/ttyACM0'  # 아두이노와 연결된 시리얼 포트

DATA_API_URL = 'https://api.ye0ngjae.com/data/'
LOG_API_URL = 'https://api.ye0ngjae.com/log/'

def send_data_to_server(data):
    requests.post(DATA_API_URL, json=data)

def send_log_to_server(status, log):
    Log = {
        "tag": status,
        "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "text": log
    }
    requests.post(LOG_API_URL, json=Log)

send_log_to_server("status", "site 2, Hardware System started")

if __name__ == "__main__":
    ser = serial.Serial(SERIAL_PORT, 115200, timeout=0.1)

    while True:
        try:
            line = ser.readline().decode('ASCII').strip()
        except:
            send_log_to_server("error", "site 2, Failed to read serial")
            continue
        if line:
            # 시리얼로부터 받은 JSON 데이터를 파싱
            try:
                json_data = json.loads(line)
                data_type = json_data.get("type")
                data_site = json_data.get("site")
                if data_site != "2":
                    json_data["site"] = "2"
                del json_data["type"]
                json_data["timestamp"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")
            except json.JSONDecodeError as e:
                send_log_to_server("error", "site 2, f{e}")
                continue

            # 데이터 타입이 "log"인 경우 로그 API로 전송
            
            # 데이터 타입이 "sensor"인 경우 센서 데이터 API로 전송
            if data_type == "sensor":
                send_data_to_server(json_data)
            else:
                continue
