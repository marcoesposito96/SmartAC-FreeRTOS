#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>


unsigned long lastMillis = 0;
const uint16_t RecvPin = 13;
const uint16_t CaptureBufferSize = 1024;
const uint8_t Timeout = 50; 
const uint16_t Threshold = 100;
IRrecv irrecv(RecvPin, CaptureBufferSize, Timeout, true); //inizializzo il ricevitore
decode_results results;  // Variabile che conterrà le letture dei comandi
extern SemaphoreHandle_t record, mutex;


void setup_receiver() {  
  Serial.println("In ascolto sul pin: " + String(RecvPin));
  irrecv.setUnknownThreshold(Threshold); //imposta una soglia per evitare segnali di disturbo
  irrecv.enableIRIn();  // Start the receiver
}

String get_signal_n_store() 
{  
  lastMillis = millis();
  while ((millis() - lastMillis)<20000) 
  {    
    if (irrecv.decode(&results)){
      stdAc::state_t readablestate;
      IRAcUtils::decodeToState(&results, &readablestate); //trasformo il segnale in un vettore di tipo state_t
      Serial.println(typeToString(results.decode_type));
      if(typeToString(results.decode_type)=="UNKNOWN")
      {
        return "unknown_protocol";
      }      
      try 
      {
        preferences.begin("storedcommand", false); //apro il namespace storedcommand
        preferences.putBytes("command", &readablestate, sizeof(readablestate)); //salvo il comando ac.next nel namespace my-app sotto la voce "comando"       
        preferences.putBool("command_stored", true);
        command_stored = true;
        preferences.end();
        
      }
      catch(...){
        return "failed";
      }
      return "ok";
    }    
  }

  return "timeout";
}

String store_command()
{
  setup_receiver();
  return get_signal_n_store();
}

void task_Record(void * parameter) //da rivedere
{
  for(;;)
  {
    xSemaphoreTake(record, portMAX_DELAY);
    Serial.println("ricevuto comando record");
    String feedback = store_command();         
    Serial.println(feedback);
    publishTelemetry("/record", feedback);  
    xSemaphoreGive(mutex);   
  } 
}




