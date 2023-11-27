#include "control.h"

Control::Control(Pxl8* pxl8, Interwebs* interwebs, std::vector<Bottle*>* bottles)
  : pxl8(pxl8), interwebs(interwebs), bottles(bottles) {}

rgb_t Control::getWhiteBalanceRGB(void) {
  return WHITE_TEMPERATURES.at(white_balance);
}

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
      payload = "warning"; // not found
    }
    Serial.println("Setting effect to " + payload);
    bottleAnimation = BOTTLE_ANIMATIONS.at(payload);
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
    String on;
    if (brightness == 0) {
      pixelsOn = false;
      on = "OFF";
      for (auto & bottle : *bottles) {
        bottle->blank();
      };
    } else {
      pixelsOn = true;
      on = "ON";
    }
    pxl8->setBrightness(brightness);
    mqttCurrentStatus();
  });

  // Set white balance in degrees kelvin.
  interwebs->onMqtt("cryptid/bottles/white_balance/set", [&](String &payload){
    white_balance = white_balance_t(min(max(MIN_WB_MIRED, roundmired(payload.toInt())), MAX_WB_MIRED));
    Serial.print("Setting white balance to " + String(white_balance) + "...");
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
  String payload = "{\"on\":\"" + on + "\",";
  payload += "\"brightness\":\"" + String(brightness) + "\",";
  payload += "\"white_balance\":\"" + String((int8_t)white_balance) + "\",";
  payload += "\"effect\":\"" + BOTTLE_ANIMATIONS_INV.at(bottleAnimation) + "\",",
  payload += "\"glow_speed\":\"" + GLOW_SPEED_INV.at(glowSpeed) + "\",",
  payload += "\"faerie_speed\":\"" + FAERIE_SPEED_INV.at(faerieSpeed) + "\"}";
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
