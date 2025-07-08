#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include <DHT.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

const char* ssid = "123456";
const char* password = "244466666";
const char *GScriptId = "AKfycbz9t1Z2FeNeg063f3rNhftRC109ZDmOyhd1KiEu-7i60VWvRbXr12UAFc-R6I6GFHko";

String payload_base = "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";
const char* host = "script.google.com";
const int httpsPort = 443;
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;

#define DHTPIN D2    
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define MQ2_PIN A0
#define LED_PIN D1
#define BUZZER_PIN D8
#define ENA D5
#define IN1 D6
#define IN2 D7

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected.");

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(MQ2_PIN, INPUT);
  dht.begin();

  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(false);
  client->setContentTypeHeader("application/json");

  for (int i = 0; i < 5 && !client->connected(); i++) {
    if (client->connect(host, httpsPort)) {
      Serial.println("HTTPS connected");
      break;
    }
    delay(1000);
  }
}

bool sendData(float temperature, int smokeValue, int latency) {
  if (!client->connected()) {
    client->connect(host, httpsPort);
  }
  payload = payload_base + "\"" + temperature + "," + smokeValue + "," + latency + "\"}";
  Serial.println("Sending: " + payload);
  return client->POST(url, host, payload);
}

void loop() {
  float temp = dht.readTemperature();
  int smoke = analogRead(MQ2_PIN);
  digitalWrite(LED_PIN, (temp < 20) ? HIGH : LOW);
  digitalWrite(BUZZER_PIN, (smoke > 150) ? HIGH : LOW);
  if (temp > 25) {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); analogWrite(ENA, 255);
  } else {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  }

  // ✅ Start timer before sending
  unsigned long startTime = millis();
  bool success = sendData(temp, smoke, -1);  //Response From GOOGLE SHEETS through APPSCRIPT post
  unsigned long endTime = millis();
  
  if (success) {
    int latency = endTime - startTime; // in ms
    sendData(temp, smoke, latency);    //  Second send with real latency
  } else {
    Serial.println("Initial send failed.");
  }

  delay(6000); // 6 sec before next reading
}
