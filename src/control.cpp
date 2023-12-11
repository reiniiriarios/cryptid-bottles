#include "control.h"

Control::Control(Pxl8* pxl8, MQTT_Looped* interwebs, std::vector<Bottle*>* bottles)
  : pxl8(pxl8), interwebs(interwebs), bottles(bottles) {
  this->lastGlowChange = millis();
}

// ---------- MQTT Commands ----------

void Control::turnOn(void) {
  Serial.println(F("Turning light on"));
  pixelsOn = true;
  if (brightness == 0) {
    brightness = 127;
    pxl8->setBrightness(brightness);
  }
}

void Control::turnOff(void) {
  Serial.println(F("Turning light off"));
  pixelsOn = false;
  for (auto & bottle : *bottles) {
    bottle->blank();
  }
}

void Control::initMQTT(void) {
  Serial.println(F("Setting up MQTT control..."));

  // Enable birth and last will and testament.
  interwebs->setBirth("cryptid/bottles/status", "online");
  interwebs->setWill("cryptid/bottles/status", "offline");

  // Add discoveries for each device.
  interwebs->addDiscovery("homeassistant/light/cryptid-bottles/cryptidBottles/config", discoveryJson.c_str());
  // The `light` type has most settings, but these two do not fit within the spec.
  interwebs->addDiscovery("homeassistant/select/glow_speed/cryptidBottles/config", discoveryJsonGlowSpeed.c_str());
  interwebs->addDiscovery("homeassistant/select/faerie_speed/cryptidBottles/config", discoveryJsonFaerieSpeed.c_str());
  // Power. Zap.
  interwebs->addDiscovery("homeassistant/sensor/bus_v/cryptidBottles/config", discoveryJsonBusVoltage.c_str());
  interwebs->addDiscovery("homeassistant/sensor/shunt_v/cryptidBottles/config", discoveryJsonShuntVoltage.c_str());
  interwebs->addDiscovery("homeassistant/sensor/load_v/cryptidBottles/config", discoveryJsonLoadVoltage.c_str());
  interwebs->addDiscovery("homeassistant/sensor/power/cryptidBottles/config", discoveryJsonPower.c_str());
  interwebs->addDiscovery("homeassistant/sensor/current/cryptidBottles/config", discoveryJsonCurrent.c_str());
  interwebs->addDiscovery("homeassistant/sensor/avg_current/cryptidBottles/config", discoveryJsonAvgCurrent.c_str());

  // Turn lights on or off.
  interwebs->onMqtt("cryptid/bottles/on/set", [&](char* payload, uint16_t /*len*/){
    String pStr = String(payload);
    if (pStr == "ON" || pStr == "on" || pStr == "1") {
      turnOn();
    } else if (pStr == "OFF" || pStr == "on" || pStr == "0") {
      turnOff();
    } else {
      Serial.print(F("Unrecognized on/off command: "));
      Serial.println(pStr);
    }
    mqttCurrentStatus();
  });

  // Set the bottles animation.
  interwebs->onMqtt("cryptid/bottles/effect/set", [&](char* payload, uint16_t /*len*/){
    pixelsOn = true;
    if (BOTTLE_ANIMATIONS.find(payload) == BOTTLE_ANIMATIONS.end()) {
      Serial.print(F("Effect not found: "));
      Serial.println(payload);
      bottleAnimation = BOTTLE_ANIMATION_WARNING;
    }
    else {
      Serial.print(F("Setting effect to "));
      Serial.println(payload);
      bottleAnimation = BOTTLE_ANIMATIONS.at(payload);
    }
    turnOn();
    mqttCurrentStatus();
  });

  // Set the glow animation speed.
  interwebs->onMqtt("cryptid/bottles/glow_speed/set", [&](char* payload, uint16_t /*len*/){
    String pStr = String(payload);
    if (bottleAnimation != BOTTLE_ANIMATION_GLOW) {
      bottleAnimation = BOTTLE_ANIMATION_FAERIES;
    }
    if (GLOW_SPEED.find(pStr) == GLOW_SPEED.end()) {
      Serial.println(F("Setting glow speed to default"));
      glowSpeed = GLOW_SPEED_MEDIUM;
    } else {
      Serial.print(F("Setting glow speed to "));
      Serial.println(pStr);
      glowSpeed = GLOW_SPEED.at(pStr);
    }
    mqttCurrentStatus();
  });

  // Set the faerie animation speed.
  interwebs->onMqtt("cryptid/bottles/faerie_speed/set", [&](char* payload, uint16_t /*len*/){
    String pStr = String(payload);
    bottleAnimation = BOTTLE_ANIMATION_FAERIES;
    if (FAERIE_SPEED.find(pStr) == FAERIE_SPEED.end()) {
      Serial.println(F("Setting faerie speed to default"));
      faerieSpeed = FAERIE_SPEED_MEDIUM;
    } else {
      Serial.print(F("Setting faerie speed to "));
      Serial.println(pStr);
      faerieSpeed = FAERIE_SPEED.at(pStr);
    }
    mqttCurrentStatus();
  });

  // Set the bottles brightness.
  interwebs->onMqtt("cryptid/bottles/brightness/set", [&](char* payload, uint16_t /*len*/){
    brightness = min(max(0, strtol(payload, nullptr, 10)), 255);
    Serial.print(F("Setting brightness to "));
    Serial.println(String(brightness));
    if (brightness == 0) {
      turnOff();
    } else {
      turnOn();
    }
    pxl8->setBrightness(brightness);
    mqttCurrentStatus();
  });

  // Set white balance in degrees kelvin.
  interwebs->onMqtt("cryptid/bottles/rgb/set", [&](char* payload, uint16_t len){
    String pStr = String(payload);
    int c1 = pStr.indexOf(",");
    int c2 = pStr.lastIndexOf(",");
    // -1 -> not found || c1 == c2 -> only one comma || no chars after second comma
    if (c1 == -1 || c2 == -1 || c1 == c2 || len < c2 + 1) {
      Serial.print(F("Invalid color: "));
      Serial.println(pStr);
      static_color = rgb_t{ 255, 255, 255 };
    }
    else {
      uint8_t r = pStr.substring(0, c1).toInt(),
              g = pStr.substring(c1 + 1, c2).toInt(),
              b = pStr.substring(c2 + 1).toInt();
      Serial.print(F("Setting color to "));
      Serial.println(pStr);
      static_color = rgb_t{ r, g, b };
    }
    bottleAnimation = BOTTLE_ANIMATION_ILLUM;
    turnOn();
    mqttCurrentStatus();
  });

  // Set to a given brightness at the current white balance.
  interwebs->onMqtt("cryptid/bottles/white/set", [&](char* payload, uint16_t /*len*/){
    brightness = min(max(0, strtol(payload, nullptr, 10)), 255);
    Serial.print(F("Setting illumination to "));
    Serial.println(String(white_balance) + "@" + String(brightness));
    static_color = WHITE_TEMPERATURES.at(white_balance);
    if (brightness == 0) {
      turnOff();
    } else {
      turnOn();
    }
    pxl8->setBrightness(brightness);
    bottleAnimation = BOTTLE_ANIMATION_ILLUM;
    mqttCurrentStatus();
  });

  // Set white balance in degrees kelvin.
  interwebs->onMqtt("cryptid/bottles/white_balance/set", [&](char* payload, uint16_t /*len*/){
    white_balance = white_balance_t(min(max(MIN_WB_MIRED, roundmired(strtol(payload, nullptr, 10))), MAX_WB_MIRED));
    Serial.print(F("Setting white balance to "));
    Serial.println(String(white_balance));
    static_color = WHITE_TEMPERATURES.at(white_balance);
    bottleAnimation = BOTTLE_ANIMATION_ILLUM;
    turnOn();
    mqttCurrentStatus();
  });

  // Send discovery when Home Assistant notifies it's online.
  interwebs->onMqtt("homeassistant/status", [&](char* payload, uint16_t /*len*/){
    if (strcmp(payload, "online") == 0) {
      interwebs->sendDiscoveries();
      mqttCurrentStatus();
    }
  });
}

