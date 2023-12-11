#ifndef CRYPTID_CONTROL_H
#define CRYPTID_CONTROL_H

#include <vector>
#include <MQTT_Looped.h>
#include "def.h"
#include "bottle.h"

/**
 * @brief Convert map values to a JSON string array.
 *
 * @tparam T 
 * @param v 
 * @return const String 
 */
template<typename T>
const inline String jsonStr(std::map<String, T> v) {
  String s = "[";
  bool f = true;
  for (auto const& x : v) {
    if (!f) s += ",";
    s += "\"" + x.first + "\"";
    f = false;
  }
  s += "]";
  return s;
}

/**
 * @brief Discovery JSON for light.
 *
 * @see https://www.home-assistant.io/integrations/mqtt
 */
const static String discoveryJson = []() -> String {
  String json = F(R"JSON({
    "~":"cryptid/bottles",
    "name":"Cryptid Bottles",
    "uniq_id":"cryptid-bottles",
    "ic":"mdi:bottle-tonic-outline",
    "stat_t":"~/state",
    "stat_val_tpl":"{{ value_json.on }}",
    "cmd_t":"~/on/set",
    "on_cmd_type":"brightness",
    "bri_cmd_t":"~/brightness/set",
    "bri_val_tpl":"{{ value_json.brightness }}",
    "bri_scl":255,
    "rgb_cmd_t":"~/rgb/set",
    "rgb_val_tpl":"{{ value_json.rgb }}",
    "whit_cmd_t":"~/white/set",
    "whit_scl":255,
    "clr_temp_cmd_t":"~/white_balance/set",
    "clr_temp_val_tpl":"{{ value_json.white_balance }}",
    "min_mirs":)JSON");
  json += String(MIN_WB_MIRED);
  json += F(R"JSON(,
    "max_mirs":)JSON");
  json += String(MAX_WB_MIRED);
  json += F(R"JSON(,
    "fx_cmd_t":"~/effect/set",
    "fx_list":)JSON");
  json += jsonStr(BOTTLE_ANIMATIONS);
  json += F(R"JSON(,
    "fx_val_tpl":"{{ value_json.effect }}",
    "dev":{"ids":["cryptidBottles"],"name":"Cryptid Bottles"}})JSON");
  json.replace("\n    ",""); // shrink data
  return json;
}();

/**
 * @brief Get discovery JSON for Select setting.
 * 
 * @tparam T map setting
 * @param id
 * @param name
 * @param icon material design icon
 * @param options map<String value, enum setting>
 * @return const String
 *
 * @see https://www.home-assistant.io/integrations/mqtt
 * @see https://pictogrammers.com/library/mdi/
 */
template<typename T>
const static String discoverySelect(String id, String name, String icon, std::map<String, T> options) {
  return "{\"~\":\"cryptid/bottles\","
         "\"name\":\"" + name + "\","
         "\"uniq_id\":\"cryptid-bottles-" + id + "\","
         "\"ic\":\"mdi:" + icon + "\","
         "\"stat_t\":\"~/state\","
         "\"cmd_t\":\"~/" + id + "/set\","
         "\"val_tpl\":\"{{ value_json." + id + " }}\","
         "\"ops\":" + jsonStr(options) + ","
         "\"dev\":{\"ids\":[\"cryptidBottles\"],\"name\":\"Cryptid Bottles\"}}";
}

/**
 * @brief Discovery JSON for Glow Speed.
 */
const static String discoveryJsonGlowSpeed = discoverySelect("glow_speed", "Glow Speed", "play-speed", GLOW_SPEED);

/**
 * @brief Discovery JSON for Faerie Speed.
 */
const static String discoveryJsonFaerieSpeed = discoverySelect("faerie_speed", "Faerie Speed", "play-speed", FAERIE_SPEED);

/**
 * @brief Get discovery JSON for Sensor.
 * 
 * @tparam T map setting
 * @param id
 * @param name
 * @param device_class type of sensor/data
 * @param state_class measurement, total, or total_increasing
 * @param unit measurement unit, such as mW
 * @return const String
 *
 * @see https://www.home-assistant.io/integrations/mqtt
 * @see https://www.home-assistant.io/integrations/sensor/#device-class
 * @see https://developers.home-assistant.io/docs/core/entity/sensor/#available-state-classes
 */
const static String discoverySensor(String id, String name, String device_class, String state_class, String unit) {
  return "{\"~\":\"cryptid/bottles/sensor\","
         "\"name\":\"" + name + "\","
         "\"uniq_id\":\"cryptid-bottles-" + id + "\","
         "\"dev_cla\":\"" + device_class + "\","
         "\"stat_cla\":\"" + state_class + "\","
         "\"unit_of_meas\":\"" + unit + "\","
         "\"stat_t\":\"~/state\","
         "\"val_tpl\":\"{{ value_json." + id + " }}\","
         "\"dev\":{\"ids\":[\"cryptidBottles\"],\"name\":\"Cryptid Bottles\"}}";
}

/**
 * @brief Discovery JSON for Bus Voltage.
 */
const static String discoveryJsonBusVoltage = discoverySensor("bus_v", "Bus Voltage", "voltage", "measurement", "V");

/**
 * @brief Discovery JSON for Shunt Voltage.
 */
