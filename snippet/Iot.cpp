#include <Arduino.h>
#include "mqtt_func.h"

#define PUBLISH_DELAY 100000

unsigned long lastMillis = 0;

void setup() {
  Serial.begin(115200); 
  setupCloudIoT();
}

void loop() {
  mqttClient->loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!mqttClient->connected()) {
    connect();
  }

  // publish a message roughly every PUBLISH_DELAY ms.
  if (millis() - lastMillis > PUBLISH_DELAY) {
    lastMillis = millis();

    time_t tnow = time(nullptr);    
    String payload = String("{\"timestamp\":") + String(ctime(&tnow)) +                    
                     String("}");
    publishTelemetry(payload);
  }
}