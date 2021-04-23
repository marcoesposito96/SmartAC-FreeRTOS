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


IRac ac(14);  // Create a A/C object using GPIO to sending messages with.

#if DECODE_AC
const uint8_t kTimeout = 50;
#else   // DECODE_AC
// Suits most messages, while not swallowing many repeats.
const uint8_t kTimeout = 15;
#endif  // DECODE_AC

const uint16_t kMinUnknownSize = 12;

// Legacy (No longer supported!)
//
// Change to `true` if you miss/need the old "Raw Timing[]" display.
#define LEGACY_TIMING_INFO false
// ==================== end of TUNEABLE PARAMETERS ====================

// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  // Somewhere to store the results

// This section of code runs only once at start-up.
void setup() {
  #if defined(ESP8266)
    Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);
  #else  // ESP8266
    Serial.begin(kBaudRate, SERIAL_8N1);
  #endif  // ESP8266
    while (!Serial)  // Wait for the serial connection to be establised.
      delay(50);
    Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
  #if DECODE_HASH
    // Ignore messages with less than minimum on or off pulses.
    irrecv.setUnknownThreshold(kMinUnknownSize);
  #endif  // DECODE_HASH
    irrecv.enableIRIn();  // Start the receiver
  preferences.begin("my-app", false);
  preferences.clear();
  preferences.end();
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

    //Serial.println(readablestate);

  

    ac.next.protocol = readablestate.protocol;  // Set a protocol to use.
    ac.next.model = readablestate.model;  // Some A/Cs have different models. Try just the first.
    ac.next.mode = readablestate.mode;  // Run in cool mode initially.
    ac.next.celsius = readablestate.celsius;  // Use Celsius for temp units. False = Fahrenheit
    ac.next.degrees = readablestate.degrees;  // 25 degrees.
    ac.next.fanspeed = readablestate.fanspeed;  // Start the fan at medium.
    ac.next.swingv = readablestate.swingv;  // Don't swing the fan up or down.
    ac.next.swingh = readablestate.swingh;  // Don't swing the fan left or right.
    ac.next.light = true;  // Turn off any LED/Lights/Display that we can.
    ac.next.beep = readablestate.beep;  // Turn off any beep from the A/C if we can.
    ac.next.econo = readablestate.econo;  // Turn off any economy modes if we can.
    ac.next.filter = readablestate.filter;  // Turn off any Ion/Mold/Health filters if we can.
    ac.next.turbo = readablestate.turbo;  // Don't use any turbo/powerful/etc modes.
    ac.next.quiet = readablestate.quiet;  // Don't use any quiet/silent/etc modes.
    ac.next.sleep = readablestate.sleep;  // Don't set any sleep time or modes.
    ac.next.clean = readablestate.clean;  // Turn off any Cleaning options if we can.
    ac.next.clock = readablestate.clock;  // Don't set any current time if we can avoid it.
    ac.next.power = true;  // Initially start with the unit off.
    
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
  
    Serial.println();    // Blank line between entries
    yield();             // Feed the WDT (again)
  }
}