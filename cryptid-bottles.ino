//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~ CRYPTID BOTTLES ~ LED Project ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "cryptid-bottles.h"
#include "src/pxl8.h"
#include "src/interwebs.h"
#include "src/bottle.h"

// GLOBALS -----------------------------------------------------------------------------------------

struct settings {
  // Whether to display pixels.
  bool pixelsOn = true;
  // Overall brightness, 0-255
  uint8_t brightness = 127;
  // Current animation.
  bottle_animation_t bottleAnimation = BOTTLE_ANIMATION_DEFAULT;
  // Glow hue change timeout speed.
  glow_speed_t glowSpeed = GLOW_SPEED_MEDIUM;
  // Faerie spawn timeout speed.
  faerie_speed_t faerieSpeed = FAERIE_SPEED_MEDIUM;
} SETTINGS;

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
  if (!SETTINGS.pixelsOn) on = "OFF";
  interwebs.mqttSendMessage("cryptid/bottles/status", on);
  interwebs.mqttSendMessage("cryptid/bottles/animation/status", BOTTLE_ANIMATIONS_INV.at(SETTINGS.bottleAnimation));
  interwebs.mqttSendMessage("cryptid/bottles/glow-speed/status", GLOW_SPEED_INV.at(SETTINGS.glowSpeed));
  interwebs.mqttSendMessage("cryptid/bottles/faerie-speed/status", FAERIE_SPEED_INV.at(SETTINGS.faerieSpeed));
  interwebs.mqttSendMessage("cryptid/bottles/brightness/status", String(SETTINGS.brightness));
}

void setupLEDs(void) {
  Serial.println("Setting up LEDs...");
  // Create bottles.            id start end <hue >hue
  bottles[0] = new Bottle(&pxl8, 0,    0, 25,   0,  25);
  bottles[1] = new Bottle(&pxl8, 0,   25, 25,  40,  80);
  bottles[2] = new Bottle(&pxl8, 1,    0, 20,  90, 120);
  bottles[3] = new Bottle(&pxl8, 1,   20, 30, 130, 160);

  // Start pixel driver.
  if (!pxl8.init()) {
    err();
  }
  pxl8.setBrightness(SETTINGS.brightness);
  // pxl8.cycle();
}

