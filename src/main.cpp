#include <Arduino.h>
#include "mqtt_func.h"
#include "IRstorecommand.h"
#include "wifi_cred_config.h"
#include "manage_temp.h"

bool auto_mode = false;
float temp,hum;
float tempdes = 20;
float humdes = 45;
float tempold, humold;
String active_mode = "none";
String ssid = "";
String password = "";
String actual_state = "off"; //keeps track of AC power state in deum+ mode
bool command_stored= false;

SemaphoreHandle_t hotspot_mode = NULL;
SemaphoreHandle_t warning_led = NULL;
SemaphoreHandle_t update_sensor = NULL;
SemaphoreHandle_t sensor_ack = NULL;
SemaphoreHandle_t pull = NULL; 
SemaphoreHandle_t record = NULL;
SemaphoreHandle_t stopdeumplus = NULL;
SemaphoreHandle_t deumplus = NULL;
SemaphoreHandle_t mutexmessage = NULL;
SemaphoreHandle_t mutexmqtt = NULL;
SemaphoreHandle_t startmqtt = NULL;


TaskHandle_t task_Hotspot_hand;
TaskHandle_t task_KeepWifi_hand;
TaskHandle_t task_KeepMqtt_hand;
TaskHandle_t task_SendValues_hand;
TaskHandle_t task_GetSensor_hand;
TaskHandle_t task_Record_hand;
TaskHandle_t task_DeumPlus_hand;
TaskHandle_t task_WarningLed_hand;
TaskHandle_t task_MessageHandler_hand;

void setup()
{
    
  hotspot_mode = xSemaphoreCreateBinary();  
  warning_led = xSemaphoreCreateBinary();
  update_sensor = xSemaphoreCreateBinary();
  sensor_ack = xSemaphoreCreateBinary();
  pull = xSemaphoreCreateBinary();
  record = xSemaphoreCreateBinary();
  deumplus = xSemaphoreCreateBinary();
  stopdeumplus = xSemaphoreCreateBinary();
  startmqtt = xSemaphoreCreateBinary();  
  mutexmessage = xSemaphoreCreateMutex();
  mutexmqtt = xSemaphoreCreateMutex();

 

  if(       // if semaphores init fails, restart device
    hotspot_mode == NULL || warning_led == NULL || update_sensor == NULL || sensor_ack == NULL ||  
    pull == NULL || record == NULL || deumplus == NULL || stopdeumplus == NULL || startmqtt == NULL || 
    mutexmessage == NULL || mutexmqtt == NULL )
  {
    Serial.println("Error in sem init");
    ESP.restart();
  }

  Serial.begin(115200);  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RSTBUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RSTBUTTON), handleInterrupt, FALLING);
  check_credentials();

  xTaskCreate(
                    task_Hotspot,          /* Task function. */
                    "Task hotspot",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &task_Hotspot_hand
                     );

  if (wifi_configured == true)
  {
    //setupWifi();
    //setupMqtt();              //initialize mqtt
  
    dht.setup(DHTPIN, DHTesp::DHT22);   
    preferences.begin("storedcommand", false);                              //check if remote is already stored
    command_stored = preferences.getBool("command_stored", false);
    preferences.end();
 
   xTaskCreatePinnedToCore(
                    task_KeepWifi,          /* Task function. */
                    "Task KeepWifi",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &task_KeepWifi_hand,
                    0
                    );
  
  xTaskCreatePinnedToCore(
                    task_KeepMqtt,          /* Task function. */
                    "Task KeepMqtt",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &task_KeepMqtt_hand,
                    0
                    ); 

  xTaskCreatePinnedToCore(
                    task_WarningLed,          /* Task function. */
                    "Task WarningLed",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &task_WarningLed_hand,
                    0
                    ); 
  
  xTaskCreatePinnedToCore(
                    task_MessageHandler,          /* Task function. */
                    "Task MessageHandler",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &task_MessageHandler_hand,
                    1
                    ); 
  xTaskCreatePinnedToCore(
                    task_SendValues,          /* Task function. */
                    "Task SendValues",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    3,                /* Priority of the task. */
                    &task_SendValues_hand,
                    0
                    ); 
  xTaskCreatePinnedToCore(
                    task_GetSensor,          /* Task function. */
                    "Task GetSensor",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    3,                /* Priority of the task. */
                    &task_GetSensor_hand,
                    0
                    );  
  xTaskCreatePinnedToCore(
                    task_Record,          /* Task function. */
                    "Task Record",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &task_Record_hand,
                    1
                    );   
  xTaskCreatePinnedToCore(
                    task_DeumPlus,          /* Task function. */
                    "Task DeumPlus",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &task_DeumPlus_hand,
                    1
                    );   
    
 }
}

void loop()
{
  vTaskDelay(portMAX_DELAY);
}