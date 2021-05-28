#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>
#include <Preferences.h>

Preferences preferences ;
const size_t MAX_SIZE = 48; //dimensione massima segnale ac
IRac ac(32);  // pin di uscita


extern TaskHandle_t task_KeepWifi_hand;
extern TaskHandle_t task_KeepMqtt_hand;
extern TaskHandle_t task_SendValues_hand;
extern TaskHandle_t task_GetSensor_hand;
extern TaskHandle_t task_Record_hand;
extern TaskHandle_t task_DeumPlus_hand;
extern TaskHandle_t task_MessageHandler_hand;
extern TaskHandle_t task_WarningLed_hand;

extern SemaphoreHandle_t pull;


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

  vTaskSuspend(task_KeepWifi_hand);
  vTaskSuspend(task_KeepMqtt_hand); 
  vTaskSuspend(task_WarningLed_hand);
  vTaskSuspend(task_SendValues_hand);
  vTaskSuspend(task_GetSensor_hand);
  vTaskSuspend(task_DeumPlus_hand);
  
  
  Serial.println("inizio send");
  edit_signal(set_temp,op_mode,state); //customizzo il segnale ac prima di inviarlo   
  
  vTaskDelay(300/portTICK_PERIOD_MS); 
  Serial.println(ac.sendAc());
  
  
  Serial.println("fine send"); 


  vTaskResume(task_KeepWifi_hand);
  vTaskResume(task_KeepMqtt_hand);
  vTaskResume(task_WarningLed_hand);
  vTaskResume(task_SendValues_hand);
  vTaskResume(task_GetSensor_hand);
  vTaskResume(task_DeumPlus_hand);
  xSemaphoreGive(pull);  // send updates values to server    
  
}


