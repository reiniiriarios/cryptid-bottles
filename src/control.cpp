#include "control.h"

Control::Control(Pxl8* pxl8, Interwebs* interwebs, std::vector<Bottle*>* bottles)
  : pxl8(pxl8), interwebs(interwebs), bottles(bottles) {}

void Control::turnOn(void) {
  pixelsOn = true;
  if (brightness == 0) {
    brightness = 127;
    pxl8->setBrightness(brightness);
  }
}

void Control::turnOff(void) {
  pixelsOn = false;
  for (auto & bottle : *bottles) {
    bottle->blank();
  }
}

void Control::initMQTT(void) {
  Serial.println(F("Setting up MQTT control..."));

  // Enable birth and last will and testament.
  interwebs->setBirthLWTtopic("cryptid/bottles/status");

  // Turn lights on or off.
  interwebs->onMqtt("cryptid/bottles/on/set", [&](String &payload){
    Serial.print(F("Setting on/off to "));
    Serial.println(payload);
    if (payload == "on" || payload == "ON" || payload.toInt() == 1) {
      turnOn();
    }
    else if (payload == "off" || payload == "OFF" || payload.toInt() == 0) {
      turnOff();
    }
    mqttCurrentStatus();
  });

  // Set the bottles animation.
  interwebs->onMqtt("cryptid/bottles/effect/set", [&](String &payload){
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
  interwebs->onMqtt("cryptid/bottles/glow_speed/set", [&](String &payload){
    if (bottleAnimation != BOTTLE_ANIMATION_GLOW) {
      bottleAnimation = BOTTLE_ANIMATION_FAERIES;
    }
    if (GLOW_SPEED.find(payload) == GLOW_SPEED.end()) {
      payload = "medium"; // default
    }
    Serial.print(F("Setting glow speed to "));
    Serial.println(payload);
    glowSpeed = GLOW_SPEED.at(payload);
    mqttCurrentStatus();
  });

  // Set the faerie animation speed.
  interwebs->onMqtt("cryptid/bottles/faerie_speed/set", [&](String &payload){
    bottleAnimation = BOTTLE_ANIMATION_FAERIES;
    if (FAERIE_SPEED.find(payload) == FAERIE_SPEED.end()) {
      payload = "medium"; // default
    }
    Serial.print(F("Setting faerie speed to "));
    Serial.println(payload);
    faerieSpeed = FAERIE_SPEED.at(payload);
    mqttCurrentStatus();
  });

  // Set the bottles brightness.
  interwebs->onMqtt("cryptid/bottles/brightness/set", [&](String &payload){
    brightness = min(max(0, payload.toInt()), 255);
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
  interwebs->onMqtt("cryptid/bottles/rgb/set", [&](String &payload){
    int c1 = payload.indexOf(",");
    int c2 = payload.lastIndexOf(",");
    // -1 -> not found || c1 == c2 -> only one comma || no chars after second comma
    if (c1 == -1 || c2 == -1 || c1 == c2 || payload.length() < c2 + 1) {
      Serial.print(F("Invalid color: "));
      Serial.println(payload);
      static_color = rgb_t{ 255, 255, 255 };
    }
    else {
      uint8_t r = payload.substring(0, c1).toInt(),
              g = payload.substring(c1 + 1, c2).toInt(),
              b = payload.substring(c2 + 1).toInt();
      Serial.print(F("Setting color to "));
      Serial.println(payload);
      static_color = rgb_t{ r, g, b };
    }
    bottleAnimation = BOTTLE_ANIMATION_ILLUM;
    turnOn();
    mqttCurrentStatus();
  });

  // Set to a given brightness at the current white balance.
  interwebs->onMqtt("cryptid/bottles/white/set", [&](String &payload){
    brightness = min(max(0, payload.toInt()), 255);
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
  interwebs->onMqtt("cryptid/bottles/white_balance/set", [&](String &payload){
    white_balance = white_balance_t(min(max(MIN_WB_MIRED, roundmired(payload.toInt())), MAX_WB_MIRED));
    Serial.print(F("Setting white balance to "));
    Serial.println(String(white_balance));
    static_color = WHITE_TEMPERATURES.at(white_balance);
    bottleAnimation = BOTTLE_ANIMATION_ILLUM;
    turnOn();
    mqttCurrentStatus();
  });

  // Send discovery when Home Assistant notifies it's online.
  interwebs->onMqtt("homeassistant/status", [&](String &payload){
    if (payload == "online") {
      sendDiscovery();
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

bool Control::sendDiscovery() {
  Serial.println(F("Sending MQTT discovery for HASS"));
  bool success = interwebs->mqttPublish("homeassistant/light/cryptid-bottles/cryptidBottles/config", discoveryJson);
  // Addl controls that don't fall under "light":
  success = success && interwebs->mqttPublish("homeassistant/select/glow_speed/cryptidBottles/config", discoveryJsonGlowSpeed);
  success = success && interwebs->mqttPublish("homeassistant/select/faerie_speed/cryptidBottles/config", discoveryJsonFaerieSpeed);
  return success;
}
