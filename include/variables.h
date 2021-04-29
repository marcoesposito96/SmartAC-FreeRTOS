#include <Arduino.h>

boolean request_in = false;
boolean auto_mode = false;
float temp,hum;
float thr = 20;
String state;
String output_status = "on";
