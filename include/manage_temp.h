#include <Arduino.h>
#include "DHTesp.h"
#include "variables.h"
#define DHTPIN 25     
#define DHTTYPE DHT22 

//DHT dht(DHTPIN, DHTTYPE);

DHTesp dht;

void get_temp(){
  vTaskDelay(10/portTICK_PERIOD_MS);
  tempold = temp;
  humold = hum;
  temp = (round(dht.getTemperature() * 2)) / 2;
  hum = (round(dht.getHumidity() * 2)) / 2;  
  
  startTime = xTaskGetTickCount(); 
  
  while (isnan(hum) || isnan(temp))                           //avoid reading errors, if reading fails for more than 1s, restore old reading
    {
      temp = (round(dht.getTemperature() * 2)) / 2;
      hum = (round(dht.getHumidity() * 2)) / 2;  
      
      if(xTaskGetTickCount()-startTime>1000)
      {
        temp = tempold;
        hum = humold;
        Serial.println("Restore old temp");
        break;
      }
    }

  Serial.println("temp: "+(String)temp);
  Serial.println("hum: "+(String)hum);
}


void task_GetSensor(void * parameter)
{
  for(;;)
  {
    xSemaphoreTake(update_sensor,portMAX_DELAY);
    Serial.println("Task get sensor");
    get_temp();
    xSemaphoreGive(sensor_ack);
    // while(xSemaphoreTake(update_sensor,0)==pdTRUE)
    // {
    //   xSemaphoreGive(sensor_ack);
    // }
  } 
}
