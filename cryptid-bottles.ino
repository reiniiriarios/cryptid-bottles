//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~ CRYPTID BOTTLES ~ LED Project ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "cryptid-bottles.h"
#include "src/pxl8.h"
#include "src/interwebs.h"
#include "src/bottle.h"

// GLOBALS -----------------------------------------------------------------------------------------

bool PIXELS_ON = true;

Pxl8 pxl8;
Interwebs interwebs;
// Bottle bottles[8] = {
//   Bottle(&pxl8, 0, 6),
// };

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

  // Turn lights on or off.
  interwebs.onMqtt("cryptid/bottles/set", [](String &payload){
    if (payload == "on" || payload == "ON" || payload.toInt() == 1) {
      PIXELS_ON = true;
    }
    else if (payload == "off" || payload == "OFF" || payload.toInt() == 0) {
      PIXELS_ON = false;
    }
  });
  // Send discovery when Home Assistant notifies it's online.
  interwebs.onMqtt("homeassistant/status", [](String &payload){
    if (payload == "online") {
      interwebs.mqttSendDiscovery();
    }
  });

  interwebs.connect();
}

// LOOP --------------------------------------------------------------------------------------------

void loop(void) {

  // Run main MQTT loop every loop.
  // interwebs.mqttLoop();

  if (PIXELS_ON) {
    // bottles[0].rain();
    // pxl8.show();
  }

  // Check interwebs connections.
  // if (!interwebs.wifiIsConnected()) {
  //   // ...
  // }
  // else if (!interwebs.mqttIsConnected()) {
  //   // ...
  // }
}
