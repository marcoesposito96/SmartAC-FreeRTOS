#include <Client.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>
#include "ciotc_config.h" 
#include "variables.h"
#include "IReditandsend.h"
#define TIMEOUT_RECONNECT 5000



TaskHandle_t task_WarningLed_hand;
TaskHandle_t task_MessageHandler_hand;
extern SemaphoreHandle_t warning_led;
extern SemaphoreHandle_t update_sensor;
extern SemaphoreHandle_t sensor_ack;
extern SemaphoreHandle_t pull, record, deumplus, stopdeumplus, mutex, mutexmqtt;


int passingLed;

unsigned long startTime;

struct messageq {
  float tempdes=TEMPMIN;
  float humdes=50;
  String command;
  bool update=0;
} mess, com;
//struct messageq *pxmess;

QueueHandle_t messageQueue_hand = xQueueCreate(QUEUELEN,sizeof(messageq));

void messageReceived(String &topic, String &payload)              // manage incoming commands in subfolders
{
  Serial.println("incoming: " + topic + " - " + payload);
  mess.command="";
  
 
  if (topic == "/devices/"+(String)device_id+"/commands/pull")
  {
    mess.command = "pull";
  }

  if (topic == "/devices/"+(String)device_id+"/commands/record")
  {
    mess.command = "record";
  }

  if (topic == "/devices/"+(String)device_id+"/commands/power")
  {
    mess.command = "off";
  }

  if (topic == "/devices/"+(String)device_id+"/commands/mode")
  {
  
    if (payload == "off")
    {      
        mess.command = payload;
    } 

    else
    {
      StaticJsonDocument<128> desired_conf;
      DeserializationError error = deserializeJson(desired_conf, payload);

      if (error) 
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      
      mess.tempdes = desired_conf["tempdes"]; 
      mess.humdes = desired_conf["humdes"];
      String mode= desired_conf["mode"];
      mess.command=mode;
      mess.update=desired_conf["update"];
      //pxmess= &mess;
    }
  }  
  
  if(mess.command!="")
  {
    if(pdTRUE == xQueueSend(messageQueue_hand,( void * ) &mess,portMAX_DELAY))
      {
        //
      }
    else
    {
      Serial.println("c'è un problema nella send");
    }
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

void initwifi(){
  Serial.println("Starting wifi");

  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid.c_str(), password.c_str());
 
  Serial.println("Connecting to WiFi");

  vTaskDelay(100/portTICK_PERIOD_MS);

  if (WiFi.status() != WL_CONNECTED)
  {   
    if (WiFi.status() != WL_DISCONNECTED) 
    {
      WiFi.disconnect(false, true);
      vTaskDelay(200/portTICK_PERIOD_MS);
    }
    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED) 
    {
      vTaskDelay(100/portTICK_PERIOD_MS);   
    } 
  }
}

