#include "control.h"

Control::Control(Pxl8* pxl8, Interwebs* interwebs, std::vector<Bottle*>* bottles)
  : pxl8(pxl8), interwebs(interwebs), bottles(bottles) {}

void Control::initMQTT(void) {
  Serial.println("Setting up MQTT control...");

  // Enable birth and last will and testament.
  interwebs->setBirthLWTtopic("cryptid/bottles/status");

  // Turn lights on or off.
  interwebs->onMqtt("cryptid/bottles/on/set", [&](String &payload){
    Serial.println("Setting on/off to " + payload);
    if (payload == "on" || payload == "ON" || payload.toInt() == 1) {
      pixelsOn = true;
      if (brightness == 0) {
        brightness = 127;
      }
    }
    else if (payload == "off" || payload == "OFF" || payload.toInt() == 0) {
      pixelsOn = false;
      for (auto & bottle : *bottles) {
        bottle->blank();
      };
    }
    mqttCurrentStatus();
  });

  // Set the bottles animation.
  interwebs->onMqtt("cryptid/bottles/effect/set", [&](String &payload){
    pixelsOn = true;
    if (BOTTLE_ANIMATIONS.find(payload) == BOTTLE_ANIMATIONS.end()) {
      Serial.println("Effect '" + payload + "' not found");
      bottleAnimation = BOTTLE_ANIMATION_WARNING;
    }
    else {
      Serial.println("Setting effect to " + payload);
      bottleAnimation = BOTTLE_ANIMATIONS.at(payload);
    }
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
    Serial.println("Setting glow speed to " + payload);
    glowSpeed = GLOW_SPEED.at(payload);
    mqttCurrentStatus();
  });

  // Set the faerie animation speed.
  interwebs->onMqtt("cryptid/bottles/faerie_speed/set", [&](String &payload){
    bottleAnimation = BOTTLE_ANIMATION_FAERIES;
    if (FAERIE_SPEED.find(payload) == FAERIE_SPEED.end()) {
      payload = "medium"; // default
    }
    Serial.println("Setting faerie speed to " + payload);
    faerieSpeed = FAERIE_SPEED.at(payload);
    mqttCurrentStatus();
  });

  // Set the bottles brightness.
  interwebs->onMqtt("cryptid/bottles/brightness/set", [&](String &payload){
    brightness = min(max(0, payload.toInt()), 255);
    Serial.println("Setting brightness to " + String(brightness));
    if (brightness == 0) {
      pixelsOn = false;
      for (auto & bottle : *bottles) {
        bottle->blank();
      };
    } else {
      pixelsOn = true;
    }
    pxl8->setBrightness(brightness);
    mqttCurrentStatus();
  });

  // Set white balance in degrees kelvin.
  interwebs->onMqtt("cryptid/bottles/rgb/set", [&](String &payload){
    stringstream ss(payload.c_str());
    vector<uint8_t> rgb;
    while (ss.good()) {
      string substr;
      getline(ss, substr, ',');
      rgb.push_back(atoi(substr.c_str()));
    }
    if (rgb.size() != 3) {
      Serial.println("Invalid color '" + String(payload) + "'");
      static_color = rgb_t{ 255, 255, 255 };
    } else {
      Serial.println("Setting color to " + String(payload));
      static_color = rgb_t{ rgb.at(0), rgb.at(1), rgb.at(2) };
    }
    bottleAnimation = BOTTLE_ANIMATION_ILLUM;
    mqttCurrentStatus();
  });

  // Set to a given brightness at the current white balance.
  interwebs->onMqtt("cryptid/bottles/white/set", [&](String &payload){
    brightness = min(max(0, payload.toInt()), 255);
    Serial.println("Setting illumination to " + String(white_balance) + " at " + String(brightness));
    static_color = WHITE_TEMPERATURES.at(white_balance);
    if (brightness == 0) {
      pixelsOn = false;
      for (auto & bottle : *bottles) {
        bottle->blank();
      };
    } else {
      pixelsOn = true;
    }
    pxl8->setBrightness(brightness);
    bottleAnimation = BOTTLE_ANIMATION_ILLUM;
    mqttCurrentStatus();
  });

  // Set white balance in degrees kelvin.
  interwebs->onMqtt("cryptid/bottles/white_balance/set", [&](String &payload){
    white_balance = white_balance_t(min(max(MIN_WB_MIRED, roundmired(payload.toInt())), MAX_WB_MIRED));
    Serial.println("Setting white balance to " + String(white_balance) + "...");
    static_color = WHITE_TEMPERATURES.at(white_balance);
    bottleAnimation = BOTTLE_ANIMATION_ILLUM;
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
  Serial.println("Sending MQTT discovery for HASS");
  bool success = interwebs->mqttPublish("homeassistant/light/cryptid-bottles/cryptidBottles/config", discoveryJson);
  // Addl controls that don't fall under "light":
  success = success && interwebs->mqttPublish("homeassistant/select/glow_speed/cryptidBottles/config", discoveryJsonGlowSpeed);
  success = success && interwebs->mqttPublish("homeassistant/select/faerie_speed/cryptidBottles/config", discoveryJsonFaerieSpeed);
  return success;
}