void Control::mqttCurrentStatus(void) {
  String on = "ON";
  if (!pixelsOn) on = "OFF";
  String payload = "{\"on\":\"" + on + "\","
    "\"brightness\":\"" + String(brightness) + "\","
    "\"rgb\":\"" + String(static_color.r) + "," + String(static_color.g) + "," + String(static_color.b) + "\","
    "\"white_balance\":\"" + String(white_balance) + "\","
    "\"effect\":\"" + this->getBottleAnimationString() + "\","
    "\"glow_speed\":\"" + this->getGlowSpeedString() + "\","
    "\"faerie_speed\":\"" + this->getFaerieSpeedString() + "\"}";
  interwebs->mqttSendMessage("cryptid/bottles/state", payload.c_str());
}

void Control::mqttCurrentSensors(void) {
  String payload = "{\"bus_v\":" + String(this->last_bus_voltage) + ","
    "\"shunt_v\":" + String(this->last_shunt_voltage) + ","
    "\"load_v\":" + String(this->last_load_voltage) + ","
    "\"power\":" + String(this->last_power) + ","
    "\"current\":" + String(this->last_current) + ","
    "\"avg_current\":" + String(this->last_avg_current) + "}";
  interwebs->mqttSendMessage("cryptid/bottles/sensor/state", payload.c_str());
}

