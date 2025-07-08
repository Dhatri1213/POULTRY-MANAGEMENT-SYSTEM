
import paho.mqtt.client as mqtt
import requests
import json
import time

# Your Google Apps Script Web App URL
WEB_APP_URL = "https://script.google.com/macros/s/AKfycbzRLjQb1pXzWCdMCAqHAYpeMs6h3I4oLX0zVm5sUl5BXVfdEp1BPZtCeXwNW6vKii9l/exec"

# MQTT Broker IP (Raspberry Pi itself)
MQTT_BROKER = "192.168.137.213"
MQTT_TOPIC = "sensor/data"

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker")
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode())

        # Ensure that 'time' exists in the message
        if "time" in data:
            esp_time = data["time"] 
        else:
            print("Error: 'time' field is missing in the data.")
            return  # Exit if 'time' is missing

        # Proceed only if other fields are available
        if "temp" in data and "smoke" in data:
            temp = data["temp"]
            smoke = data["smoke"]
            pi_received_time = int(time.time())

            latency_esp_to_pi = (pi_received_time - esp_time)  # in seconds
            latency_pi_to_sheets = -1  # Will be measured after sending

            send_time = time.time()
            payload = {
                "temp": temp,
                "smoke": smoke,
                "latency_esp_to_pi": latency_esp_to_pi,
                "latency_pi_to_sheets": latency_pi_to_sheets
            }

            response = requests.post(WEB_APP_URL, json=payload)
            if response.text == "Success":
                latency_pi_to_sheets = time.time() - send_time
                payload["latency_pi_to_sheets"] = latency_pi_to_sheets
                response = requests.post(WEB_APP_URL, json=payload)
                print(f"Data sent. Latency ESP→Pi: {latency_esp_to_pi:.2f}s, Pi→Sheet: {latency_pi_to_sheets:.2f}s")
            else:
                print("Failed to send to sheet:", response.text)
        else:
            print("Error: 'temp' or 'smoke' field is missing in the data.")

    except Exception as e:
        print("Error:", e)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, 1883, 60)
client.loop_forever()