void setupInterwebs(void) {
  Serial.println("Setting up Interwebs...");
  // Turn lights on or off.
  interwebs.onMqtt("cryptid/bottles/set", [](String &payload){
    if (payload == "on" || payload == "ON" || payload.toInt() == 1) {
      SETTINGS.pixelsOn = true;
      if (SETTINGS.brightness == 0) {
        SETTINGS.brightness = 127;
      }
      interwebs.mqttSendMessage("cryptid/bottles/status", "ON");
    }
    else if (payload == "off" || payload == "OFF" || payload.toInt() == 0) {
      SETTINGS.pixelsOn = false;
      allBottles([](int i){
        bottles[i]->blank();
      });
      interwebs.mqttSendMessage("cryptid/bottles/status", "OFF");
    }
  });

  // Set the bottles animation.
  interwebs.onMqtt("cryptid/bottles/animation/set", [](String &payload){
    SETTINGS.pixelsOn = true;
    if (BOTTLE_ANIMATIONS.find(payload) == BOTTLE_ANIMATIONS.end()) {
      payload = "warning"; // not found
    }
    SETTINGS.bottleAnimation = BOTTLE_ANIMATIONS.at(payload);
    interwebs.mqttSendMessage("cryptid/bottles/animation/status", payload);
  });

  // Set the glow animation speed.
  interwebs.onMqtt("cryptid/bottles/glow-speed/set", [](String &payload){
    if (SETTINGS.bottleAnimation != BOTTLE_ANIMATION_GLOW) {
      SETTINGS.bottleAnimation = BOTTLE_ANIMATION_FAERIES;
    }
    if (GLOW_SPEED.find(payload) == GLOW_SPEED.end()) {
      payload = "medium"; // default
    }
    SETTINGS.glowSpeed = GLOW_SPEED.at(payload);
    interwebs.mqttSendMessage("cryptid/bottles/glow-speed/status", payload);
    interwebs.mqttSendMessage("cryptid/bottles/animation/status", BOTTLE_ANIMATIONS_INV.at(SETTINGS.bottleAnimation));
  });

  // Set the glow animation speed.
  interwebs.onMqtt("cryptid/bottles/faerie-speed/set", [](String &payload){
    SETTINGS.bottleAnimation = BOTTLE_ANIMATION_FAERIES;
    if (FAERIE_SPEED.find(payload) == FAERIE_SPEED.end()) {
      payload = "medium"; // default
    }
    SETTINGS.faerieSpeed = FAERIE_SPEED.at(payload);
    interwebs.mqttSendMessage("cryptid/bottles/faerie-speed/status", payload);
    interwebs.mqttSendMessage("cryptid/bottles/animation/status", BOTTLE_ANIMATIONS_INV.at(BOTTLE_ANIMATION_FAERIES));
  });

  // Set the bottles brightness.
  interwebs.onMqtt("cryptid/bottles/brightness/set", [](String &payload){
    uint8_t b = payload.toInt() & 0xFF;
    String on;
    if (b == 0) {
      SETTINGS.pixelsOn = false;
      on = "OFF";
      allBottles([](int i){
        bottles[i]->blank();
      });
    } else {
      SETTINGS.pixelsOn = true;
      on = "ON";
    }
    pxl8.setBrightness(b);
    SETTINGS.brightness = b;
    interwebs.mqttSendMessage("cryptid/bottles/status", on);
    interwebs.mqttSendMessage("cryptid/bottles/brightness/status", String(SETTINGS.brightness));
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

bool shouldChangeHue(void) {
  if (millis() - lastHueChange < 2500) return false; // Don't change too often.
  if (random(0, 15000) == 0) return true;
  if (millis() - lastHueChange > SETTINGS.glowSpeed) return true;
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
// Bottle faerie is currently in.
int8_t faerieBottle = -1;

bool shouldShowFaerie(void) {
  // If a faerie is already flying, keep displaying animation.
  if (faerieFlying) return true;
  // Randomly spawn a faerie.
  if (random(0, 10000) == 0) return true;
  // Timeout for spawning a faerie has been reached.
  if (millis() - lastFaerieFly > SETTINGS.faerieSpeed) return true;

  return false;
}

void spawnFaeries(void) {
  if (shouldShowFaerie()) {
    // If a new faerie, pick a random bottle.
    if (faerieBottle == -1) {
      faerieBottle = randBottleId();
      Serial.println("Spawning new faerie in bottle " + String(faerieBottle));
      bottles[faerieBottle]->spawnFaerie();
    }
    faerieFlying = bottles[faerieBottle]->showFaerie();
    // After animation, reset bottle and log time.
    if (!faerieFlying) {
      Serial.println("Faerie has flown away from bottle " + String(faerieBottle));
      faerieBottle = -1;
      lastFaerieFly = millis();
      faerieFlying = false;
    }
  }
}

// LOOP --------------------------------------------------------------------------------------------

uint32_t prevMicros; // FPS throttle.
uint32_t prevMillis = 0; // Speed check.
uint16_t loopCounter = 0;  // Counts up every frame.

void loop(void) {
  // FPS Throttle.
  uint32_t t;
  while (((t = micros()) - prevMicros) < (1000000L / MAX_FPS));
  prevMicros = t;

  // Run main MQTT loop every loop.
  // interwebs.mqttLoop();

  if (SETTINGS.pixelsOn) {
    switch (SETTINGS.bottleAnimation) {
      case BOTTLE_ANIMATION_DEFAULT:
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

  // At max FPS, every 30 seconds.
  if (loopCounter % (MAX_FPS * 30) == 0) {
    // mqttCurrentStatus();
    loopCounter = 0;
  }
  loopCounter++;

  // Speed check.
  uint32_t m = millis();
  if (prevMillis != 0 && m > prevMillis) { // skips first, ignores millis() overflow
    uint32_t s = m - prevMillis;
    if (s > 20) {
      Serial.println("Slow frame at " + String(s) + " ms.");
    }
  }
  prevMillis = m;
}
