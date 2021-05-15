#include <Arduino.h>
#include "mqtt_func.h"
#include <DHT.h>
#include "IRstorecommand.h"

#define DHTPIN 25     
#define DHTTYPE DHT22 

DHT dht(DHTPIN, DHTTYPE);

#define PUBLISH_DELAY 100000

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

  preferences.begin("storedcommand", false);
  command_stored = preferences.getBool("command_stored", false);
  preferences.end();

}

void loop() {
  mqttClient->loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!mqttClient->connected()) {
    connect();
  }  
     

  if (active_mode=="deumplus"){
    deumPlusMode();
  }

  if (request_in)
  {
    
    get_temp();    
    String payload = String("{\"temp\": ") + String(temp) + String(",\"hum\": ") + String(hum) + String(",\"tempdes\": ") + tempdes + String(",\"humdes\": ") + humdes + String(",\"mode\": \"") + active_mode + String("\",\"command_stored\": \"")+ String(command_stored) + String("\"}");

    
    Serial.println(payload);
    publishTelemetry(payload);
    request_in = false;
  }

  if (request_rec)
  {
    Serial.println("ricevuto comando record");
    String feedback = store_command();         
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
