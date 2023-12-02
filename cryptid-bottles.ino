//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~ CRYPTID BOTTLES ~ LED Project ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "cryptid-bottles.h"

// GLOBALS -----------------------------------------------------------------------------------------

Pxl8 pxl8;
Interwebs interwebs(new WiFiClient, WIFI_SSID, WIFI_PASS, new IPAddress(MQTT_SERVER),
  MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS);
std::vector<Bottle*> bottles = {};
Control control(&pxl8, &interwebs, &bottles);
Adafruit_NeoPixel statusLED(1, 8, NEO_GRB + NEO_KHZ800);
VoltageMonitor voltageMonitor;

// ERROR HANDLING ----------------------------------------------------------------------------------

void err(uint32_t ledColor) {
  Serial.println(F("FATAL ERROR"));
  uint32_t c;
  for (;;) {
    c = 0;
    if ((millis() / 500) & 1) {
      c = ledColor;
    }
    statusLED.setPixelColor(0, c);
  }
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
      bottles.at(faerieBottle)->spawnFaerie(random(8, 14) * 0.1);
    }
    faerieFlying = bottles.at(faerieBottle)->showFaerie();
    // After animation, reset bottle and log time.
    if (!faerieFlying) {
      faerieBottle = -1;
      lastFaerieFly = millis();
      faerieFlying = false;
    }
  }
}

// SETUP -------------------------------------------------------------------------------------------

void ledStatus(status_t status) {
  uint8_t r, g, b;
  switch (status) {
    case STATUS_OK:
      statusLED.setPixelColor(0, 0);
      WiFi.setLEDs(0, 0, 0);
      break;
    case STATUS_WIFI_OFFLINE:
      statusLED.setPixelColor(0, 0xFF5000);
      WiFi.setLEDs(255, 80, 0);
      break;
    case STATUS_MQTT_OFFLINE:
      statusLED.setPixelColor(0, 0xFF0080);
      WiFi.setLEDs(255, 0, 127);
      break;
    case STATUS_MQTT_ACTIVE:
      statusLED.setPixelColor(0, 0x00C8FF);
      WiFi.setLEDs(0, 200, 255);
      break;
    case STATUS_UNKNOWN_ERROR:
    default:
      statusLED.setPixelColor(0, 0xFF0000);
      WiFi.setLEDs(255, 0, 0);
  }
  statusLED.show();
}

void setup(void) {
  Serial.begin(9600);
  // Wait for serial port to open.
  while (!Serial) delay(10);
  Serial.println(F("Starting..."));

  // Configure WiFi featherwing.
  WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);

  Serial.println(F("Reading voltage monitor..."));
  if (!voltageMonitor.begin()) {
    Serial.println(F("Failed to find INA219 chip"));
    err(0xFF6000);
  }
  // By default the INA219 will be calibrated with a range of 32V, 2A.
  // However uncomment one of the below to change the range.  A smaller
  // range can't measure as large of values but will measure with slightly
  // better precision.
  // voltageMonitor.setCalibration_32V_1A();
  // voltageMonitor.setCalibration_16V_400mA();

  // Seed by reading unused anolog pin.
  randomSeed(analogRead(A0));

  statusLED.begin();
  statusLED.setBrightness(64);

  // Bottles !! Config pin, start, and length according to hardware !!
  Serial.println(F("Setting up LEDs..."));
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

  // Start pixel driver. Call after bottle setup.
  if (!pxl8.init()) {
    Serial.println(F("Error starting NeoPXL8"));
    err(0xFF0000);
  }
  pxl8.setBrightness(control.brightness);

  // Check connection to WiFi board.
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println(F("Communication with WiFi module failed"));
    err(0xFF0080);
  }

  // Set up MQTT callbacks, etc.
  control.initMQTT();
}

// LOOP --------------------------------------------------------------------------------------------

// FPS throttle.
uint32_t prevMicros;
// Speed check.
uint32_t prevMillis = 0;
// Counts up every frame for "every so often" items.
// By starting at 0, actions are called the first loop as well.
uint32_t loopCounter = 0;

void loop(void) {
  // FPS Throttle.
  uint32_t t;
  while (((t = micros()) - prevMicros) < (1000000L / MAX_FPS));
  prevMicros = t;

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
          bottle->illuminate(control.static_color);
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

  // Interwebs main loop.
  interwebs.loop();

  // Set LED status depending on interwebs status.
  if (!interwebs.wifiIsConnected()) {
    ledStatus(STATUS_WIFI_OFFLINE);
  } else if (!interwebs.mqttIsConnected()) {
    ledStatus(STATUS_MQTT_OFFLINE);
  } else if (interwebs.mqttIsActive()) {
    ledStatus(STATUS_MQTT_ACTIVE);
  } else {
    ledStatus(STATUS_OK);
  }

  // Send status update.
  every_n_seconds(240) {
    control.mqttCurrentStatus();
  }

  // Push all pixel changes to bottles.
  pxl8.show();

  // Check memory available.
  every_n_seconds(120) {
    Serial.print(F("Free Memory: "));
    Serial.print(freeMemory() * 0.001f, 2);
    Serial.println(F(" KB")); // 192KB total
  }

  // Check voltage.
  // every_n_seconds(125) {
  //   Serial.print("Bus Voltage:   "); Serial.print(voltageMonitor.getBusVoltage_V()); Serial.println(" V");
  //   Serial.print("Shunt Voltage: "); Serial.print(voltageMonitor.getShuntVoltage_mV()); Serial.println(" mV");
  //   Serial.print("Load Voltage:  "); Serial.print(voltageMonitor.getLoadVoltage()); Serial.println(" V");
  //   Serial.print("Current:       "); Serial.print(voltageMonitor.getCurrent_mA()); Serial.println(" mA");
  //   Serial.print("Power:         "); Serial.print(voltageMonitor.getPower_mW()); Serial.println(" mW");
  // }

  // Speed check.
  uint32_t m = millis();
  if (prevMillis != 0 && m > prevMillis) { // skips first, ignores millis() overflow
    uint32_t s = m - prevMillis;
    if (s > 14) {
      Serial.print(F("Slow frame (ms): "));
      Serial.println(String(s));
    }
  }
  prevMillis = m;

  loopCounter++;
}

#ifdef __arm__
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
