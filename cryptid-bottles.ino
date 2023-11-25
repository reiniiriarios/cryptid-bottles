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
uint8_t BRIGHTNESS = 127;

Pxl8 pxl8;
Interwebs interwebs;
Bottle *bottles[NUM_BOTTLES];

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

void setupLEDs(void) {
  // Create bottles.            id start end <hue >hue
  bottles[0] = new Bottle(&pxl8, 0,    0, 25,   0,  25);
  bottles[1] = new Bottle(&pxl8, 0,   25, 25,  40,  80);
  bottles[2] = new Bottle(&pxl8, 1,    0, 20,  90, 120);
  bottles[3] = new Bottle(&pxl8, 1,   20, 30, 130, 160);

  // Start pixel driver.
  if (!pxl8.init()) {
    err();
  }
  pxl8.setBrightness(BRIGHTNESS);
  // pxl8.cycle();
}

void setupInterwebs(void) {
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
    uint8_t b = payload.toInt() & 0xFF;
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

  // if (interwebs.connect()) {
  //   mqttCurrentStatus();
  // }
}

void setup(void) {
  Serial.begin(9600);
  // Wait for serial port to open.
  // while (!Serial) delay(10);
  Serial.println("Starting...");

  // Seed by reading unused anolog pin.
  randomSeed(analogRead(A0));

  // NeoPXL8, etc.
  setupLEDs();

  // WiFi, MQTT, etc.
  // setupInterwebs();
}

// ANIMATION HELPERS -------------------------------------------------------------------------------

// Last time a bottle changed hues.
uint32_t lastHueChange = millis();
// Timeout in ms until another bottle should change hue.
const uint32_t hueChangeTimeout = 50000;

bool shouldChangeHue(void) {
  if (millis() - lastHueChange < 2500) return false; // Don't change too often.
  if (random(0, 15000) == 0) return true;
  if (millis() - lastHueChange > hueChangeTimeout) return true;
  return false;
}

void updateBottleHues(void) {
  if (shouldChangeHue()) {
    uint8_t id = randBottleId();
    uint16_t hueStart = random(0, 360);
    uint16_t hueEnd = hueStart + random(30, 40);
    Serial.println("Updating hue for bottle " + String(id) + " to " + String(hueStart) + "-" + String(hueEnd));
    bottles[id]->setHue(hueStart, hueEnd, random(1500, 2500));
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
      Serial.println("Spawning new faerie in bottle " + String(faerieBottle));
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
// uint32_t millisCounter = millis();

void loop(void) {
  // Run main MQTT loop every loop.
  // interwebs.mqttLoop();

  if (PIXELS_ON) {
    switch (bottleAnimation) {
      case BOTTLE_ANIMATION_FAERIES:
        updateBottleHues();
        allBottles([](int i){
          bottles[i]->glow();
        });
        spawnFaeries();
        break;
      case BOTTLE_ANIMATION_DEFAULT:
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

  // Speed check.
  // Serial.print("\r" + String(millis() - millisCounter) + "   ");
  // millisCounter = millis();
}
