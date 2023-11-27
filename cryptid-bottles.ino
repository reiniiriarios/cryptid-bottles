//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~ CRYPTID BOTTLES ~ LED Project ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "cryptid-bottles.h"
#include "src/color.h"
#include "src/control.h"
#include "src/pxl8.h"
#include "src/interwebs.h"
#include "src/bottle.h"

// GLOBALS -----------------------------------------------------------------------------------------

Pxl8 pxl8;
Interwebs interwebs;
Bottle *bottles[NUM_BOTTLES];
Control control(&pxl8, &interwebs, *bottles, NUM_BOTTLES);

// ERROR HANDLING ----------------------------------------------------------------------------------

void err(void) {
  Serial.println("FATAL ERROR");
  for (;;) digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
}

// SETUP -------------------------------------------------------------------------------------------

// @todo MOVE THIS
rgb_t prettyWhiteColors[17] = {
  rgb_t{ 255, 175,  10 }, // ~2k
  rgb_t{ 255, 195,  35 }, // ~2.25k
  rgb_t{ 255, 210,  50 }, // ~2.5k
  rgb_t{ 255, 225,  70 }, // ~2.75k
  rgb_t{ 255, 245, 150 }, // ~3.5k
  rgb_t{ 255, 255, 160 }, // ~3.75k
  rgb_t{ 255, 255, 175 }, // mid---
  rgb_t{ 255, 255, 206 }, // mid--
  rgb_t{ 255, 255, 225 }, // mid-
  rgb_t{ 255, 255, 255 }, // mid
  rgb_t{ 225, 255, 255 }, // mid+
  rgb_t{ 206, 255, 255 }, // ~5.5k
  rgb_t{ 175, 255, 255 }, // ~6.5k
  rgb_t{ 150, 250, 255 }, // ~7.5k
  rgb_t{ 135, 225, 255 }, // ~8.5k
  rgb_t{ 127, 220, 225 }, // ~9k
  rgb_t{ 125, 215, 225 }  // ~9.5k
};

void setup(void) {
  Serial.begin(9600);
  // Wait for serial port to open.
  while (!Serial) delay(10);
  Serial.println("Starting...");

  // Seed by reading unused anolog pin.
  randomSeed(analogRead(A0));

  // Start bottles with random hue ranges.
  uint16_t hs[NUM_BOTTLES] = {};
  uint16_t he[NUM_BOTTLES] = {};
  rgb_t* color[NUM_BOTTLES] = {};
  allBottles([&](int i){
    hs[i] = random(0, 360);
    he[i] = hs[i] + random(30, 40);
    color[i] = &prettyWhiteColors[random(0, 17)];
  });

  // Bottles !! Config pin, start, and length according to hardware !!
  Serial.println("Setting up LEDs...");
  //                        pin  1st  len
  bottles[0] = new Bottle(&pxl8,  0,   0,  25, hs[0], he[0], *color[0]);
  bottles[1] = new Bottle(&pxl8,  0,  25,  25, hs[1], he[1], *color[1]);
  bottles[2] = new Bottle(&pxl8,  1,   0,  20, hs[2], he[2], *color[2]);
  bottles[3] = new Bottle(&pxl8,  1,  20,  30, hs[3], he[3], *color[3]);

  // Start pixel driver.
  if (!pxl8.init()) {
    err();
  }
  pxl8.setBrightness(control.brightness);

  // WiFi, MQTT, etc.
  // control.initMQTT();
  // if (interwebs.connect()) {
  //   control.sendDiscoveryAll();
  //   control.mqttCurrentStatus();
  // }
}

// ANIMATION HELPERS -------------------------------------------------------------------------------

// Last time a bottle changed hues.
uint32_t lastGlowChange = millis();

bool shouldChangeGlow(void) {
  if (millis() - lastGlowChange < 2500) return false; // Don't change too often.
  if (random(0, 15000) == 0) return true;
  if (millis() - lastGlowChange > control.glowSpeed) return true;
  return false;
}

void updateBottleHues(void) {
  if (shouldChangeGlow()) {
    uint8_t id = randBottleId();
    uint16_t hueStart = random(0, 360);
    uint16_t hueEnd = hueStart + random(30, 40);
    Serial.println("Updating hue for bottle " + String(id) + " to " + String(hueStart) + "-" + String(hueEnd));
    bottles[id]->setHue(hueStart, hueEnd, random(1500, 2500));
    lastGlowChange = millis();
  }
}

void updateBottleWhiteBalance(void) {
  if (shouldChangeGlow()) {
    uint8_t id = randBottleId();
    rgb_t c = prettyWhiteColors[random(0, 17)];
    Serial.println("Updating white balance for bottle " + String(id));
    bottles[id]->setColor(c, random(1500, 2500));
    lastGlowChange = millis();
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
  if (millis() - lastFaerieFly > control.faerieSpeed) return true;
  return false;
}

void spawnFaeries(void) {
  if (shouldShowFaerie()) {
    // If a new faerie, pick a random bottle.
    if (faerieBottle == -1) {
      faerieBottle = randBottleId();
      Serial.println("Spawning new faerie in bottle " + String(faerieBottle));
      bottles[faerieBottle]->spawnFaerie(random(8, 14) * 0.1);
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

  // Main animation processing.
  if (control.pixelsOn) {
    switch (control.bottleAnimation) {
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
      case BOTTLE_ANIMATION_GLOW_W:
        updateBottleWhiteBalance();
        allBottles([](int i){
          bottles[i]->glowColor();
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
      case BOTTLE_ANIMATION_ILLUM:
        allBottles([](int i){
          bottles[i]->illuminate(control.white_color);
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
  //     control.sendDiscoveryAll();
  //     control.mqttCurrentStatus();
  //   }
  // }

  // Push all pixel changes to bottles.
  pxl8.show();

  // At max FPS, every 30 seconds.
  if (loopCounter % (MAX_FPS * 30) == 0) {
    // control.mqttCurrentStatus();
    loopCounter = 0;
  }
  loopCounter++;

  // Speed check.
  uint32_t m = millis();
  if (prevMillis != 0 && m > prevMillis) { // skips first, ignores millis() overflow
    uint32_t s = m - prevMillis;
    if (s > 10) {
      Serial.println("Slow frame at " + String(s) + " ms.");
    }
  }
  prevMillis = m;
}
