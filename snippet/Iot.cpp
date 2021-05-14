#include <Arduino.h>
#include "mqtt_func.h"
#include <DHT.h>
#define DHTPIN 25     
#define DHTTYPE DHT22 

DHT dht(DHTPIN, DHTTYPE);

#define PUBLISH_DELAY 100000

unsigned long lastMillis = 0;

void get_temp(){
  temp = (round(dht.readTemperature() * 2)) / 2;
  hum = (round(dht.readHumidity() * 2)) / 2;
  
  while (isnan(hum) || isnan(temp))                           //avoid reading errors
      {
        temp = (round(dht.readTemperature() * 2)) / 2;
        hum = (round(dht.readHumidity() * 2)) / 2;
      }

  Serial.println("temp: "+(String)temp);
  Serial.println("hum: "+(String)hum);

}

void setup() {
  Serial.begin(115200); 
  setupCloudIoT();
  dht.begin();

  pinMode(RSTBUTTON, INPUT);  //intialize reset button
  if (digitalRead(RSTBUTTON) == HIGH)    //if button is pressed at startup, reset wifi credentials
  {
    //reset_preferences_wifi();
  }

}

void loop() {
  mqttClient->loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!mqttClient->connected()) {
    connect();
  }  
  
   if (auto_mode)                 //if auto mode is active updates state variable and run auto function
  {
    state = "Attiva";
    //auto_temp();
  }
  else
    state = "Spenta";

  if (active_mode=="deumplus"){
    deumPlusMode();
  }

  if (request_in)
  {
    
    get_temp();    
    String payload = String("{\"temp\": ") + String(temp) + String(",\"hum\": ") + String(hum) + String(",\"tempdes\": ") + tempdes + String(",\"humdes\": ") + humdes + String(",\"mode\": \"") + active_mode + String("\"}");

    
    Serial.println(payload);
    publishTelemetry(payload);
    request_in = false;
  }

  if (request_rec)
  {
    Serial.println("ricevuto comando record");
    //result = enable_receiver_mode();
    //if (result)
    {
     //message = "ok"
    }
    //else
    {
      //message = "failed"     
    }
    String feedback = "ok";    
    Serial.println(feedback);
    publishTelemetry("/record", feedback);
    request_rec = false;
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