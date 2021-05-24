#include <Arduino.h>
#include "mqtt_func.h"
#include "IRstorecommand.h"
#include "wifi_cred_config.h"
#include "manage_temp.h"

SemaphoreHandle_t hotspot_mode;
SemaphoreHandle_t warning_led;

TaskHandle_t task_Hotspot_hand;
TaskHandle_t task_KeepWifi_hand;
TaskHandle_t task_KeepMqtt_hand;



void setup()
{
  hotspot_mode = xSemaphoreCreateBinary();
  warning_led = xSemaphoreCreateBinary();
  
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
    
  }
 
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
  // xTaskCreate(
  //                   task_IncomingMess,          /* Task function. */
  //                   "Task IncomingMess",        /* String with name of task. */
  //                   10000,            /* Stack size in bytes. */
  //                   NULL,             /* Parameter passed as input of the task */
  //                   1,                /* Priority of the task. */
  //                   &task_IncomingMess_hand
  //                   );   
    
    
 
}

void loop()
{
  vTaskDelay(portMAX_DELAY);

}