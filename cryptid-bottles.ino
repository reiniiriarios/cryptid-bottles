//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~ CRYPTID BOTTLES ~ LED Project ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "cryptid-bottles.h"
#include "src/pxl8.h"
#include "src/interwebs.h"
#include "src/bottle.h"

// GLOBALS -----------------------------------------------------------------------------------------

bool PIXELS_ON = true;
bottle_animation_t bottleAnimation = BOTTLE_ANIMATION_DEFAULT;

Pxl8 pxl8;
Interwebs interwebs;
Bottle *bottles[8] = {
  new Bottle(&pxl8, 0, 6),
};

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
      interwebs.mqttSendMessage("cryptid/bottles/status", "ON");
    }
    else if (payload == "off" || payload == "OFF" || payload.toInt() == 0) {
      PIXELS_ON = false;
      interwebs.mqttSendMessage("cryptid/bottles/status", "OFF");
    }
  });

  // Set the bottles animation.
  interwebs.onMqtt("cryptid/bottles/animation/set", [](String &payload){
    PIXELS_ON = true;
    if (BOTTLE_ANIMATIONS.find(payload) != BOTTLE_ANIMATIONS.end()) {
      bottleAnimation = BOTTLE_ANIMATIONS[payload];
      interwebs.mqttSendMessage("cryptid/bottles/animation/status", payload);
    } else {
      bottleAnimation = BOTTLE_ANIMATION_WARNING;
      interwebs.mqttSendMessage("cryptid/bottles/animation/status", "warning");
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
    switch (bottleAnimation) {
      case BOTTLE_ANIMATION_DEFAULT:
        // bottles[0]->testBlink();
        break;
      case BOTTLE_ANIMATION_RAIN:
        bottles[0]->rain();
        break;
      case BOTTLE_ANIMATION_TEST:
        bottles[0]->testBlink();
        break;
      case BOTTLE_ANIMATION_WARNING:
      default:
        bottles[0]->warning();
    }
  }
  // pxl8.show();

  // Check interwebs connections.
  // if (!interwebs.wifiIsConnected()) {
  //   // ...
  // }
  // else if (!interwebs.mqttIsConnected()) {
  //   // ...
  // }
}
