#include <Client.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>
#include "ciotc_config.h" 
#include "variables.h"
#include "IReditandsend.h"


int c= 0;

void messageReceived(String &topic, String &payload)              // manage incoming commands in subfolders
{
  Serial.println("incoming: " + topic + " - " + payload);
 
  if (topic == "/devices/"+(String)device_id+"/commands/pull")
  {

    request_in = true;
  }
  
   if (topic == "/devices/"+(String)device_id+"/commands/mode")   //TODO add cool, deum and deum+ mode
  {
    
    if (payload == "off"){      
      active_mode="none";
      send_signal(TEMPMIN,"deumplus",false);
    }
    else{
      StaticJsonDocument<128> desired_conf;
      DeserializationError error = deserializeJson(desired_conf, payload);

      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      
      tempdes = desired_conf["tempdes"]; 
      humdes = desired_conf["humdes"]; 

      if (desired_conf["mode"] == "cool") 
      {                  
        active_mode="cool";
        send_signal(tempdes,active_mode,true);
      }
      if (desired_conf["mode"] == "deum")
      {
        active_mode="deum";
        send_signal(tempdes,active_mode,true);
      }
      if (desired_conf["mode"] == "deumplus")
        active_mode="deumplus";
    }  
  }

  if (topic == "/devices/"+(String)device_id+"/commands/power")
  {
    if (payload == "off"){
      active_mode="none";
      send_signal(TEMPMIN,"deumplus",false); 
    }
    
  }

  if (topic == "/devices/"+(String)device_id+"/commands/record")   //to enable ir receier mode
  {
    
    request_rec=true;
    
  }
  
}

void deumPlusMode(){                            //task to repeat when deum+ is active (if active_mode=="deum+" in main loop)
  if(hum < humdes && actual_state=="on")
  {
    send_signal(TEMPMIN,"deumplus",false);
    actual_state="off";
  }
  if (actual_state=="off" && hum > (humdes + 10))  //10% tollerance
  {
    send_signal(TEMPMIN,"deumplus",true);
    actual_state="on";
  }
}


///////////////////////////////

// Initialize WiFi and MQTT for this board
Client *netClient;
CloudIoTCoreDevice *device;
CloudIoTCoreMqtt *mqtt;
MQTTClient *mqttClient;
unsigned long iat = 0;
String jwt;

///////////////////////////////
// Helpers specific to this board
///////////////////////////////
String getDefaultSensor(){
  return "Wifi: " + String(WiFi.RSSI()) + "db";
}

String getJwt(){
  iat = time(nullptr);
  Serial.println("Refreshing JWT");
  jwt = device->createJWT(iat, jwt_exp_secs);
  return jwt;
}

void setupWifi(){
  Serial.println("Starting wifi");

  WiFi.mode(WIFI_STA);
  // WiFi.setSleep(false); // May help with disconnect? Seems to have been removed from WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED){
    delay(100);
  }

  configTime(0, 0, ntp_primary, ntp_secondary);
  Serial.println("Waiting on time sync...");
  while (time(nullptr) < 1510644967){
    delay(10);
  }
}

void connectWifi(){
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
  }
}

///////////////////////////////
// Orchestrates various methods from preceeding code.
///////////////////////////////
bool publishTelemetry(String data){
  return mqtt->publishTelemetry(data);
}

bool publishTelemetry(const char *data, int length){
  return mqtt->publishTelemetry(data, length);
}

bool publishTelemetry(String subfolder, String data){
  return mqtt->publishTelemetry(subfolder, data);
}

bool publishTelemetry(String subfolder, const char *data, int length){
  return mqtt->publishTelemetry(subfolder, data, length);
}

void connect(){
  connectWifi();
  mqtt->mqttConnect();
}

void setupCloudIoT(){
  device = new CloudIoTCoreDevice(
      project_id, location, registry_id, device_id,
      private_key_str);

  setupWifi();
  netClient = new WiFiClientSecure();
  mqttClient = new MQTTClient(512);
  mqttClient->setOptions(180, true, 1000); // keepAlive, cleanSession, timeout
  mqtt = new CloudIoTCoreMqtt(mqttClient, netClient, device);
  mqtt->setUseLts(true);
  mqtt->startMQTT();
}