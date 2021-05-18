#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>
#include <Preferences.h>

Preferences preferences ;
const size_t MAX_SIZE = 48; //dimensione massima segnale ac
IRac ac(32);  // pin di uscita


void get_stored() 
{
  try
  {
    preferences.begin("storedcommand", false);
    preferences.getBytes("command", &ac.next, MAX_SIZE);
    preferences.end();
  } 
  catch(...)
  {

  }
}

void edit_signal(float set_temp, stdAc::opmode_t set_mode, bool set_power ) //used to send signal with mode and temp
{
    get_stored();                               //retrieve stored signal 
    ac.next.degrees = set_temp;
    ac.next.mode = set_mode;
    ac.next.power = set_power;
}

void send_signal(float set_temp, String set_mode, bool state)
{
  stdAc::opmode_t op_mode;
  
  if(set_mode=="cool")  
  {    
    op_mode = stdAc::opmode_t::kCool;
	}
  else
  {
    op_mode = stdAc::opmode_t::kDry;
  }

  edit_signal(set_temp,op_mode,state); //customizzo il segnale ac prima di inviarlo   
  ac.sendAc(); 
}


