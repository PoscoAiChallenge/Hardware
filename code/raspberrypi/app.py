import flask
import requests
import site1
import site2
import json
from datetime import datetime

app = flask.Flask(__name__)

LOG_API_URL = 'https://api.ye0ngjae.com/log/'

status = True

def send_log_to_server(hardware_tag, log):
    Log = {
        "tag": hardware_tag,
        "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "message": log
    }
    Log = json.dumps(Log)
    requests.post(LOG_API_URL, json=Log)

send_log_to_server("status","Hardware System started")

@app.route('/')
def index():
    #return index.html
    return flask.render_template('index.html')

@app.route('/on', methods=['POST'])
def on():
    global status
    send_log_to_server("status","Hardware System turned on")

    while(status == True):
        if status == False:
            break
        try:
            site1.serial_read()
            site2.serial_read()
        except:
            send_log_to_server("error","Failed to read serial")
            continue

@app.route('/off', methods=['POST'])
def off():
    global status
    status = False
    send_log_to_server("status","Hardware System turned off")
    

if __name__ == "__main__":
    app.run(port=5000, debug=True)