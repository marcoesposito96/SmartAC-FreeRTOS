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

int passingLed;
unsigned long startTime;

Client *netClient;
CloudIoTCoreDevice *device;
CloudIoTCoreMqtt *mqtt;
MQTTClient *mqttClient;
unsigned long iat = 0;
String jwt;
struct messageq
{
  float tempdes = TEMPMIN;
  float humdes = 50;
  String command;
  bool update = 0;
} mess, com;

QueueHandle_t messageQueue_hand = xQueueCreate(QUEUELEN, sizeof(messageq)); //queue used for passing messages from mqtt topics to decisional task

void messageReceived(String &topic, String &payload) //manage incoming commands in mqtt topics
{
  Serial.println("incoming: " + topic + " - " + payload);
  mess.command = "";

  if (topic == "/devices/" + (String)device_id + "/commands/pull")
  {
    mess.command = "pull";
  }

  else if (topic == "/devices/" + (String)device_id + "/commands/record")
  {
    mess.command = "record";
  }

  else if (topic == "/devices/" + (String)device_id + "/commands/power")
  {
    mess.command = "off";
  }

  else if (topic == "/devices/" + (String)device_id + "/commands/mode")
  {

    if (payload == "off")
    {
      mess.command = payload;
    }

    else
    {
      StaticJsonDocument<128> desired_conf; //desarialize json passed in payload
      DeserializationError error = deserializeJson(desired_conf, payload);

      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      mess.tempdes = desired_conf["tempdes"];
      mess.humdes = desired_conf["humdes"];
      String mode = desired_conf["mode"];
      mess.command = mode;
      mess.update = desired_conf["update"];
    }
  }

  if (mess.command != "")
  {
    if (pdTRUE == xQueueSend(messageQueue_hand, (void *)&mess, portMAX_DELAY)) //if there is a free slot, put message in queue
    {
      //
    }
    else
    {
      Serial.println("c'è un problema nella send");
    }
  }
}

String getDefaultSensor() //LA USIAMO QUESTA FUNZIONE???
{
  return "Wifi: " + String(WiFi.RSSI()) + "db";
}

String getJwt() //calculate token for Google cloud auth
{
  iat = time(nullptr);
  Serial.println("Refreshing JWT");
  jwt = device->createJWT(iat, jwt_exp_secs);
  return jwt;
}

