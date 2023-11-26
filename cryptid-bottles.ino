//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~ CRYPTID BOTTLES ~ LED Project ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "cryptid-bottles.h"
#include "src/control.h"
#include "src/pxl8.h"
#include "src/interwebs.h"
#include "src/bottle.h"

// GLOBALS -----------------------------------------------------------------------------------------

Pxl8 pxl8;
Interwebs interwebs;
Bottle bottles[NUM_BOTTLES];
Control control(&pxl8, &interwebs, bottles, NUM_BOTTLES);

// ERROR HANDLING ----------------------------------------------------------------------------------

void err(void) {
  Serial.println("FATAL ERROR");
  for (;;) digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
}

// SETUP -------------------------------------------------------------------------------------------

void setup(void) {
  Serial.begin(9600);
  // Wait for serial port to open.
  // while (!Serial) delay(10);
  Serial.println("Starting...");

  // Seed by reading unused anolog pin.
  randomSeed(analogRead(A0));

  Serial.println("Setting up LEDs...");
  // Create bottles.        id start end <hue >hue
  bottles[0] = Bottle(&pxl8, 0,   0,  25,   0,  25);
  bottles[1] = Bottle(&pxl8, 0,  25,  25,  40,  80);
  bottles[2] = Bottle(&pxl8, 1,   0,  20,  90, 120);
  bottles[3] = Bottle(&pxl8, 1,  20,  30, 130, 160);

  // Start pixel driver.
  if (!pxl8.init()) {
    err();
  }
  pxl8.setBrightness(control.brightness);
  // pxl8.cycle();

  // WiFi, MQTT, etc.
  // setupInterwebs();
  // if (interwebs.connect()) {
  //   control.sendDiscoveryAll();
  //   control.mqttCurrentStatus();
  // }
}

// ANIMATION HELPERS -------------------------------------------------------------------------------

// Last time a bottle changed hues.
uint32_t lastHueChange = millis();

bool shouldChangeHue(void) {
  if (millis() - lastHueChange < 2500) return false; // Don't change too often.
  if (random(0, 15000) == 0) return true;
  if (millis() - lastHueChange > control.glowSpeed) return true;
  return false;
}

void updateBottleHues(void) {
  if (shouldChangeHue()) {
    uint8_t id = randBottleId();
    uint16_t hueStart = random(0, 360);
    uint16_t hueEnd = hueStart + random(30, 40);
    Serial.println("Updating hue for bottle " + String(id) + " to " + String(hueStart) + "-" + String(hueEnd));
    bottles[id].setHue(hueStart, hueEnd, random(1500, 2500));
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
  if (millis() - lastFaerieFly > control.faerieSpeed) return true;

  return false;
}

void spawnFaeries(void) {
  if (shouldShowFaerie()) {
    // If a new faerie, pick a random bottle.
    if (faerieBottle == -1) {
      faerieBottle = randBottleId();
      Serial.println("Spawning new faerie in bottle " + String(faerieBottle));
      bottles[faerieBottle].spawnFaerie();
    }
    faerieFlying = bottles[faerieBottle].showFaerie();
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

  if (control.pixelsOn) {
    switch (control.bottleAnimation) {
      case BOTTLE_ANIMATION_DEFAULT:
      case BOTTLE_ANIMATION_FAERIES:
        updateBottleHues();
        allBottles([](int i){
          bottles[i].glow();
        });
        spawnFaeries();
        break;
      case BOTTLE_ANIMATION_GLOW:
        updateBottleHues();
        allBottles([](int i){
          bottles[i].glow();
        });
        break;
      case BOTTLE_ANIMATION_RAIN:
        allBottles([](int i){
          bottles[i].rain();
        });
        break;
      case BOTTLE_ANIMATION_RAINBOW:
        allBottles([](int i){
          bottles[i].rainbow();
        });
        break;
      case BOTTLE_ANIMATION_TEST:
        allBottles([](int i){
          bottles[i].testBlink();
        });
        break;
      case BOTTLE_ANIMATION_WARNING:
      default:
        allBottles([](int i){
          bottles[i].warning();
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
    if (s > 20) {
      Serial.println("Slow frame at " + String(s) + " ms.");
    }
  }
  prevMillis = m;
}
