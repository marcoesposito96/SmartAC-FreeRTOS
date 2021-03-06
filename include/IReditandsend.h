#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>
#include <Preferences.h>
#include "variables.h"

Preferences preferences;
const size_t MAX_SIZE = 48; //ac state_t size
IRac ac(32); //initialize IR pin

void get_stored() //retrieve stored IR signal
{
  try
  {
    preferences.begin("storedcommand", false);
    preferences.getBytes("command", &ac.next, MAX_SIZE);
    preferences.end();
  }
  catch (...)
  {
  }
}

void edit_signal(float set_temp, stdAc::opmode_t set_mode, bool set_power) //edit recorded signal with mode,temp and hum desired
{
  get_stored();
  ac.next.degrees = set_temp;
  ac.next.mode = set_mode;
  ac.next.power = set_power;
}

void send_signal(float set_temp, String set_mode, bool state)
{
  stdAc::opmode_t op_mode;

  if (set_mode == "cool") //converts a string with mode in a opmode_t
  {
    op_mode = stdAc::opmode_t::kCool;
  }
  else
  {
    op_mode = stdAc::opmode_t::kDry;
  }

  edit_signal(set_temp, op_mode, state);
  ac.sendAc(); //send IR signal
 
  xSemaphoreGive(pull); //send updates values to server (current state changes after a signal is sent)
}
