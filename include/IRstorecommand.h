#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>
#include "variables.h"

const uint16_t RecvPin = 13;
const uint16_t CaptureBufferSize = 1024;
const uint8_t Timeout = 50;
const uint16_t Threshold = 100;
IRrecv irrecv(RecvPin, CaptureBufferSize, Timeout, true); //initialize IR receiver
decode_results results;

void setup_receiver()
{
  Serial.println("Waiting for IR input on pin: " + String(RecvPin));
  irrecv.setUnknownThreshold(Threshold); //threshold to avoid noise signal
  irrecv.enableIRIn();                   //start receiver
}

String get_signal_n_store()
{
  TickType_t lastMillis = xTaskGetTickCount();
  while ((xTaskGetTickCount() - lastMillis) < 20000) //20 seconds timeout in receiver mode
  {
    if (irrecv.decode(&results) == true)
    {
      stdAc::state_t readablestate;
      IRAcUtils::decodeToState(&results, &readablestate);
      Serial.println(typeToString(results.decode_type));
      if (typeToString(results.decode_type) == "UNKNOWN")
      {
        return "unknown_protocol";
      }
      try
      {
        xSemaphoreTake(mutexmessage, portMAX_DELAY);
        preferences.begin("storedcommand", false); //store recorded signal in SPIFFS
        preferences.putBytes("command", &readablestate, sizeof(readablestate));
        preferences.putBool("command_stored", true);
        preferences.end();
        xSemaphoreGive(mutexmessage);
        command_stored = true;
      }
      catch (...)
      {
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

void task_Record(void *parameter) //wait for a IR signal and publish result to mqtt server
{
  for (;;)
  {
    xSemaphoreTake(record, portMAX_DELAY);
    Serial.println("Record command received");
    String feedback = store_command();
    Serial.println(feedback);
    xSemaphoreTake(mutexmqtt, portMAX_DELAY);
    publishTelemetry("/record", feedback);
    xSemaphoreGive(mutexmqtt);
  }
}
