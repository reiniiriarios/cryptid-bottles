#include "control.h"

Control::Control(Pxl8 *pxl8, Interwebs *interwebs, Bottle *bottles, uint8_t num_bottles)
  : pxl8(pxl8), interwebs(interwebs), bottles(bottles), num_bottles(num_bottles) {}

void Control::initMQTT(void) {
  Serial.println("Setting up MQTT control...");
  // Enable birth and last will and testament.
  interwebs->setBirthLWTtopic("cryptid/bottles/status");
  // Turn lights on or off.
  interwebs->onMqtt("cryptid/bottles/on/set", [&](String &payload){
    if (payload == "on" || payload == "ON" || payload.toInt() == 1) {
      pixelsOn = true;
      if (brightness == 0) {
        brightness = 127;
      }
      interwebs->mqttSendMessage("cryptid/bottles/on/status", "ON");
    }
    else if (payload == "off" || payload == "OFF" || payload.toInt() == 0) {
      pixelsOn = false;
      for (int i = 0; i < num_bottles; i++) {
        bottles[i].blank();
      };
      interwebs->mqttSendMessage("cryptid/bottles/on/status", "OFF");
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
    uint8_t b = min(0,max(round(payload.toFloat() * 2.55f),255));
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
    interwebs->mqttSendMessage("cryptid/bottles/on/status", on);
    interwebs->mqttSendMessage("cryptid/bottles/brightness/status", String(brightness));
  });

  // Send discovery when Home Assistant notifies it's online.
  interwebs->onMqtt("homeassistant/status", [&](String &payload){
    if (payload == "online") {
      sendDiscoveryAll();
      mqttCurrentStatus();
    }
  });

  // Set white balance in degrees kelvin.
  interwebs->onMqtt("cryptid/bottles/white-balance/set", [&](String &payload){
    uint16_t k = min(1000,max(round(payload.toInt()),10000));
    white_kelvin = k;
    rgb_t c = kelvin2rgb(k);
    white_color = pxl8->color(c.r, c.g, c.b);
  });
}

void Control::mqttCurrentStatus(void) {
  String on = "ON";
  if (!pixelsOn) on = "OFF";
  interwebs->mqttSendMessage("cryptid/bottles/on/status", on);
  interwebs->mqttSendMessage("cryptid/bottles/animation/status", BOTTLE_ANIMATIONS_INV.at(bottleAnimation));
  interwebs->mqttSendMessage("cryptid/bottles/glow-speed/status", GLOW_SPEED_INV.at(glowSpeed));
  interwebs->mqttSendMessage("cryptid/bottles/faerie-speed/status", FAERIE_SPEED_INV.at(faerieSpeed));
  interwebs->mqttSendMessage("cryptid/bottles/brightness/status", String(brightness));
  interwebs->mqttSendMessage("cryptid/bottles/white-balance/status", String(white_kelvin));
}

bool Control::sendDiscoveryAll(void) {
  bool success = true;
  success = success && sendDiscoverySwitch("on");
  success = success && sendDiscoveryNumber("brightness", 0, 100);
  success = success && sendDiscoveryNumber("white-balance", 1000, 10000);
  success = success && sendDiscoverySelect("animation", BOTTLE_ANIMATIONS);
  success = success && sendDiscoverySelect("glow-speed", GLOW_SPEED, "glow speed");
  success = success && sendDiscoverySelect("faerie-speed", FAERIE_SPEED, "faerie speed");
  return success;
}

bool Control::sendDiscoverySwitch(String id, String name = "") {
  return sendDiscovery("switch", name, id, "");
}

bool Control::sendDiscoveryNumber(String id, uint32_t min, uint32_t max, String name = "") {
  String addl = "\"min\":"+String(min)+",\"max\":"+String(max)+",\"mode\":\"slider\",";
  return sendDiscovery("number", name, id, addl);
}

template<typename T>
bool Control::sendDiscoverySelect(String id, std::map<String, T> options, String name = "") {
  String addl = "\"options\":[";
  for (auto const& x : options) {
    addl += "\"" + x.first + "\",";
  }
  addl += "],";
  return sendDiscovery("select", name, id, addl);
}

bool Control::sendDiscovery(String type, String name, String id, String addl) {
  Serial.println("Sending MQTT discovery for '" + id + "'");
  if (name == "") name = id;
  // Manually building this here makes more sense than including a JSON library.
  String topic = "homeassistant/" + type + "/" + id + "/cryptidBottles/config";
  String payload = "{";
  payload += "\"name\":\"" + name + "\",";
  payload += "\"state_topic\":\"cryptid/bottles/" + id + "/state\",";
  payload += "\"command_topic\":\"cryptid/bottles/" + id + "/set\",";
  payload += addl;
  payload += "\"unique_id\":\"cryptid-bottles-" + id + "\",";
  payload += "\"device\":{";
  payload += "\"identifiers\":[\"cryptidBottles\"],";
  payload += "\"name\":\"Cryptid Bottles\"";
  payload += "}}";
  return interwebs->mqttPublish(topic, payload);
}
