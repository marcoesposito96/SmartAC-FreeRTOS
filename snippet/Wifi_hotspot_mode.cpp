#include <Arduino.h>

#include <WiFi.h>
#include <WebServer.h>
#include "index.h"
#include <Preferences.h>

const char *ssid_ap = "Smart_ac";
const char *password_ap = "123456789";
String ssid = "";
String password = "";
boolean wifi_configured = false;
Preferences preferences ;

WebServer server(80);

void loop()
{
  if (!wifi_configured )  //(!wifi_configured || !wifi_correct)
  { //mange access point mode
    server.handleClient();
  }
}

void handleIndex()    
{ // serve page to configure connection and Sinric credentials
  server.send_P(200, "text/html", INDEX_page, sizeof(INDEX_page));
}

void handleConf()
{
  Serial.println(server.arg("wifi"));
  Serial.println(server.arg("password"));
  if (server.arg("wifi") == "" || server.arg("password") == "")
  { // if module is sent without wifi ssid or password serve again page 
    handleIndex();
  }
  else
  {
    ssid = server.arg("wifi");
    password = server.arg("password");

    preferences.begin("credentials", false);                //store obtained credentials
    preferences.putString("ssid", ssid); 
    preferences.putString("password", password);    
    preferences.end();

    server.send(200, "text/html", "<h1>Configuration acquired, please connect wifi!</h1>");
    wifi_configured = true;
    setup();
  }
}

void check_credentials()
{
  
  preferences.begin("credentials", false); 
  ssid = preferences.getString("ssid", "wifi"); 
  password = preferences.getString("password", "passwordwifi");
  preferences.end();

  if (ssid == "" || password == "")
  {
    Serial.println("No values saved for ssid or password");
    wifi_configured == false;
  }
  else
  {
    Serial.println("Wifi: "+ ssid + " psw: " + password);
    wifi_configured == true;
  }
}

void reset_preferences_wifi()
{
  preferences.begin("credentials", false);                //reset stored credentials
  preferences.putString("ssid", ""); 
  preferences.putString("password", "");    
  preferences.end();
}



void setup()
{
  Serial.begin(115200);

if (wifi_configured == true)
  {

    Serial.println("Working mode");
    
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