void initwifi() //intialize wifi connection and reconnect
{
  Serial.println("Starting wifi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  Serial.println("Connecting to WiFi");

  vTaskDelay(100 / portTICK_PERIOD_MS);

  if (WiFi.status() != WL_CONNECTED)
  {
    if (WiFi.status() != WL_DISCONNECTED)
    {
      WiFi.disconnect(false, true);
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED)
    {
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  }
}

void setupWifi() //setup wifi and update time from ntp server
{

  initwifi();
  Serial.println(time(nullptr));
  configTime(0, 3600, ntp_primary);
  Serial.println("Waiting on time sync...");
  startTime = xTaskGetTickCount();
  while (true)
  { //if it fails getting valid time, reinit wifi
    if (time(nullptr) > 1510644967)
    {
      Serial.println(time(nullptr));
      Serial.println("Got time");
      break;
    }
    if (xTaskGetTickCount() - startTime > 3000)
    {
      Serial.println("reinitialize wifi connection");
      WiFi.disconnect(false, true);
      initwifi();
      startTime = xTaskGetTickCount();
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

///////////////////////////////
// Orchestrates various methods from preceeding code.
///////////////////////////////
bool publishTelemetry(String data)
{
  return mqtt->publishTelemetry(data);
}

bool publishTelemetry(const char *data, int length)
{
  return mqtt->publishTelemetry(data, length);
}

bool publishTelemetry(String subfolder, String data)
{
  return mqtt->publishTelemetry(subfolder, data);
}

bool publishTelemetry(String subfolder, const char *data, int length)
{
  return mqtt->publishTelemetry(subfolder, data, length);
}

void setupMqtt() //setup mqtt connection with Google Iot Core
{
  device = new CloudIoTCoreDevice(project_id, location, registry_id, device_id, private_key_str);
  netClient = new WiFiClientSecure();
  mqttClient = new MQTTClient(512);
  mqttClient->setOptions(100, true, 1000); //keepAlive, cleanSession, timeout
  mqtt = new CloudIoTCoreMqtt(mqttClient, netClient, device);
  mqtt->setUseLts(true);
  mqtt->startMQTT();
  mqtt->mqttConnect();
}

void task_WarningLed(void *parameter) //use led to notify if wifi isn't working (passingLed->0) or if mqtt isn't working (passingLed->1)
{

  for (;;)
  {
    xSemaphoreTake(warning_led, portMAX_DELAY);

    Serial.println(passingLed);
    while (passingLed == 0)
    {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    while (passingLed == 1)
    {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    digitalWrite(LED_BUILTIN, LOW);
  }
}

void task_KeepWifi(void *parameter) //check if wifi is still alive and eventually reconnects
{
  setupWifi();
  xSemaphoreGive(startmqtt);
  for (;;)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      continue;
    }

    Serial.print("checking wifi...");
    startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < TIMEOUT_RECONNECT)
    {
      Serial.print(".");
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      continue;
    }

    passingLed = 0; //if isn't working -> unlock warning led task and try reconnection
    xSemaphoreGive(warning_led);
    setupWifi();
    passingLed = 2; //neutral value
  }
}

void task_KeepMqtt(void *parameter) //check if mqtt is still alive and eventually reconnects
{
  xSemaphoreTake(startmqtt, portMAX_DELAY);
  setupMqtt();
  for (;;)
  {
    xSemaphoreTake(mutexmqtt, portMAX_DELAY);
    mqttClient->loop();
    vTaskDelay(10 / portTICK_PERIOD_MS);

    if (!mqttClient->connected())
    {
      if (WiFi.status() == WL_CONNECTED) //retry mqtt connection only if wifi is connected
      {
        passingLed = 1;
        xSemaphoreGive(warning_led);
        mqtt->mqttConnect();
        passingLed = 2;
      }
    }
    xSemaphoreGive(mutexmqtt);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void task_MessageHandler(void *parameter) //decisional task, reads messagge from queue and take action
{
  for (;;)
  {
    if (pdTRUE == xQueueReceive(messageQueue_hand, (void *)&com, portMAX_DELAY)) //take a message from the queue
    {
      xSemaphoreTake(mutexmessage, portMAX_DELAY);
      Serial.print("Picked from the queue: ");
      Serial.println(com.command);

      if (com.command == active_mode && !com.update) //if command sent is the current active mode -> power off
      {
        if (active_mode == "deumplus") //checks if last active mode was deumplus and eventually blocks deumplus task
          xSemaphoreGive(stopdeumplus);
        active_mode = "none";
        send_signal(TEMPMIN, "deumplus", false); //poweroff signal (first two args are irrelevants)
        xSemaphoreGive(mutexmessage);
      }
      else if (com.command == "pull") //if commands is a pull, unlocks task that send current status to mqtt server
      {
        xSemaphoreGive(pull);
      }
      else if (com.command == "deum" || com.command == "cool")
      {
        if (active_mode == "deumplus")
          xSemaphoreGive(stopdeumplus);
        tempdes = com.tempdes;
        active_mode = com.command;
        send_signal(com.tempdes, active_mode, true); //send ir signal with desired mode and temp
        xSemaphoreGive(mutexmessage);
      }
      else if (com.command == "deumplus")
      {
        humdes = com.humdes;
        active_mode = com.command;
        actual_state = "off";
        if (hum > humdes)
        {
          send_signal(TEMPMIN, "deumplus", true); //set AC in deum mode
          actual_state = "on";
        }
        else
        {
          send_signal(TEMPMIN, "deumplus", false); //power off AC
        }
        if (xSemaphoreTake(stopdeumplus, 0) == pdFALSE && !com.update)
          xSemaphoreGive(deumplus); //if isn't already unlocked, unlocks deumplus task
        xSemaphoreGive(mutexmessage);
      }
      else if (com.command == "off")
      {
        if (active_mode == "deumplus")
          xSemaphoreGive(stopdeumplus);
        active_mode = "none";
        send_signal(TEMPMIN, "deumplus", false);
        xSemaphoreGive(mutexmessage);
      }
      else if (com.command == "record")
      {
        xSemaphoreGive(record); //unlocks record mode task
      }
    }
  }
}

void task_SendValues(void *parameter) //get actual state and send it to mqtt server
{
  for (;;)
  {

    xSemaphoreTake(pull, portMAX_DELAY);
    xSemaphoreTake(mutexmqtt, portMAX_DELAY);
    xSemaphoreGive(update_sensor); //unlocks task that update hum and temp and wait for ack
    xSemaphoreTake(sensor_ack, portMAX_DELAY);
    String payload = String("{\"temp\": ") + String(temp) + String(",\"hum\": ") + String(hum) + String(",\"tempdes\": ") + tempdes + String(",\"humdes\": ") + humdes + String(",\"mode\": \"") + active_mode + String("\",\"command_stored\": \"") + String(command_stored) + String("\"}");
    publishTelemetry("/pull", payload);
    Serial.println(payload);
    xSemaphoreGive(mutexmqtt);
    xSemaphoreGive(mutexmessage);
  }
}

void deumPlusMode() //deumPlus mode routine, compare current hum with desired hum and power off/on AC
{
  if (hum < humdes && actual_state == "on")
  {
    Serial.println("actual state OFF");
    send_signal(TEMPMIN, "deumplus", false);
    actual_state = "off";
  }
  if (actual_state == "off" && hum > (humdes + 5)) //5% tollerance, avoids mutiple sends near threshold
  {
    Serial.println("actual state ON");
    send_signal(TEMPMIN, "deumplus", true);
    actual_state = "on";
  }
  Serial.println(actual_state);
}

void task_DeumPlus(void *parameter) //enables deumplus mode routine
{
  for (;;)
  {
    xSemaphoreTake(deumplus, portMAX_DELAY);
    for (;;)
    {
      if (xSemaphoreTake(mutexmessage, 0) == pdTRUE)
      {
        if (xSemaphoreTake(stopdeumplus, 0) == pdTRUE)
        {
          xSemaphoreGive(mutexmessage);
          break; //come back to semaphore deumplus if the active mode was changed
        }
        xSemaphoreGive(update_sensor); //update current temp e hum before calling deumplus routine
        xSemaphoreTake(sensor_ack, portMAX_DELAY);
        deumPlusMode();
        xSemaphoreGive(mutexmessage);
      }
      vTaskDelay(10000 / portTICK_PERIOD_MS); //high dealy, we don't need to check for hum variations too often
    }
  }
}
