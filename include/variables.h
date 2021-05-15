#include <Arduino.h>
#define RSTBUTTON 33
#define TEMPMIN 18 

bool request_in = false;
bool request_rec = false;
bool auto_mode = false;
float temp,hum;
float tempdes = 20;
float humdes = 45;
String active_mode = "none";

//deumPlus
String output_status = "on";
String actual_state = "off"; //keeps track of AC power state in deum+ mode

bool command_stored= false;
 