const static String discoveryJsonShuntVoltage = discoverySensor("shunt_v", "Shunt Voltage", "voltage", "measurement", "mV");

/**
 * @brief Discovery JSON for Load Voltage.
 */
const static String discoveryJsonLoadVoltage = discoverySensor("load_v", "Load Voltage", "voltage", "measurement", "V");

/**
 * @brief Discovery JSON for Power.
 */
const static String discoveryJsonPower = discoverySensor("power", "Power", "power", "measurement", "mW");

/**
 * @brief Discovery JSON for Current.
 */
const static String discoveryJsonCurrent = discoverySensor("current", "Current", "current", "measurement", "mA");

/**
 * @brief Discovery JSON for Average Current.
 */
const static String discoveryJsonAvgCurrent = discoverySensor("avg_current", "Average Current", "current", "measurement", "mA");

/**
 * @brief Round mired value to the nearest value that has an enum.
 *
 * @param n mireds
 * @return mired, rounded
 */
constexpr int roundmired(int n) {
  return n + abs((n % 5) - 5);
}

/**
 * @brief This class provides control via MQTT for various settings/options.
 */
class Control {
  public:
    /**
     * @brief Constructor.
     *
     * @param interwebs Pointer to Interwebs object.
     */
    Control(Pxl8* pxl8, MQTT_Looped* interwebs, std::vector<Bottle*>* bottles);

    /**
     * @brief Whether to display pixels.
     */
    bool pixelsOn = true;

    /**
     * @brief Overall brightness, 0-255
     */
    uint8_t brightness = 127;

    /**
     * @brief Current animation.
     */
    bottle_animation_t bottleAnimation = BOTTLE_ANIMATION_DEFAULT;

    /**
     * @brief Glow hue change timeout speed.
     */
    glow_speed_t glowSpeed = GLOW_SPEED_MEDIUM;

    /**
     * @brief Faerie spawn timeout speed.
     */
    faerie_speed_t faerieSpeed = FAERIE_SPEED_MEDIUM;

    /**
     * @brief Global white balance.
     */
    white_balance_t white_balance = 65;

    /**
     * @brief Global color.
     */
    rgb_t static_color = rgb_t{ 255, 255, 255 };

    /**
     * @brief Last reading for Bus Voltage.
     */
    float last_bus_voltage = 0;

    /**
     * @brief Last reading for Shunt Voltage.
     */
    float last_shunt_voltage = 0;

    /**
     * @brief Last reading for Load Voltage.
     */
    float last_load_voltage = 0;

    /**
     * @brief Last reading for Power.
     */
    float last_power = 0;

    /**
     * @brief Last reading for Current.
     */
    float last_current = 0;

    /**
     * @brief Last reading for Average Current.
     */
    float last_avg_current = 0;

    /**
     * @brief Turn on light and check brightness is not zero.
     */
    void turnOn(void);

    /**
     * @brief Turn off light and blank bottle LEDs.
     */
    void turnOff(void);

    /**
     * @brief Send all current MQTT status for settings.
     */
    void mqttCurrentStatus(void);

    /**
     * @brief Send all current MQTT status for sensors.
     */
    void mqttCurrentSensors(void);

    /**
     * @brief Init MQTT control commands. Call before connecting interwebs.
     */
    void initMQTT(void);

    /**
     * @brief Get the Bottle Animation string for MQTT.
     * 
     * @return String 
     */
    String getBottleAnimationString(void);

    /**
     * @brief Get the Glow Speed string for MQTT.
     * 
     * @return String 
     */
    String getGlowSpeedString(void);

    /**
     * @brief Get the Faerie Speed string for MQTT.
     * 
     * @return String 
     */
    String getFaerieSpeedString(void);

    /**
     * @brief Get a random white balance in rgb.
     *
     * @return rgb_t 
     */
    rgb_t getRandomWhiteBalance(void);

    /**
     * @brief Whether it's time for a bottle to change glow hues.
     * 
     * @return bool
     */
    bool shouldChangeGlow(void);

    /**
     * @brief Whether a faerie should be rendered.
     * 
     * @return bool
     */
    bool shouldShowFaerie(void);

    /**
     * @brief Update the hue of a random bottle.
     */
    void updateRandomBottleHue(void);

    /**
     * @brief Update the white balance of a random bottle.
     */
    void updateRandomBottleWhiteBalance(void);

    /**
     * @brief Render faerie animation at current status in current (random) bottle.
     */
    void showFaerie(void);

  private:
    /**
     * @brief Pointer to Pxl8 object.
     */
    Pxl8* pxl8;

    /**
     * @brief Pointer to Interwebs object.
     */
    MQTT_Looped* interwebs;

    /**
     * @brief Pointer to Bottle objects array.
     */
    std::vector<Bottle*>* bottles;

    /**
     * @brief Last time a bottle changed hues.
     */
    uint32_t lastGlowChange;

    /**
     * @brief Whether a faerie is currently spawned.
     */
    bool faerieFlying = false;

    /**
     * @brief Last time a faerie flew.
     */
    uint32_t lastFaerieFly = millis();

    /**
     * @brief Id of bottle a faerie is currently in.
     */
    int8_t faerieBottle = -1;

};

#endif
