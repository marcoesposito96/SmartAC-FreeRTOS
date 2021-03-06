#include <Arduino.h>
#include "DHTesp.h"
#include "variables.h"
#define DHTPIN 25
#define DHTTYPE DHT22

DHTesp dht;
float tempold, humold; //to append temp and hum (used if the relevation fail for more than one second)

void get_temp()
{
  tempold = temp;
  humold = hum;
  temp = (round(dht.getTemperature() * 2)) / 2; //get temp e hum from sensor with 2 decimal places
  hum = (round(dht.getHumidity() * 2)) / 2;

  TickType_t startTime = xTaskGetTickCount();

  while (isnan(hum) || isnan(temp)) //avoid reading errors, if reading fails for more than 1s, restore old reading
  {
    temp = (round(dht.getTemperature() * 2)) / 2;
    hum = (round(dht.getHumidity() * 2)) / 2;

    if ((xTaskGetTickCount() - startTime) > 30) //check if a second has passed since the start of while
    {
      temp = tempold;
      hum = humold;
      Serial.println("Restore old temp");
      break;
    }
  }
  Serial.println("temp: " + (String)temp);
  Serial.println("hum: " + (String)hum);
}

void task_GetSensor(void *parameter) //waits for unlock on update_sensor, update current temp and hum and ack caller
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 250;
  xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    xSemaphoreTake(update_sensor, portMAX_DELAY);
    TickType_t initTime = xTaskGetTickCount();
    Serial.println("Task get sensor");
    get_temp();
    xSemaphoreGive(sensor_ack);
    while (xSemaphoreTake(update_sensor, 0) == pdTRUE) //ack other tasks waiting for updated temp and hum
    {
      xSemaphoreGive(sensor_ack);
    }
    Serial.print("Tempo gets:");
    Serial.println(xTaskGetTickCount()-initTime);
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}
