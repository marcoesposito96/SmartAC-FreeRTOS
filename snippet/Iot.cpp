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

  temp= 25.5;                       //JUST FOR DEBUGGING
  hum= 24;
  
   if (auto_mode)                 //if auto mode is active updates state variable and run auto function
  {
    state = "Attiva";
    //auto_temp();
  }
  else
    state = "Spenta";

  if (request_in)
  {
    // if (output_status = "off") output_status = "on";
    // else output_status = "off";
    
    String payload = String("{\"temp\": ") + String(temp) + String(",\"hum\": ") + String(hum) + String(",\"auto\": \"") + state + String("\",\"thr\": \"") + thr + String("\",\"status\": \"")+ output_status + String("\"}");
    Serial.println(payload);
    publishTelemetry(payload);
    request_in = false;
  }

  // publish a message roughly every PUBLISH_DELAY ms.
  // if (millis() - lastMillis > PUBLISH_DELAY) {
  //   lastMillis = millis();

  //   time_t tnow = time(nullptr);    
  //   String payload = String("{\"timestamp\":") + String(ctime(&tnow)) +                    
  //                    String("}");
  //   publishTelemetry(payload);
  // }
}