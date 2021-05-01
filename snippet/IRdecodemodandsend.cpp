#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>
#include <Preferences.h>

const size_t MAX_SIZE = 48;

Preferences preferences ;

const uint16_t kRecvPin = 13;
const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;


IRac ac(14);  // pin di uscita

const uint8_t kTimeout = 50;
const uint16_t kMinUnknownSize = 12;

// Use turn on the save buffer feature for more complete capture coverage. ****************
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  // Variabile che conterrà le letture dei comandi



void setup() {
 
  Serial.begin(kBaudRate, SERIAL_8N1);
  while (!Serial)  // Aspettiamo che la connessione seriale sia stabilita
    delay(50);
  Serial.printf("\n In ascolto sul pin: \n", kRecvPin);
  irrecv.setUnknownThreshold(kMinUnknownSize); //imposta una soglia per evitare segnali di disturbo
  irrecv.enableIRIn();  // Start the receiver
}


void loop() {

  if (irrecv.decode(&results)) {
  
    stdAc::state_t comando ;
    preferences.begin("my-app", false);
    preferences.clear();
    Serial.print("byte presi ");
    size_t a;
    a=preferences.getBytes("comando", &comando, MAX_SIZE);
    Serial.print("Risposta di getbytes: ");
    Serial.println(a);
    preferences.end();

    Serial.print("i gradi sono: ");
    Serial.println(comando.degrees);

    Serial.print(resultToHumanReadableBasic(&results));
    stdAc::state_t readablestate;

    IRAcUtils::decodeToState(&results, &readablestate);  

    
    ac.next.protocol = readablestate.protocol; 
    ac.next.model = readablestate.model;
    ac.next.mode = readablestate.mode;  
    ac.next.celsius = readablestate.celsius;  
    ac.next.degrees = readablestate.degrees;  
    ac.next.fanspeed = readablestate.fanspeed;
    ac.next.swingv = readablestate.swingv;
    ac.next.swingh = readablestate.swingh; 
    ac.next.light = true; 
    ac.next.beep = readablestate.beep; 
    ac.next.econo = readablestate.econo; 
    ac.next.filter = readablestate.filter;
    ac.next.turbo = readablestate.turbo;  
    ac.next.quiet = readablestate.quiet; 
    ac.next.sleep = readablestate.sleep;  
    ac.next.clean = readablestate.clean;  
    ac.next.clock = readablestate.clock;  
    ac.next.power = true;  
    

    Serial.print("Dimensione ac: ");
    Serial.println(sizeof(ac.next));

 

    preferences.begin("my-app", false); //apro il namespace my-app
    preferences.putBytes("comando", &ac.next, sizeof(ac.next));        
    preferences.end();

    /* sleep(10);

    while(true){
    
    sleep(3);
    ac.sendAc(); 
    } */
    
    Serial.print(resultToHumanReadableBasic(&results));
    //Serial.print("IL PROTOCOLO: " + typeToString(results.decode_type, results.repeat));  
    /*
    Serial.println(results.command);
    Serial.println("QUESTO E' RESULTS: ");
    Serial.println(results.state[1]);*/
    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    //if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
    yield();  // Feed the WDT as the text output can take a while to print.
  }
}