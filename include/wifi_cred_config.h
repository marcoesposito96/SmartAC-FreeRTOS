#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "index.h"
#include "variables.h"

const char *ssid_ap = "Smart_ac";
const char *password_ap = "123456789"; //hotspot mode credentials
boolean wifi_configured = false;       //to know if credentials are stored in flash memory
WebServer server(80);
volatile uint32_t lastTime = 0; 

void handleIndex()
{ // serve page to configure connection and Sinric credentials
  server.send_P(200, "text/html", INDEX_page, sizeof(INDEX_page));
}

void handleConf()
{
  Serial.println(server.arg("wifi"));
  Serial.println(server.arg("password"));
  if ((server.arg("wifi") == "") || (server.arg("password") == ""))
  { // if module is sent without wifi ssid or password serve again page
    handleIndex();
  }
  else
  {
    ssid = server.arg("wifi");
    password = server.arg("password");

    preferences.begin("credentials", false);
    preferences.putString("ssid", ssid); //store obtained credentials
    preferences.putString("password", password);
    preferences.end();

    server.send(200, "text/html", "<h1>Configuration acquired</h1>");
    wifi_configured = true;
  }
}

void check_credentials()
{
  preferences.begin("credentials", false);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  preferences.end();

  if ((ssid == "") || (password == ""))
  {
    Serial.println("No values saved for ssid or password");
    wifi_configured = false;
    xSemaphoreGive(hotspot_mode); //start hotspot mode if there aren't stored credentials
  }
  else
  {
    Serial.println("Wifi: " + ssid + " psw: " + password);
    wifi_configured = true;
  }
}

void reset_preferences_wifi()
{
  preferences.begin("credentials", false); //reset stored credentials
  preferences.putString("ssid", "");
  preferences.putString("password", "");
  preferences.end();
  wifi_configured = false;
}

void IRAM_ATTR handleInterrupt()
{

  if ((xTaskGetTickCount() - lastTime) > 10)
  {
    vTaskDelete(task_KeepWifi_hand);
    vTaskDelete(task_KeepMqtt_hand);
    vTaskDelete(task_SendValues_hand);
    vTaskDelete(task_GetSensor_hand);
    vTaskDelete(task_Record_hand);
    vTaskDelete(task_DeumPlus_hand);
    vTaskDelete(task_MessageHandler_hand);
    xSemaphoreGiveFromISR(hotspot_mode, NULL); //start hotspot mode after reset button has been pressed
    Serial.println("handle");
  }
  lastTime = xTaskGetTickCount();
}

void task_Hotspot(void *parameter)
{
  for (;;)
  {
    xSemaphoreTake(hotspot_mode, portMAX_DELAY);
    reset_preferences_wifi(); //clean wifi credentials stored
    //disconnect wifi and puts the device into hotspot mode
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

    for (;;)
    {
      server.handleClient();

      if (wifi_configured==true)
      {
        Serial.println("prima break");
        ESP.restart();
      }
    }
  }
}
