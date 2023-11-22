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

// Whether there is currently a faerie spawned.
bool faerieFlying = false;
// Last time a faerie flew. Start at current time to avoid immediate spawn.
uint32_t lastFaerieFly = millis();
// Timeout in ms until another faerie should be spawned.
const uint32_t faerieFlyTimeout = 30000;
// Bottle faerie is currently in.
int8_t faerieBottle = -1;

bool shouldShowFaerie() {
  // If a faerie is already flying, keep displaying animation.
  if (faerieFlying) return true;
  // Randomly spawn a faerie.
  if (random(0, 10000) == 0) return true;
  // Timeout for spawning a faerie has been reached.
  if (millis() - lastFaerieFly > faerieFlyTimeout) return true;
}

void loop(void) {
  // Run main MQTT loop every loop.
  // interwebs.mqttLoop();

  if (PIXELS_ON) {
    switch (bottleAnimation) {
      case BOTTLE_ANIMATION_DEFAULT:
        // bottles[0]->testBlink();
        break;
      case BOTTLE_ANIMATION_FAERIES:
        // @todo sync multiple bottles, cycle through colors
        // bottles[0]->glow(...);
        // Render a fairy on top of the glow animations.
        if (shouldShowFaerie()) {
          // If a new faerie, pick a random bottle.
          if (faerieBottle = -1) {
            faerieBottle = random(0, 7);
          }
          faerieFlying = bottles[faerieBottle]->showFaerie();
          // After animation, reset bottle and log time.
          if (!faerieFlying) {
            faerieBottle = -1;
            lastFaerieFly = millis();
          }
        }
        break;
      case BOTTLE_ANIMATION_GLOW:
        // @todo sync multiple bottles, cycle through colors
        // bottles[0]->glow(...);
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
