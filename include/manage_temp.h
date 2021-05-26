#include <Arduino.h>
#include <DHT.h>

#define DHTPIN 25     
#define DHTTYPE DHT22 

DHT dht(DHTPIN, DHTTYPE);
extern SemaphoreHandle_t update_sensor;
extern SemaphoreHandle_t sensor_ack;

void get_temp(){
  temp = (round(dht.readTemperature() * 2)) / 2;
  hum = (round(dht.readHumidity() * 2)) / 2;
  
  while (isnan(hum) || isnan(temp))                           //avoid reading errors
      {
        temp = (round(dht.readTemperature() * 2)) / 2;
        hum = (round(dht.readHumidity() * 2)) / 2;
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
