#define BLYNK_TEMPLATE_ID "TMPL6APvdl9JR"
#define BLYNK_TEMPLATE_NAME "azeoscandado"
#define BLYNK_AUTH_TOKEN "QfUzCBes8ELp8fXTT0FEubydiKlpMc3a"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#define SENSOR_IN D1
#define ALARM_PIN D2
#define SWITCH V0
#define SENSITIVITY V1

const char auth[] = BLYNK_AUTH_TOKEN;

unsigned long previousMillisDelay = 0;
const long intervalDelay = 500;
uint sensitivity = 3;
uint sensor_power = 0;
uint triggerCtr = 0;

BLYNK_CONNECTED() {
  Blynk.syncAll();
}

BLYNK_WRITE(SWITCH) {
  sensor_power = param.asInt();
  Serial.println("ALARM: " + String(sensor_power));
}

BLYNK_WRITE(SENSITIVITY) {
  sensitivity = param.asInt();
  Serial.println("SENSITIVITY: " + String(sensitivity));
}

void setup() {
  WiFiManager wifiManager;
  wifiManager.autoConnect("AZEOS", "CANDADOS");
  Serial.begin(9600);
  Serial.println("Starting...");
  Blynk.config(auth);
  pinMode(SENSOR_IN, INPUT);
  pinMode(ALARM_PIN, OUTPUT);
  Blynk.virtualWrite(SENSITIVITY, sensitivity);
  Blynk.virtualWrite(SWITCH, HIGH);
}

void loop() {
  Blynk.run();
  if (sensor_power) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillisDelay >= intervalDelay) {
      previousMillisDelay = currentMillis;
      int read = digitalRead(SENSOR_IN);
      if (read == HIGH) {
        if (triggerCtr < 10) triggerCtr++;
        Serial.println("Vibrating...");
      } else {
        if (triggerCtr > 0) triggerCtr--;
        Serial.println("No vibration");
      }
      Serial.println(triggerCtr);
      if (triggerCtr >= sensitivity) {
        Blynk.logEvent("alarm", "Hey hey hey");
        digitalWrite(ALARM_PIN, HIGH);
      } else {
        digitalWrite(ALARM_PIN, LOW);
      }
    }
  } else {
    digitalWrite(ALARM_PIN, LOW);
    triggerCtr = 0;
  }
}