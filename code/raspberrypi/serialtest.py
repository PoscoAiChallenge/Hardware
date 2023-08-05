import serial
import json
from datetime import datetime

#COM3
ser = serial.Serial(port='COM3', baudrate=9600, timeout=0.1)

while True:
    line = ser.readline().decode('ASCII')
    if line:
        try:
            json_data = json.loads(line)
        except:
            continue
        data_type = json_data.get("type")
        del json_data["type"]
        water_data = json_data.get("WaterPressure")
        #if water_data > 1000:
        #    continue
        json_data["timestamp"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")
        print(json_data)