void setupWifi(){

  initwifi();
  Serial.println(time(nullptr));
  configTime(0, 3600, ntp_primary);
  Serial.println("Waiting on time sync...");
  startTime = xTaskGetTickCount(); 
  while (true)
  {  
    if(time(nullptr) > 1510644967)
    {
      Serial.println(time(nullptr));
      Serial.println("Ho ottenuto l'ora");
      break;
    }
    if(xTaskGetTickCount()-startTime>3000)
    {
      Serial.println("RIAVVIO IL WIFI");
      WiFi.disconnect(false,true);
      initwifi();
      startTime = xTaskGetTickCount();
    }
      
    vTaskDelay(100/portTICK_PERIOD_MS);
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

void setupMqtt(){
  device = new CloudIoTCoreDevice(project_id, location, registry_id, device_id, private_key_str);  
  netClient = new WiFiClientSecure();  
  mqttClient = new MQTTClient(512);
  mqttClient->setOptions(100, true, 1000); // keepAlive, cleanSession, timeout
  mqtt = new CloudIoTCoreMqtt(mqttClient, netClient, device);
  mqtt->setUseLts(true);
  mqtt->startMQTT();
  mqtt->mqttConnect(); 
}

void task_WarningLed(void * parameter)
{ 
    
  for(;;)
  {
    xSemaphoreTake(warning_led,portMAX_DELAY);
  
    
    Serial.println(passingLed);
    while(passingLed == 0)
    {      
      Serial.println("accendo led");
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      vTaskDelay(100/portTICK_PERIOD_MS);
      
    }
    while(passingLed == 1)
    {          
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      vTaskDelay(1000/portTICK_PERIOD_MS);      
    }

    digitalWrite(LED_BUILTIN, LOW); 
  }
}



void task_KeepWifi(void * parameter)
{
  for(;;)
  {    
    if (WiFi.status() == WL_CONNECTED)
    {
      //Serial.println("wifi connesso");
      vTaskDelay(1000/portTICK_PERIOD_MS);
      continue;
    }

    Serial.print("checking wifi...");
    startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() -startTime < TIMEOUT_RECONNECT)
    {
      Serial.print(".");
      vTaskDelay(1000/portTICK_PERIOD_MS);
    }   
    if(WiFi.status() == WL_CONNECTED )
    {
      continue;
    }
    //led di stato -> non connesso
    passingLed = 0;
    xSemaphoreGive(warning_led);
    setupWifi();
    passingLed= 2;       
  }
}

void task_KeepMqtt(void * parameter)
{
  for(;;)
  {
    xSemaphoreTake(mutexmqtt, portMAX_DELAY);    
    mqttClient->loop();
    vTaskDelay(10/portTICK_PERIOD_MS);  
    
    if (!mqttClient->connected()) 
    {      
      if(WiFi.status() == WL_CONNECTED)            //retry mqtt connection only if wifi is connected
      {         
        passingLed = 1;
        xSemaphoreGive(warning_led);
        mqtt->mqttConnect();  
        passingLed = 2;             
      }      
    }    
    xSemaphoreGive(mutexmqtt);
    vTaskDelay(1000/portTICK_PERIOD_MS);    
  }
}

void task_MessageHandler(void * parameter)
{
  for(;;)
  {   
    if(pdTRUE == xQueueReceive(messageQueue_hand,(void * ) &com,portMAX_DELAY))
    {
      Serial.println("Task MessageHANDLER");
      xSemaphoreTake(mutex, portMAX_DELAY);
      Serial.print("Riprendo comando: ");
      Serial.println(com.command);

      if(com.command==active_mode && !com.update)
      {
        if(active_mode=="deumplus")     // To check if last active mode was deumplus
          xSemaphoreGive(stopdeumplus);
        active_mode="none";
        send_signal(TEMPMIN,"deumplus",false);   
        xSemaphoreGive(mutex);
      }
      else if(com.command=="pull")
      {
        xSemaphoreGive(pull);        
      }
      else if(com.command=="deum" || com.command=="cool")
      {
        if(active_mode=="deumplus")     // To check if last active mode was deumplus
          xSemaphoreGive(stopdeumplus);
        tempdes=com.tempdes;
        active_mode=com.command;
        send_signal(com.tempdes,active_mode,true);
        xSemaphoreGive(mutex);
      }
      else if(com.command=="deumplus")
      {
        humdes=com.humdes;
        active_mode=com.command;
        actual_state="off";
        if (hum > humdes) 
        {
          send_signal(TEMPMIN,"deumplus",true);
          actual_state="on";
        }
        if(xSemaphoreTake(stopdeumplus, 0)==pdFALSE) 
          xSemaphoreGive(deumplus);
        Serial.println("TaskMESSAGEHANDLER HO DATO LA GIVE A DEUMPLUS");
        xSemaphoreGive(mutex);
      }
      else if(com.command=="off")
      {
        if(active_mode=="deumplus")     // To check if last active mode was deumplus
          xSemaphoreGive(stopdeumplus);
        active_mode="none";
        send_signal(TEMPMIN,"deumplus",false);   
        xSemaphoreGive(mutex);
      }
      else if(com.command=="record")
      {    
        xSemaphoreGive(record);
      }
    }
    //vTaskDelay(200/portTICK_PERIOD_MS);    
  }
}

void task_SendValues(void * parameter)
{
  for(;;)
  {
    
    xSemaphoreTake(pull, portMAX_DELAY);
    xSemaphoreTake(mutexmqtt, portMAX_DELAY);
    Serial.println("Task SendValues MUTEX MQTT PRESO");
    xSemaphoreGive(update_sensor);
    xSemaphoreTake(sensor_ack, portMAX_DELAY);  
    Serial.println("Task SendValues GET TEMP PRESA");
    String payload = String("{\"temp\": ") + String(temp) + String(",\"hum\": ") + String(hum) + String(",\"tempdes\": ") + tempdes + String(",\"humdes\": ") + humdes + String(",\"mode\": \"") + active_mode + String("\",\"command_stored\": \"")+ String(command_stored) + String("\"}");
    publishTelemetry("/pull",payload);
    Serial.println(payload);
    xSemaphoreGive(mutexmqtt);
    xSemaphoreGive(mutex); 
  } 
}

void deumPlusMode(){                            //task to repeat when deum+ is active (if active_mode=="deum+" in main loop)
  if(hum < humdes && actual_state=="on")
  {
    send_signal(TEMPMIN,"deumplus",false);
    actual_state="off";
  }
  if (actual_state=="off" && hum > (humdes + 5))  //5% tollerance
  {
    send_signal(TEMPMIN,"deumplus",true);
    actual_state="on";
  }
}

void task_DeumPlus(void * parameter) //da rivedere
{
  //float molt;
  for(;;)
  {
    xSemaphoreTake(deumplus, portMAX_DELAY);
    Serial.println("Task Deumplus PRENDO IL PRIMO SEMAFORO DAL FOR PIU' ESTERNO");
    for(;;)
    {
      if(xSemaphoreTake(mutex, 0)==pdTRUE)
      {
        if(xSemaphoreTake(stopdeumplus, 0)==pdTRUE) 
        {
          Serial.println("Task Deumplus FINE E RILASCIO MUTEX"); 
          xSemaphoreGive(mutex);
          break; //Come back to semaphore deumplus if the active mode was changed 
        }
        xSemaphoreGive(update_sensor);
        xSemaphoreTake(sensor_ack, portMAX_DELAY); 
        Serial.println("Task Deumplus GET TEMP PRESA");
        deumPlusMode();
        xSemaphoreGive(mutex);
      }
      //molt=(abs(hum-humdes))/2;
      //vTaskDelay(10000/portTICK_PERIOD_MS);
      vTaskDelay(10000/portTICK_PERIOD_MS); //High delay, usefull in this case
    } 
  } 
}

    
  

          // while(xSemaphoreTake(stopdeumplus, 0)==pdTRUE)
          // {
          //   if(xSemaphoreTake(deumplus, 0)!=pdTRUE)
          //   {
          //     Serial.println("Task Deumplus NON C'ERANO ABBASTANZA GIVE IN DEUMPLUS");
          //   }
          // }

