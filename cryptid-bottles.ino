//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~ CRYPTID BOTTLES ~ LED Project ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "cryptid-bottles.h"
#include "src/pxl8.h"
#include "src/interwebs.h"

// GLOBALS -----------------------------------------------------------------------------------------

bool PIXELS_ON = true;

Pxl8 pxl8;
Interwebs interwebs(&PIXELS_ON);

// ERROR HANDLING ----------------------------------------------------------------------------------

void err(void) {
  Serial.println("FATAL ERROR");
  for (;;) digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
}

// SETUP -------------------------------------------------------------------------------------------

void setup(void) {
  // if (!pxl8.begin()) {
  //   err();
  // }
  // interwebs.connect();
}

// LOOP --------------------------------------------------------------------------------------------

void loop(void) {

  // Run main MQTT loop every loop.
  // interwebs.mqttLoop();

  // Check interwebs connections.
  // if (!interwebs.wifiIsConnected()) {
  //   // ...
  // }
  // else if (!interwebs.mqttIsConnected()) {
  //   // ...
  // }
}
