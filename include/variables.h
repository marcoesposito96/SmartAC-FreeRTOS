#include <Arduino.h>
#define RSTBUTTON 12
#define TEMPMIN 18 
#define LED_BUILTIN 2
#define QUEUESIZE 20
#define QUEUELEN 10

extern bool auto_mode;
extern float temp,hum;
extern float tempdes;
extern float humdes;
extern float tempold; 
extern float humold;
extern String active_mode;
extern String ssid;
extern String password;
extern String actual_state; //keeps track of AC power state in deum+ mode
extern bool command_stored;

extern SemaphoreHandle_t warning_led;
extern SemaphoreHandle_t update_sensor;
extern SemaphoreHandle_t sensor_ack;
extern SemaphoreHandle_t pull;
extern SemaphoreHandle_t record;
extern SemaphoreHandle_t deumplus;
extern SemaphoreHandle_t stopdeumplus;
extern SemaphoreHandle_t mutexmessage;
extern SemaphoreHandle_t waitmessage;
extern SemaphoreHandle_t startmqtt;
extern SemaphoreHandle_t mutexmqtt;
extern SemaphoreHandle_t hotspot_mode;
extern TaskHandle_t task_Hotspot_hand;
extern TaskHandle_t task_KeepWifi_hand;
extern TaskHandle_t task_KeepMqtt_hand;
extern TaskHandle_t task_SendValues_hand;
extern TaskHandle_t task_GetSensor_hand;
extern TaskHandle_t task_Record_hand;
extern TaskHandle_t task_DeumPlus_hand;
extern TaskHandle_t task_WarningLed_hand;
extern TaskHandle_t task_MessageHandler_hand;

