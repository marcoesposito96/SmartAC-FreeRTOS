#include <Arduino.h>
#define RSTBUTTON 12
#define TEMPMIN 18 
#define LED_BUILTIN 2
#define QUEUESIZE 20
#define QUEUELEN 10

bool request_in = false;
bool request_rec = false;
bool auto_mode = false;
float temp,hum;
float tempdes = 20;
float humdes = 45;
String active_mode = "none";

String ssid = "";
String password = "";

//deumPlus
String output_status = "on";
String actual_state = "off"; //keeps track of AC power state in deum+ mode

bool command_stored= false;
 
