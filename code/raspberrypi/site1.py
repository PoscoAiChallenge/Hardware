import serial
import requests
from datetime import datetime
import json

SERIAL_PORT = '/dev/ttyACM1'  # 아두이노와 연결된 시리얼 포트

DATA_API_URL = 'https://api.ye0ngjae.com/data/'
LOG_API_URL = 'https://api.ye0ngjae.com/log/'

def send_data_to_server(data):
    response = requests.post(DATA_API_URL, json=data)
    if response.status_code == 201:
        print(f"Data sent successfully: {data}")
    else:
        print(f"Failed to send data: {data}")

def send_log_to_server(log):
    log = {
        "tag": "error",
        "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "text": log
    }
    log = json.dumps(log)
    response = requests.post(LOG_API_URL, json=log)
    if response.status_code == 201:
        print(f"Log sent successfully: {log}")
    else:
        print(f"Failed to send log: {log}")

def serial_read():
    ser = serial.Serial(SERIAL_PORT, 9600, timeout=0.1)
    try:
        line = ser.readline().decode('ASCII').strip()
    except:
        send_log_to_server({"type": "log", "message": "Failed to read serial"})

    if line:
            # 시리얼로부터 받은 JSON 데이터를 파싱
        try:
            json_data = json.loads(line)
            data_type = json_data.get("type")
            del json_data["type"]
            data_site = json_data.get("site")
            if data_site == "2":
                json_data["site"] = "1"            
            json_data["timestamp"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        except json.JSONDecodeError as e:
            print(f"Failed to parse JSON: {e}")
            send_log_to_server({"type": "log", "message": f"Failed to parse JSON: {e}"})

            # 데이터 타입이 "log"인 경우 로그 API로 전송
            
            # 데이터 타입이 "sensor"인 경우 센서 데이터 API로 전송
        if data_type == "sensor":
            send_data_to_server(json_data)
        else:
            send_log_to_server(json_data)
