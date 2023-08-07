from flask import Flask, render_template, request
import requests
import json
import threading
import site1
import site2
from datetime import datetime

app = Flask(__name__)

LOG_API_URL = 'https://api.ye0ngjae.com/log/'

status = False

def send_log_to_server(hardware_tag, log):
    Log = {
        "tag": hardware_tag,
        "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "message": log
    }
    Log = json.dumps(Log)
    requests.post(LOG_API_URL, json=Log)

send_log_to_server("status", "Hardware System started")

@app.route('/')
def index():
    # return index.html
    return render_template('index.html')

# 백그라운드에서 실행할 함수
def serial_read_task():
    while True:
        if not status:
            break
        try:
            site1.serial_read()
            site2.serial_read()
        except:
            send_log_to_server("error", "Failed to read serial")
            continue  

@app.route('/on', methods=['POST'])
def turn_on():
    global status
    print("Hardware System turned on")

    if not status:
        status = True
        send_log_to_server("status", "Hardware System turned on")

        # 백그라운드 스레드 시작
        serial_thread = threading.Thread(target=serial_read_task)
        serial_thread.start()

    return "Hardware System turned on"

@app.route('/off', methods=['POST'])
def turn_off():
    global status
    status = False
    send_log_to_server("status", "Hardware System turned off")
    return "Hardware System turned off"

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000, debug=True)
