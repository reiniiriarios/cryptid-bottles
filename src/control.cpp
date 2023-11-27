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
  interwebs->onMqtt("cryptid/bottles/animation/set", [&](String &payload){
    pixelsOn = true;
    if (BOTTLE_ANIMATIONS.find(payload) == BOTTLE_ANIMATIONS.end()) {
      payload = "warning"; // not found
    }
    Serial.println("Setting animation to " + payload);
    bottleAnimation = BOTTLE_ANIMATIONS.at(payload);
    mqttCurrentStatus();
  });

  // Set the glow animation speed.
  interwebs->onMqtt("cryptid/bottles/glow-speed/set", [&](String &payload){
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
  interwebs->onMqtt("cryptid/bottles/faerie-speed/set", [&](String &payload){
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
    uint8_t b = min(max(0, round(payload.toFloat() * 2.55f)), 255);
    Serial.println("Setting brightness to " + String(b));
    String on;
    if (b == 0) {
      pixelsOn = false;
      on = "OFF";
      for (auto & bottle : *bottles) {
        bottle->blank();
      };
    } else {
      pixelsOn = true;
      on = "ON";
    }
    pxl8->setBrightness(b);
    brightness = b;
    mqttCurrentStatus();
  });

  // Set white balance in degrees kelvin.
  interwebs->onMqtt("cryptid/bottles/white-balance/set", [&](String &payload){
    int8_t wb = payload.toInt();
    if (wb < -5 || wb > 5) wb = 0; // default
    white_balance = (white_balance_t)wb;
    Serial.print("Setting white balance to " + String(wb) + "...");
    mqttCurrentStatus();
  });

  // Send discovery when Home Assistant notifies it's online.
  interwebs->onMqtt("homeassistant/status", [&](String &payload){
    if (payload == "online") {
      sendDiscoveryAll();
      mqttCurrentStatus();
    }
  });
}

void Control::mqttCurrentStatus(void) {
  String on = "ON";
  if (!pixelsOn) on = "OFF";
  String payload = "{";
  payload += "\"on\":\"" + on + "\",";
  payload += "\"brightness\":\"" + String(brightness) + "\",";
  payload += "\"white-balance\":\"" + String((int8_t)white_balance) + "\",";
  payload += "\"animation\":\"" + BOTTLE_ANIMATIONS_INV.at(bottleAnimation) + "\",",
  payload += "\"glow-speed\":\"" + GLOW_SPEED_INV.at(glowSpeed) + "\",",
  payload += "\"faerie-speed\":\"" + FAERIE_SPEED_INV.at(faerieSpeed) + "\"",
  payload += "}";
  interwebs->mqttSendMessage("cryptid/bottles/state", payload);
}

bool Control::sendDiscoveryAll(void) {
  bool success = true;
  success = success && sendDiscoverySwitch("on", "On/Off");
  success = success && sendDiscoveryNumber("brightness", 0, 100, "Brightness");
  success = success && sendDiscoveryNumber("white-balance", -5, 5, "White Balance");
  success = success && sendDiscoverySelect("animation", BOTTLE_ANIMATIONS, "Animation");
  success = success && sendDiscoverySelect("glow-speed", GLOW_SPEED, "Glow Speed");
  success = success && sendDiscoverySelect("faerie-speed", FAERIE_SPEED, "Faerie Speed");
  return success;
}

bool Control::sendDiscoverySwitch(String id, String name) {
  return sendDiscovery("switch", name, id, "");
}

bool Control::sendDiscoveryNumber(String id, uint32_t min, uint32_t max, String name) {
  String addl = "\"min\":"+String(min)+",\"max\":"+String(max)+",\"mode\":\"slider\",";
  return sendDiscovery("number", name, id, addl);
}

template<typename T>
bool Control::sendDiscoverySelect(String id, std::map<String, T> options, String name) {
  String addl = "\"options\":[";
  bool f = true;
  for (auto const& x : options) {
    if (!f) addl += ",";
    addl += "\"" + x.first + "\"";
    f = false;
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
  payload += "\"state_topic\":\"cryptid/bottles/state\",";
  payload += "\"command_topic\":\"cryptid/bottles/" + id + "/set\",";
  payload += "\"value_template\":\"{{ value_json." + id + " }}\",",
  payload += addl;
  payload += "\"unique_id\":\"cryptid-bottles-" + id + "\",";
  payload += "\"device\":{";
  payload += "\"identifiers\":[\"cryptidBottles\"],";
  payload += "\"name\":\"Cryptid Bottles\"";
  payload += "}}";
  return interwebs->mqttPublish(topic, payload);
}
