
#define BLYNK_AUTH_TOKEN "IVhIZeGFRgGgvqKg0gkQLAkIuHJlRsJo"
#define BLYNK_TEMPLATE_ID "TMPL3BBMs8Opv"
#define BLYNK_TEMPLATE_NAME "Poultry Management"
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>



#define WIFI_SSID "Dhatri Laptop"    
#define WIFI_PASS "9177428815"
#define DHTPIN 4 // GPIO4 (D2)
#define DHTTYPE DHT11

#define MQ2_PIN A0      
#define LED_PIN D1      
#define BUZZER_PIN D8  
#define ENA D5          
#define IN1 D6          
#define IN2 D7

DHT dht(DHTPIN, DHTTYPE);
unsigned long lastTime = 0;
const long interval = 2000; // 2 seconds

void setup() {
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");

    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(MQ2_PIN, INPUT);

    dht.begin();
    delay(2000); // Stabilization delay
    Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
    Serial.println("System Initialized...");
}

void loop() {
    Blynk.run();

    if (millis() - lastTime >= interval) {
        lastTime = millis();

        float temperature = dht.readTemperature();
        int smokeValue = analogRead(MQ2_PIN);

        if (isnan(temperature)) {
            Serial.println("Failed to read from DHT sensor!");
        } else {
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.println(" °C");
            Blynk.virtualWrite(V0, temperature);
        }

        Serial.print("Smoke Sensor Value: ");
        Serial.println(smokeValue);
        Blynk.virtualWrite(V1, smokeValue);

        // LED control
        digitalWrite(LED_PIN, (temperature < 20) ? HIGH : LOW);

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

        // Buzzer control
        digitalWrite(BUZZER_PIN, (smokeValue > 150) ? HIGH : LOW);

        Serial.println("----------------------");

        // WiFi/Blynk check (Optional, if facing disconnection)
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi Disconnected, reconnecting...");
            WiFi.begin(WIFI_SSID, WIFI_PASS);
            while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
            }
            Serial.println("\nWiFi Reconnected!");
        }
    }
}


