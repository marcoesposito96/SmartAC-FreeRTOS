#include <Arduino.h>
#include "mqtt_func.h"
#include "IRstorecommand.h"
#include "wifi_cred_config.h"
#include "manage_temp.h"

float temp, hum; //temperature and humidity at last sensor relevation
float tempdes = 20; //temperature sended in the request by user
float humdes = 50; //humidity sended in the request by user
String active_mode = "none"; //the mode that are currently running
String ssid = ""; //wifi ssid
String password = ""; //wifi pw
String actual_state = "off"; //keeps track of AC power state in deum+ mode
bool command_stored = false; //to check if is stored a command in flash memory

SemaphoreHandle_t hotspot_mode = NULL; 
SemaphoreHandle_t warning_led = NULL;  
SemaphoreHandle_t update_sensor = NULL; 
SemaphoreHandle_t sensor_ack = NULL; //to confirm the successful detection from DHT sensor
SemaphoreHandle_t pull = NULL; 
SemaphoreHandle_t record = NULL; 
SemaphoreHandle_t stopdeumplus = NULL; 
SemaphoreHandle_t deumplus = NULL; 
SemaphoreHandle_t mutexmessage = NULL; //to put in mutual exclusion the tasks that handle messages
SemaphoreHandle_t mutexmqtt = NULL; //to put in mutual exclusion the tasks that using the mqtt's send channel
SemaphoreHandle_t startmqtt = NULL; //to setup the mqtt after wifi is running

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
  update_sensor = xSemaphoreCreateCounting(2,0);
  sensor_ack = xSemaphoreCreateCounting(2,0);
  pull = xSemaphoreCreateBinary();
  record = xSemaphoreCreateBinary();
  deumplus = xSemaphoreCreateBinary();
  stopdeumplus = xSemaphoreCreateBinary();
  startmqtt = xSemaphoreCreateBinary();
  mutexmessage = xSemaphoreCreateMutex();
  mutexmqtt = xSemaphoreCreateMutex();

  if ( // if semaphores init fails, restart device
      (hotspot_mode == NULL) || (warning_led == NULL) || (update_sensor == NULL) || (sensor_ack == NULL) ||
      (pull == NULL) || (record == NULL) || (deumplus == NULL) || (stopdeumplus == NULL) || (startmqtt == NULL) ||
      (mutexmessage == NULL) || (mutexmqtt == NULL))
  {
    Serial.println("Error in sem init");
    ESP.restart();
  }

  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RSTBUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RSTBUTTON), handleInterrupt, FALLING); //link pin to the interrupt function
  check_credentials(); //check wifi credentials

  xTaskCreate(
      task_Hotspot,
      "Task hotspot",
      10000,
      NULL,
      1,
      &task_Hotspot_hand);

  if (wifi_configured == true)
  {
    dht.setup(DHTPIN, DHTesp::DHT22); //init DHT sensor
    preferences.begin("storedcommand", false); 
    command_stored = preferences.getBool("command_stored", false); //check if remote is already stored else return false
    preferences.end();

    xTaskCreatePinnedToCore(
        task_KeepWifi,
        "Task KeepWifi",
        3500,
        NULL,
        1,
        &task_KeepWifi_hand,
        0);

    xTaskCreatePinnedToCore(
        task_KeepMqtt,
        "Task KeepMqtt",
        5000,
        NULL,
        1,
        &task_KeepMqtt_hand,
        0);

    xTaskCreatePinnedToCore(
        task_WarningLed,
        "Task WarningLed",
        1500,
        NULL,
        1,
        &task_WarningLed_hand,
        0);

    xTaskCreatePinnedToCore(
        task_MessageHandler,
        "Task MessageHandler",
        3000,
        NULL,
        3,
        &task_MessageHandler_hand,
        1);
    xTaskCreatePinnedToCore(
        task_SendValues,
        "Task SendValues",
        3000,
        NULL,
        4,
        &task_SendValues_hand,
        0);
    xTaskCreatePinnedToCore(
        task_GetSensor,
        "Task GetSensor",
        2000,
        NULL,
        4,
        &task_GetSensor_hand,
        1);
    xTaskCreatePinnedToCore(
        task_Record,
        "Task Record",
        3500,
        NULL,
        5,
        &task_Record_hand,
        1);
    xTaskCreatePinnedToCore(
        task_DeumPlus,
        "Task DeumPlus",
        4000,
        NULL,
        2,
        &task_DeumPlus_hand,
        1);
  }
  vTaskDelete(NULL);  //delete loop() and setup() task
}

void loop()
{  
}