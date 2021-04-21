#include <Arduino.h>

#include <DHT.h>
#define DHTPIN 25     
#define DHTTYPE DHT22 

DHT dht(DHTPIN, DHTTYPE);
float temp_old = 0;
float hum_old = 0;

#define TEMP_DELAY 10000    // 10 seconds
uint64_t tempUpdateTime = 0;


void readTempature()x
{ //check if temp has changed and send it to backend server

  float temp_new = (round(dht.readTemperature() * 2)) / 2;
  float hum_new = (round(dht.readHumidity() * 2)) / 2;

  Serial.println(temp_new);
  Serial.println(hum_new;

  if (temp_new != temp_old)
  {
    Serial.println("sending temp");    
    //sendTemptoServer();                     //TODO
    temp_old = temp_new;
    hum_old = hum_new;   
  }
}

void setup()
{
  Serial.begin(115200);
  dht.begin();
}

void loop()
{   
    uint64_t now = millis();

    if ((now - tempUpdateTime) > TEMP_DELAY)
        {
          tempUpdateTime = now;
          readTempature();
        }
}