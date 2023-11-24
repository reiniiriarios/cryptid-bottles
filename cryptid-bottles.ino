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
uint8_t BRIGHTNESS = 33;

Pxl8 pxl8;
Interwebs interwebs;
Bottle *bottles[NUM_BOTTLES] = {
  new Bottle(&pxl8, 0, 50, 0, 30),
  new Bottle(&pxl8, 1, 50, 0, 30)
};

// ERROR HANDLING ----------------------------------------------------------------------------------

void err(void) {
  Serial.println("FATAL ERROR");
  for (;;) digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
}

// SETUP -------------------------------------------------------------------------------------------

void mqttCurrentStatus(void) {
  String on = "ON";
  if (!PIXELS_ON) on = "OFF";
  interwebs.mqttSendMessage("cryptid/bottles/status", on);
  interwebs.mqttSendMessage("cryptid/bottles/animation/status", BOTTLE_ANIMATIONS_INV.at(bottleAnimation));
  interwebs.mqttSendMessage("cryptid/bottles/brightness/status", String(BRIGHTNESS));
}

void setup(void) {
  if (!pxl8.begin()) {
    err();
  }
  pxl8.setBrightness(BRIGHTNESS);
  // pxl8.cycle();
  return;

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
      bottleAnimation = BOTTLE_ANIMATIONS.at(payload);
      interwebs.mqttSendMessage("cryptid/bottles/animation/status", payload);
    } else {
      bottleAnimation = BOTTLE_ANIMATION_WARNING;
      interwebs.mqttSendMessage("cryptid/bottles/animation/status", "warning");
    }
  });

  // Set the bottles brightness.
  interwebs.onMqtt("cryptid/bottles/brightness/set", [](String &payload){
    uint8_t b = normalizeSL(payload.toInt());
    BRIGHTNESS = b;
    pxl8.setBrightness(b);
    String on;
    if (b == 0) {
      PIXELS_ON = false;
      on = "OFF";
    } else {
      PIXELS_ON = true;
      on = "ON";
    }
    interwebs.mqttSendMessage("cryptid/bottles/status", on);
    interwebs.mqttSendMessage("cryptid/bottles/brightness/status", String(BRIGHTNESS));
  });

  // Send discovery when Home Assistant notifies it's online.
  interwebs.onMqtt("homeassistant/status", [](String &payload){
    if (payload == "online") {
      interwebs.mqttSendDiscovery();
      mqttCurrentStatus();
    }
  });

  if (interwebs.connect()) {
    mqttCurrentStatus();
  }
}

// ANIMATION HELPERS -------------------------------------------------------------------------------

// Last time a bottle changed hues.
uint32_t lastHueChange = millis();
// Timeout in ms until another bottle should change hue.
const uint32_t hueChangeTimeout = 50000;

bool shouldChangeHue(void) {
  if (random(0, 20000) == 0) return true;
  if (millis() - lastHueChange > hueChangeTimeout) return true;

  return false;
}

void updateBottleHues(void) {
  if (shouldChangeHue()) {
    uint16_t hueStart = random(0, 360);
    bottles[randBottleId()]->setHue(hueStart, hueStart + 40, random(750, 1500));
    lastHueChange = millis();
  }
}

// Whether there is currently a faerie spawned.
bool faerieFlying = false;
// Last time a faerie flew. Start at current time to avoid immediate spawn.
uint32_t lastFaerieFly = millis();
// Timeout in ms until another faerie should be spawned.
const uint32_t faerieFlyTimeout = 30000;
// Bottle faerie is currently in.
int8_t faerieBottle = -1;

bool shouldShowFaerie(void) {
  // If a faerie is already flying, keep displaying animation.
  if (faerieFlying) return true;
  // Randomly spawn a faerie.
  if (random(0, 10000) == 0) return true;
  // Timeout for spawning a faerie has been reached.
  if (millis() - lastFaerieFly > faerieFlyTimeout) return true;

  return false;
}

void spawnFaeries(void) {
  if (shouldShowFaerie()) {
    // If a new faerie, pick a random bottle.
    if (faerieBottle = -1) {
      faerieBottle = randBottleId();
    }
    faerieFlying = bottles[faerieBottle]->showFaerie();
    // After animation, reset bottle and log time.
    if (!faerieFlying) {
      faerieBottle = -1;
      lastFaerieFly = millis();
    }
  }
}

// LOOP --------------------------------------------------------------------------------------------

uint16_t loopCounter = 0;  // Counts up every frame based on MAX_FPS.

void loop(void) {
  // Run main MQTT loop every loop.
  // interwebs.mqttLoop();

  if (PIXELS_ON) {
    switch (bottleAnimation) {
      case BOTTLE_ANIMATION_DEFAULT:
        bottles[0]->warning();
        bottles[1]->warning();
        break;
      case BOTTLE_ANIMATION_FAERIES:
        updateBottleHues();
        allBottles([](int i){
          bottles[i]->glow();
        });
        spawnFaeries();
        break;
      case BOTTLE_ANIMATION_GLOW:
        updateBottleHues();
        allBottles([](int i){
          bottles[i]->glow();
        });
        break;
      case BOTTLE_ANIMATION_RAIN:
        allBottles([](int i){
          bottles[i]->rain();
        });
        break;
      case BOTTLE_ANIMATION_RAINBOW:
        allBottles([](int i){
          bottles[i]->rainbow();
        });
        break;
      case BOTTLE_ANIMATION_TEST:
        allBottles([](int i){
          bottles[i]->testBlink();
        });
        break;
      case BOTTLE_ANIMATION_WARNING:
      default:
        allBottles([](int i){
          bottles[i]->warning();
        });
    }
  }

  // Check and repair interwebs connections.
  // if (!interwebs.wifiIsConnected()) {
  //   bottles[0]->warningWiFi();
  //   interwebs.wifiReconnect();
  // }
  // else if (!interwebs.mqttIsConnected()) {
  //   bottles[0]->warningMQTT();
  //   if (interwebs.mqttReconnect()) {
  //     mqttCurrentStatus();
  //   }
  // }

  pxl8.show();

  // Assuming 60fps, every 30 seconds.
  if (loopCounter % 1800 == 0) {
    // mqttCurrentStatus();
    loopCounter = 0;
  }
  loopCounter++;
}
