#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>
#include <Preferences.h>

Preferences preferences ;
const uint16_t RecvPin = 13;
const uint32_t BaudRate = 115200;
const uint16_t CaptureBufferSize = 1024;
IRac ac;  // CONTROLLARE SE FUNZIONA ANCHE SENZA IL PIN DI USCITA
const uint8_t Timeout = 50; 
const uint16_t Threshold = 12;
IRrecv irrecv(RecvPin, CaptureBufferSize, Timeout, true); //inizializzo il ricevitore
decode_results results;  // Variabile che conterrà le letture dei comandi

void creasegnale(stdAc::state_t readablestate) {
    ac.next.protocol = readablestate.protocol; 
    ac.next.model = readablestate.model;
    ac.next.celsius = readablestate.celsius;  
    ac.next.fanspeed = readablestate.fanspeed;
    ac.next.swingv = readablestate.swingv;
    ac.next.swingh = readablestate.swingh; 
    ac.next.light = readablestate.light; 
    ac.next.beep = readablestate.beep; 
    ac.next.econo = readablestate.econo; 
    ac.next.filter = readablestate.filter;
    ac.next.turbo = readablestate.turbo;  
    ac.next.quiet = readablestate.quiet; 
    ac.next.sleep = readablestate.sleep;  
    ac.next.clean = readablestate.clean;  
    ac.next.clock = readablestate.clock;  
    ac.next.power = readablestate.power;  
    ac.next.degrees = readablestate.degrees;
    ac.next.mode = readablestate.mode; 
}

void setup() {
  Serial.begin(BaudRate, SERIAL_8N1);
  while (!Serial)  // Aspettiamo che la connessione seriale sia stabilita
    delay(50);
  Serial.printf("\n In ascolto sul pin: \n", RecvPin);
  irrecv.setUnknownThreshold(Threshold); //imposta una soglia per evitare segnali di disturbo
  irrecv.enableIRIn();  // Start the receiver
}

void loop() {
  if (irrecv.decode(&results)) {
    stdAc::state_t readablestate;
    IRAcUtils::decodeToState(&results, &readablestate); //trasformo il segnale in un vettore di tipo state_t
    creasegnale(readablestate); //genero il segnale ac da salvare
    preferences.begin("my-app", false); //apro il namespace my-app
    preferences.putBytes("command", &ac.next, sizeof(ac.next)); //salvo il comando ac.next nel namespace my-app sotto la voce "comando"       
    preferences.end();
  }
}




/* DEBUG
Serial.print(resultToHumanReadableBasic(&results));
Serial.print("Dimensione ac: ");
Serial.println(sizeof(ac.next));*/

/*PER EVENTUALE DEBUG
Serial.print("IL PROTOCOLO: " + typeToString(results.decode_type, results.repeat));  
Serial.println(results.command);
Serial.println("QUESTO E' RESULTS: ");
Serial.println(results.state[1]);
Display any extra A/C info if we have it.
String description = IRAcUtils::resultAcToString(&results);
if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
yield();  // Feed the WDT as the text output can take a while to print.*/