String Control::getBottleAnimationString(void) {
  if (BOTTLE_ANIMATIONS_INV.find(this->bottleAnimation) == BOTTLE_ANIMATIONS_INV.end()) {
    this->bottleAnimation = BOTTLE_ANIMATION_DEFAULT;
  }
  return BOTTLE_ANIMATIONS_INV.at(this->bottleAnimation);
}

String Control::getGlowSpeedString(void) {
  if (GLOW_SPEED_INV.find(this->glowSpeed) == GLOW_SPEED_INV.end()) {
    this->glowSpeed = GLOW_SPEED_MEDIUM;
  }
  return GLOW_SPEED_INV.at(this->glowSpeed);
}

String Control::getFaerieSpeedString(void) {
  if (FAERIE_SPEED_INV.find(this->faerieSpeed) == FAERIE_SPEED_INV.end()) {
    this->faerieSpeed = FAERIE_SPEED_MEDIUM;
  }
  return FAERIE_SPEED_INV.at(this->faerieSpeed);
}

// ---------- Animation ----------

rgb_t Control::getRandomWhiteBalance(void) {
  auto it = WHITE_TEMPERATURES.begin();
  std::advance(it, rand() % WHITE_TEMPERATURES.size());
  return it->second;
}

bool Control::shouldChangeGlow(void) {
  if (millis() - this->lastGlowChange < 2500) return false; // Don't change too often.
  if (random(0, this->glowSpeed - 1000) == 0) return true;
  if (millis() - this->lastGlowChange > this->glowSpeed) return true;
  return false;
}

bool Control::shouldShowFaerie(void) {
  // If a faerie is already flying, keep displaying animation.
  if (this->faerieFlying) return true;
  // Randomly spawn a faerie.
  if (random(0, this->faerieSpeed - 1000) == 0) return true;
  // Timeout for spawning a faerie has been reached.
  if (millis() - this->lastFaerieFly > this->faerieSpeed) return true;
  return false;
}

void Control::updateRandomBottleHue(void) {
  uint8_t id = random(0, this->bottles->size());
  uint16_t hueStart = random(0, 360);
  uint16_t hueEnd = hueStart + random(30, 40);
  this->bottles->at(id)->setHue(hueStart, hueEnd, random(1500, 2500));
  this->lastGlowChange = millis();
}

void Control::updateRandomBottleWhiteBalance(void) {
  uint8_t id = random(0, this->bottles->size());
  rgb_t c = this->getRandomWhiteBalance();
  this->bottles->at(id)->setColor(c, random(1500, 2500));
  this->lastGlowChange = millis();
}

void Control::showFaerie(void) {
  // If a new faerie, pick a random bottle.
  if (this->faerieBottle == -1) {
    this->faerieBottle = random(0, this->bottles->size());
    this->bottles->at(this->faerieBottle)->spawnFaerie(random(8, 14) * 0.1);
  }
  this->faerieFlying = this->bottles->at(this->faerieBottle)->showFaerie();
  // After animation, reset bottle and log time.
  if (!this->faerieFlying) {
    this->faerieBottle = -1;
    this->lastFaerieFly = millis();
    this->faerieFlying = false;
  }
}
