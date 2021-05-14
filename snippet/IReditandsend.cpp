#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>
#include <Preferences.h>

const size_t MAX_SIZE = 48; //dimensione massima segnale ac
Preferences preferences;
const uint32_t BaudRate = 115200;
const uint16_t CaptureBufferSize = 1024;
IRac ac(14);  // pin di uscita
decode_results results;  // Variabile in cui allocheremo la lettura del comando base da customizzare

void edit()//TEMP E MODE VARIABILI GLOBALI
{
    ac.next.degrees=TEMP;
    ac.next.mode=MODE;
}

void setup() {
  Serial.begin(BaudRate, SERIAL_8N1);
  while (!Serial)  // Aspettiamo che la connessione seriale sia stabilita
    delay(50);
}

void loop() {
  if (true) { //METTERE CONDIZIONE DI CICLO (PROB. QUANDO ARRIVA IL SEGNALE DAL BACKEND MANDA)
    preferences.begin("my-app", false);
    preferences.getBytes("command", &ac.next, MAX_SIZE);
    preferences.end();
    edit(); //customizzo il segnale ac prima di inviarlo
    //MANCA GESTIONE UMIDITA
    ac.sendAc(); 
  }
}

/* PER EVENTUALE DEBUG
Serial.print("Risposta di getbytes: "); // Debug
Serial.println(a); // Debug
Serial.print("i gradi sono: "); 
Serial.println(command.degrees); 
Serial.print(resultToHumanReadableBasic(&results)); 
Serial.print("Dimensione ac: ");
Serial.println(sizeof(ac.next)); 
*/