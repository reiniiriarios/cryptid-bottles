//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~ CRYPTID BOTTLES ~ LED Project ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "cryptid-bottles.h"

// GLOBALS -----------------------------------------------------------------------------------------

Pxl8 pxl8;
Interwebs interwebs;
std::vector<Bottle*> bottles = {};
Control control(&pxl8, &interwebs, &bottles);

// ERROR HANDLING ----------------------------------------------------------------------------------

void err(void) {
  Serial.println("FATAL ERROR");
  for (;;) digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
}

// ANIMATION HELPERS -------------------------------------------------------------------------------

// Last time a bottle changed hues.
uint32_t lastGlowChange = millis();

bool shouldChangeGlow(void) {
  if (millis() - lastGlowChange < 2500) return false; // Don't change too often.
  if (random(0, control.glowSpeed - 1000) == 0) return true;
  if (millis() - lastGlowChange > control.glowSpeed) return true;
  return false;
}

void updateBottleHues(void) {
  if (shouldChangeGlow()) {
    uint8_t id = random(0, bottles.size());
    uint16_t hueStart = random(0, 360);
    uint16_t hueEnd = hueStart + random(30, 40);
    Serial.println("Updating hue for bottle " + String(id) +
      " to " + String(hueStart) + "-" + String(hueEnd));
    bottles.at(id)->setHue(hueStart, hueEnd, random(1500, 2500));
    lastGlowChange = millis();
  }
}

rgb_t randomWhiteBalance(void) {
  auto it = WHITE_TEMPERATURES.begin();
  std::advance(it, rand() % WHITE_TEMPERATURES.size());
  return it->second;
}

void updateBottleWhiteBalance(void) {
  if (shouldChangeGlow()) {
    uint8_t id = random(0, bottles.size());
    rgb_t c = randomWhiteBalance();
    Serial.println("Updating white balance for bottle " + String(id) +
      " to " + String(c.r) + " " + String(c.g) + " " + String(c.b));
    bottles.at(id)->setColor(c, random(1500, 2500));
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
  if (random(0, control.faerieSpeed - 1000) == 0) return true;
  // Timeout for spawning a faerie has been reached.
  if (millis() - lastFaerieFly > control.faerieSpeed) return true;
  return false;
}

void spawnFaeries(void) {
  if (shouldShowFaerie()) {
    // If a new faerie, pick a random bottle.
    if (faerieBottle == -1) {
      faerieBottle = random(0, bottles.size());
      Serial.println("Spawning new faerie in bottle " + String(faerieBottle));
      bottles.at(faerieBottle)->spawnFaerie(random(8, 14) * 0.1);
    }
    faerieFlying = bottles.at(faerieBottle)->showFaerie();
    // After animation, reset bottle and log time.
    if (!faerieFlying) {
      Serial.println("Faerie has flown away from bottle " + String(faerieBottle));
      faerieBottle = -1;
      lastFaerieFly = millis();
      faerieFlying = false;
    }
  }
}

// SETUP -------------------------------------------------------------------------------------------

void setup(void) {
  Serial.begin(9600);
  // Wait for serial port to open.
  while (!Serial) delay(10);
  Serial.println("Starting...");

  // Seed by reading unused anolog pin.
  randomSeed(analogRead(A0));

  // Bottles !! Config pin, start, and length according to hardware !!
  Serial.println("Setting up LEDs...");
  //                                 pin  1st  len
  bottles.push_back(new Bottle(&pxl8,  0,   0,  25));
  bottles.push_back(new Bottle(&pxl8,  0,  25,  25));
  bottles.push_back(new Bottle(&pxl8,  1,   0,  20));
  bottles.push_back(new Bottle(&pxl8,  1,  20,  30));
  for (auto & bottle : bottles) {
    uint16_t hs = random(0, 360);
    bottle->setHue(hs, hs + random(30, 40));
    bottle->setColor(randomWhiteBalance());
  };

  // Start pixel driver.
  if (!pxl8.init()) {
    err();
  }
  pxl8.setBrightness(control.brightness);

  // WiFi, MQTT, etc.
  control.initMQTT();
  if (interwebs.connect()) {
    control.sendDiscoveryAll();
    control.mqttCurrentStatus();
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
  interwebs.mqttLoop();

  // Main animation processing.
  if (control.pixelsOn) {
    switch (control.bottleAnimation) {
      case BOTTLE_ANIMATION_DEFAULT:
      case BOTTLE_ANIMATION_FAERIES:
        updateBottleHues();
        for (auto & bottle : bottles) {
          bottle->glow();
        };
        spawnFaeries();
        break;
      case BOTTLE_ANIMATION_GLOW:
        updateBottleHues();
        for (auto & bottle : bottles) {
          bottle->glow();
        };
        break;
      case BOTTLE_ANIMATION_GLOW_W:
        updateBottleWhiteBalance();
        for (auto & bottle : bottles) {
          bottle->glowColor();
        };
        break;
      case BOTTLE_ANIMATION_RAIN:
        for (auto & bottle : bottles) {
          bottle->rain();
        };
        break;
      case BOTTLE_ANIMATION_RAINBOW:
        for (auto & bottle : bottles) {
          bottle->rainbow();
        };
        break;
      case BOTTLE_ANIMATION_ILLUM:
        for (auto & bottle : bottles) {
          bottle->illuminate(control.getWhiteBalanceRGB());
        };
        break;
      case BOTTLE_ANIMATION_TEST_WB:
        for (auto & bottle : bottles) {
          bottle->loopColors(&WHITE_TEMPERATURES_VECTOR);
        };
        break;
      case BOTTLE_ANIMATION_TEST:
        for (auto & bottle : bottles) {
          bottle->testBlink();
        };
        break;
      case BOTTLE_ANIMATION_WARNING:
      default:
        for (auto & bottle : bottles) {
          bottle->warning();
        };
    }
  }

  // Push all pixel changes to bottles.
  pxl8.show();


  if (loopCounter % (MAX_FPS * 15) == 0) {
    // Check and repair interwebs connections.
    if (!interwebs.wifiIsConnected()) {
      bottles.at(0)->warningWiFi();
      interwebs.wifiReconnect();
    }
    else if (!interwebs.mqttIsConnected()) {
      bottles.at(0)->warningMQTT();
      if (interwebs.mqttReconnect()) {
        control.sendDiscoveryAll();
        control.mqttCurrentStatus();
      }
    }
  }
  // At max FPS, every 30 seconds.
  if (loopCounter % (MAX_FPS * 60) == 0) {
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
