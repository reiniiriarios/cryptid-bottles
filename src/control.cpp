#include "control.h"

Control::Control(Pxl8* pxl8, Interwebs* interwebs, std::vector<Bottle*>* bottles)
  : pxl8(pxl8), interwebs(interwebs), bottles(bottles) {}

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
  interwebs->addDiscovery("homeassistant/light/cryptid-bottles/cryptidBottles/config", discoveryJson);
  interwebs->addDiscovery("homeassistant/select/glow_speed/cryptidBottles/config", discoveryJsonGlowSpeed);
  interwebs->addDiscovery("homeassistant/select/faerie_speed/cryptidBottles/config", discoveryJsonFaerieSpeed);

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
      pStr = "medium"; // default
    }
    Serial.print(F("Setting glow speed to "));
    Serial.println(pStr);
    glowSpeed = GLOW_SPEED.at(pStr);
    mqttCurrentStatus();
  });

  // Set the faerie animation speed.
  interwebs->onMqtt("cryptid/bottles/faerie_speed/set", [&](char* payload, uint16_t /*len*/){
    String pStr = String(payload);
    bottleAnimation = BOTTLE_ANIMATION_FAERIES;
    if (FAERIE_SPEED.find(pStr) == FAERIE_SPEED.end()) {
      pStr = "medium"; // default
    }
    Serial.print(F("Setting faerie speed to "));
    Serial.println(pStr);
    faerieSpeed = FAERIE_SPEED.at(pStr);
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
    "\"effect\":\"" + BOTTLE_ANIMATIONS_INV.at(bottleAnimation) + "\","
    "\"glow_speed\":\"" + GLOW_SPEED_INV.at(glowSpeed) + "\","
    "\"faerie_speed\":\"" + FAERIE_SPEED_INV.at(faerieSpeed) + "\"}";
  interwebs->mqttSendMessage("cryptid/bottles/state", payload);
}
