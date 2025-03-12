#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include <DHT.h>

// WiFi credentials
const char* ssid     = "Dhatri Laptop";
const char* password = "9177428815";

// Google Script Deployment ID
const char *GScriptId = "AKfycbwS0R7UhlFJHjq5sHqQZ_pFcC4JEvk-5B4oux336EtPKid4miWuYMZOL2pzO0TxBIor";

// Payload setup
String payload_base = "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

// Google Sheets setup
const char* host = "script.google.com";
const int httpsPort = 443;
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;

// Variables to publish
int value0 = 0;
int value1 = 0;
int value2 = 0; // Added to fix 'value2' not declared error

// DHT setup
#define DHTPIN D2    
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Pins
#define MQ2_PIN A0      
#define LED_PIN D1      
#define BUZZER_PIN D8  
#define ENA D5          
#define IN1 D6          
#define IN2 D7  

void setup() {
  Serial.begin(9600);        
  delay(10);
  Serial.println('\n');

  // Connect to WiFi
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(MQ2_PIN, INPUT);

  dht.begin();
  Serial.println("System Initialized...");

  // HTTPS connection setup
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  Serial.print("Connecting to ");
  Serial.println(host);

  bool flag = false;
  for (int i = 0; i < 5; i++) { 
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
       flag = true;
       Serial.println("Connected");
       break;
    } else {
      Serial.println("Connection failed. Retrying...");
    }
  }

  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    delete client;
    client = nullptr;
  }
}

void loop() {
  float temperature = dht.readTemperature();
  int smokeValue = analogRead(MQ2_PIN);

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Smoke Sensor Value: ");
  Serial.println(smokeValue);

  // LED control
  digitalWrite(LED_PIN, (temperature < 20) ? HIGH : LOW);
  Serial.println(temperature < 20 ? "LED ON: Temperature below 20°C" : "LED OFF");

  // Fan control
  if (temperature > 25) {
    digitalWrite(IN1, HIGH);    
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 255);
    Serial.println("Fan ON: Temperature above 25°C");
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    Serial.println("Fan OFF");
  }

  // Buzzer
  digitalWrite(BUZZER_PIN, (smokeValue > 150) ? HIGH : LOW);
  Serial.println(smokeValue > 150 ? "Buzzer ON: Smoke detected!" : "Buzzer OFF");

  Serial.println("----------------------");

  // Prepare data
  value0 = temperature;
  value1 = smokeValue;
  value2 = 0; // Placeholder if you want to add another value

  if (client != nullptr) {
    if (!client->connected()) {
      client->connect(host, httpsPort);
    }

    // Payload to send
    payload = payload_base + "\"" + value0 + "," + value1 + "," + value2 + "\"}";
    Serial.println("Publishing data...");
    Serial.println(payload);

    if (client->POST(url, host, payload)) {
      Serial.println("Data published successfully.");
    } else {
      Serial.println("Error while connecting");
    }
  } else {
    Serial.println("Error creating client object!");
  }

  delay(5000);
}
