#include <Arduino.h>
#define RSTBUTTON 33  

boolean request_in = false;
boolean request_rec = false;
boolean auto_mode = false;
float temp,hum;
float tempdes = 20;
float humdes = 45; 
String state;  //for automode
String active_mode = "none";

//deumPlus
String output_status = "on";
String actual_state = "off"; //keeps track of AC power state in deum+ mode

 
