#include <Arduino.h>
#include "mqtt_func.h"
#include "IRstorecommand.h"
#include "wifi_cred_config.h"
#include "manage_temp.h"

SemaphoreHandle_t hotspot_mode;
SemaphoreHandle_t warning_led;
SemaphoreHandle_t update_sensor;
SemaphoreHandle_t sensor_ack;
SemaphoreHandle_t pull, record, stopdeumplus, deumplus, mutex, mutexmqtt;

TaskHandle_t task_Hotspot_hand;
TaskHandle_t task_KeepWifi_hand;
TaskHandle_t task_KeepMqtt_hand;
TaskHandle_t task_SendValues_hand;
TaskHandle_t task_GetSensor_hand;
TaskHandle_t task_Record_hand;
TaskHandle_t task_DeumPlus_hand;

void setup()
{
  hotspot_mode = xSemaphoreCreateBinary();
  warning_led = xSemaphoreCreateBinary();
  update_sensor = xSemaphoreCreateBinary();
  sensor_ack = xSemaphoreCreateBinary();
  pull = xSemaphoreCreateBinary();
  record = xSemaphoreCreateBinary();
  deumplus = xSemaphoreCreateBinary();
  stopdeumplus= xSemaphoreCreateBinary();
  mutex= xSemaphoreCreateMutex();
  mutexmqtt= xSemaphoreCreateMutex();
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
    setupWifi();
    setupMqtt();              //initialize mqtt
    dht.begin();                  //initialize hum & temp sensor
       
    preferences.begin("storedcommand", false);                              //check if remote is already stored
    command_stored = preferences.getBool("command_stored", false);
    preferences.end();
 
  xTaskCreate(
                    task_KeepWifi,          /* Task function. */
                    "Task KeepWifi",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &task_KeepWifi_hand
                    );
  
  xTaskCreate(
                    task_KeepMqtt,          /* Task function. */
                    "Task KeepMqtt",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &task_KeepMqtt_hand
                    ); 

  xTaskCreate(
                    task_WarningLed,          /* Task function. */
                    "Task WarningLed",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &task_WarningLed_hand
                    ); 
  
  xTaskCreate(
                    task_MessageHandler,          /* Task function. */
                    "Task MessageHandler",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &task_MessageHandler_hand
                    ); 
  xTaskCreate(
                    task_SendValues,          /* Task function. */
                    "Task SendValues",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    3,                /* Priority of the task. */
                    &task_SendValues_hand
                    ); 
  xTaskCreate(
                    task_GetSensor,          /* Task function. */
                    "Task GetSensor",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    3,                /* Priority of the task. */
                    &task_GetSensor_hand
                    );  
  xTaskCreate(
                    task_Record,          /* Task function. */
                    "Task Record",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &task_Record_hand
                    );   
  xTaskCreate(
                    task_DeumPlus,          /* Task function. */
                    "Task DeumPlus",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &task_DeumPlus_hand
                    );   
    
 }
}

void loop()
{
  vTaskDelay(portMAX_DELAY);

}