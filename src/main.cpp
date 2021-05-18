#include <Arduino.h>
#include "mqtt_func.h"
#include <DHT.h>
#include "IRstorecommand.h"
#include "wifi_cred_config.h"
#include "manage_temp.h"


void setup() {
  pinMode(33, OUTPUT);  
  digitalWrite(33, HIGH);

  Serial.begin(115200);  
  pinMode(RSTBUTTON, INPUT);  //intialize reset button
  if (digitalRead(RSTBUTTON) == HIGH)    //if button is pressed at startup, reset wifi credentials
  {
    reset_preferences_wifi();
  }

  check_credentials();

  if (wifi_configured == true)
  {
    setupCloudIoT();
    dht.begin();
    Serial.println("Working mode");    
    preferences.begin("storedcommand", false);
    command_stored = preferences.getBool("command_stored", false);
    preferences.end();   
  }
  else
  {
  WiFi.disconnect();
  WiFi.softAP(ssid_ap, password_ap);
  Serial.println();
  Serial.print("Server IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Server MAC address: ");
  Serial.println(WiFi.softAPmacAddress());
  server.on("/", HTTP_GET, handleIndex);
  server.on("/", HTTP_POST, handleConf);
  server.begin();
  Serial.println("Server listening");
  }

}

void loop() {

  if (!wifi_configured )  
  { 
    server.handleClient();
  }
  else
  {
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

    
  }
}
