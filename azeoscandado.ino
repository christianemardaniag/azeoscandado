#define BLYNK_TEMPLATE_ID "TMPL6APvdl9JR"
#define BLYNK_TEMPLATE_NAME "azeoscandado"
#define BLYNK_AUTH_TOKEN "QfUzCBes8ELp8fXTT0FEubydiKlpMc3a"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <BlynkSimpleEsp8266.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Servo.h>

#define SENSOR_IN D1
#define ALARM_PIN D2
#define SERVO_PIN D3

#define LOCK_POWER_BL V0
#define SENSITIVITY_BL V1
#define SERVO_BL V2

Servo myservo;
const char auth[] = BLYNK_AUTH_TOKEN;
unsigned long previousMillisDelay = 0;
const long intervalDelay = 500;
unsigned long alarmPreviousMillisDelay = 0;
long alarmIntervalDelay = 50;
uint sensitivity = 3;
uint sensor_power = 1;
uint triggerCtr = 0;

BLYNK_CONNECTED() {
  Blynk.syncAll();
}

BLYNK_WRITE(LOCK_POWER_BL) {
  sensor_power = param.asInt();
  Serial.println("AL: " + String(sensor_power));
}

BLYNK_WRITE(SENSITIVITY_BL) {
  sensitivity = param.asInt();
  Serial.println("SENSITIVITY_BL: " + String(sensitivity));
}

BLYNK_WRITE(SERVO_BL) {
  int servo = param.asInt();
  Serial.println("SERVO: " + String(servo));
  servo ? myservo.write(180) : myservo.write(0);  
}

void setup() {
  WiFiManager wifiManager;
  Serial.begin(115200);
  Serial.println("Booting");
  wifiManager.autoConnect("AZEOS", "CANDADOS");
  Blynk.config(auth);

  WiFi.mode(WIFI_STA);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  myservo.attach(SERVO_PIN);
  pinMode(SENSOR_IN, INPUT);
  pinMode(ALARM_PIN, OUTPUT);
  Blynk.virtualWrite(SENSITIVITY_BL, sensitivity);
  Blynk.virtualWrite(LOCK_POWER_BL, HIGH);
}

void loop() {
  ArduinoOTA.handle();
  Blynk.run();
  if (sensor_power) {
    unsigned long currentMillis = millis();
    unsigned long alarmCurrentMillis = millis();
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
        Blynk.logEvent("alarm", "Your padlock has moved. This could mean that someone has tampered with it or tried to break it.");
        digitalWrite(ALARM_PIN, HIGH);
        delay(200);
        digitalWrite(ALARM_PIN, LOW);
        delay(50);
      } else {
        digitalWrite(ALARM_PIN, LOW);
      }
    }
  } else {
    digitalWrite(ALARM_PIN, LOW);
    triggerCtr = 0;
  }
}
