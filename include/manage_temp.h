#include <Arduino.h>
#include "DHTesp.h"
#include "variables.h"
#define DHTPIN 25
#define DHTTYPE DHT22

DHTesp dht;
float tempold, humold; //to append temp and hum (used if the relevation fail for more than one second)

void get_temp()
{
  vTaskDelay(10 / portTICK_PERIOD_MS);
  tempold = temp;
  humold = hum;
  temp = (round(dht.getTemperature() * 2)) / 2; //detect temp and hum from dht
  hum = (round(dht.getHumidity() * 2)) / 2; 

  startTime = xTaskGetTickCount();

  while (isnan(hum) || isnan(temp)) //avoid reading errors, if reading fails for more than 1s, restore old reading
  {
    temp = (round(dht.getTemperature() * 2)) / 2;
    hum = (round(dht.getHumidity() * 2)) / 2;

    if (xTaskGetTickCount() - startTime > 1000) //check if a second has passed since the start of while
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

void task_GetSensor(void *parameter)
{
  for (;;)
  {
    xSemaphoreTake(update_sensor, portMAX_DELAY);
    Serial.println("Task get sensor");
    get_temp();
    xSemaphoreGive(sensor_ack);
  }
}

// while(xSemaphoreTake(update_sensor,0)==pdTRUE)*********************************DA RIVEDERE
// {
//   xSemaphoreGive(sensor_ack);
// }