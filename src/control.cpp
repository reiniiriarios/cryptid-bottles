#include "control.h"

Control::Control(Pxl8 *pxl8, Interwebs *interwebs, Bottle *bottles, uint8_t num_bottles)
  : pxl8(pxl8), interwebs(interwebs), bottles(bottles), num_bottles(num_bottles) {}

void Control::mqttCurrentStatus(void) {
  String on = "ON";
  if (!pixelsOn) on = "OFF";
  interwebs->mqttSendMessage("cryptid/bottles/status", on);
  interwebs->mqttSendMessage("cryptid/bottles/animation/status", BOTTLE_ANIMATIONS_INV.at(bottleAnimation));
  interwebs->mqttSendMessage("cryptid/bottles/glow-speed/status", GLOW_SPEED_INV.at(glowSpeed));
  interwebs->mqttSendMessage("cryptid/bottles/faerie-speed/status", FAERIE_SPEED_INV.at(faerieSpeed));
  interwebs->mqttSendMessage("cryptid/bottles/brightness/status", String(brightness));
}

void Control::initMQTT(void) {
  Serial.println("Setting up MQTT control...");
  // Turn lights on or off.
  interwebs->onMqtt("cryptid/bottles/set", [&](String &payload){
    if (payload == "on" || payload == "ON" || payload.toInt() == 1) {
      pixelsOn = true;
      if (brightness == 0) {
        brightness = 127;
      }
      interwebs->mqttSendMessage("cryptid/bottles/status", "ON");
    }
    else if (payload == "off" || payload == "OFF" || payload.toInt() == 0) {
      pixelsOn = false;
      for (int i = 0; i < num_bottles; i++) {
        bottles[i].blank();
      };
      interwebs->mqttSendMessage("cryptid/bottles/status", "OFF");
    }
  });

  // Set the bottles animation.
  interwebs->onMqtt("cryptid/bottles/animation/set", [&](String &payload){
    pixelsOn = true;
    if (BOTTLE_ANIMATIONS.find(payload) == BOTTLE_ANIMATIONS.end()) {
      payload = "warning"; // not found
    }
    bottleAnimation = BOTTLE_ANIMATIONS.at(payload);
    interwebs->mqttSendMessage("cryptid/bottles/animation/status", payload);
  });

  // Set the glow animation speed.
  interwebs->onMqtt("cryptid/bottles/glow-speed/set", [&](String &payload){
    if (bottleAnimation != BOTTLE_ANIMATION_GLOW) {
      bottleAnimation = BOTTLE_ANIMATION_FAERIES;
    }
    if (GLOW_SPEED.find(payload) == GLOW_SPEED.end()) {
      payload = "medium"; // default
    }
    glowSpeed = GLOW_SPEED.at(payload);
    interwebs->mqttSendMessage("cryptid/bottles/glow-speed/status", payload);
    interwebs->mqttSendMessage("cryptid/bottles/animation/status", BOTTLE_ANIMATIONS_INV.at(bottleAnimation));
  });

  // Set the glow animation speed.
  interwebs->onMqtt("cryptid/bottles/faerie-speed/set", [&](String &payload){
    bottleAnimation = BOTTLE_ANIMATION_FAERIES;
    if (FAERIE_SPEED.find(payload) == FAERIE_SPEED.end()) {
      payload = "medium"; // default
    }
    faerieSpeed = FAERIE_SPEED.at(payload);
    interwebs->mqttSendMessage("cryptid/bottles/faerie-speed/status", payload);
    interwebs->mqttSendMessage("cryptid/bottles/animation/status", BOTTLE_ANIMATIONS_INV.at(BOTTLE_ANIMATION_FAERIES));
  });

  // Set the bottles brightness.
  interwebs->onMqtt("cryptid/bottles/brightness/set", [&](String &payload){
    uint8_t b = payload.toInt() & 0xFF;
    String on;
    if (b == 0) {
      pixelsOn = false;
      on = "OFF";
      for (int i = 0; i < num_bottles; i++) {
        bottles[i].blank();
      };
    } else {
      pixelsOn = true;
      on = "ON";
    }
    pxl8->setBrightness(b);
    brightness = b;
    interwebs->mqttSendMessage("cryptid/bottles/status", on);
    interwebs->mqttSendMessage("cryptid/bottles/brightness/status", String(brightness));
  });

  // Send discovery when Home Assistant notifies it's online.
  interwebs->onMqtt("homeassistant/status", [&](String &payload){
    if (payload == "online") {
      interwebs->mqttSendDiscovery();
      mqttCurrentStatus();
    }
  });
}
