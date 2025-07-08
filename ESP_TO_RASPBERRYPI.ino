#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <time.h>

// Wi-Fi and MQTT settings
#define WIFI_SSID "123456"
#define WIFI_PASS "244466666"
#define MQTT_SERVER "192.168.137.213" // Raspberry Pi IP
#define MQTT_PORT 1883
#define MQTT_TOPIC "sensor/data"

// Sensor and device pins
#define DHTPIN 4
#define DHTTYPE DHT11
#define MQ2_PIN A0
#define LED_PIN D1
#define BUZZER_PIN D8
#define ENA D5
#define IN1 D6
#define IN2 D7

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastTime = 0;
const long interval = 5000;  // Send every 5 seconds

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  dht.begin();

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  client.setServer(MQTT_SERVER, MQTT_PORT);

  // Initialize time via NTP
  configTime(0, 0, "pool.ntp.org"); 

  Serial.print("Waiting for NTP time sync");
  while (time(nullptr) < 100000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nTime synced!");

  Serial.println("ESP8266 Ready");
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  if (millis() - lastTime > interval) {
    lastTime = millis();

    float temperature = dht.readTemperature();
    int smoke = analogRead(MQ2_PIN);

    if (!isnan(temperature)) {
      digitalWrite(LED_PIN, (temperature < 20) ? HIGH : LOW);
      digitalWrite(BUZZER_PIN, (smoke > 150) ? HIGH : LOW);

      if (temperature > 25) {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        analogWrite(ENA, 255);
      } else {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
      }

      time_t unixTime = time(NULL);  // Unix time in seconds
      String payload = "{\"time\":" + String(unixTime) +
                       ",\"temp\":" + String(temperature, 2) +
                       ",\"smoke\":" + String(smoke) +
                       "}";

      client.publish(MQTT_TOPIC, payload.c_str());
      Serial.println("MQTT Sent: " + payload);
    } else {
      Serial.println("DHT Read Failed");
    }
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}
