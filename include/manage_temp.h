#include <Arduino.h>
#include <DHT.h>

#define DHTPIN 25     
#define DHTTYPE DHT22 

DHT dht(DHTPIN, DHTTYPE);


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