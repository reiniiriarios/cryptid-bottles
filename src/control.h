#ifndef CRYPTID_CONTROL_H
#define CRYPTID_CONTROL_H

#include <vector>
#include <sstream>
#include "def.h"
#include "interwebs.h"
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
    Control(Pxl8* pxl8, Interwebs* interwebs, std::vector<Bottle*>* bottles);

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
     * @brief Get the RGB value for the current white balance.
     * 
     * @return RGB
     */
    rgb_t getWhiteBalanceRGB(void);

    /**
     * @brief Send all current MQTT status.
     */
    void mqttCurrentStatus(void);

    /**
     * @brief Init MQTT control commands. Call before connecting interwebs.
     */
    void initMQTT(void);

    /**
     * @brief Send MQTT discovery message.
     *
     * @return success
     */
    bool sendDiscovery();

    /**
     * @brief Send MQTT discovery message for select option.
     * 
     * @tparam T map.first
     * @tparam R map.second
     * @param id unique id
     * @param name device name if different from id
     * @param options map of options (mqtt value as first (string))
     * @return success
     */
    template<typename T>
    bool sendDiscoverySelect(String id, String name, std::map<String, T> options);

  private:
    /**
     * @brief Pointer to Pxl8 object.
     */
    Pxl8* pxl8;

    /**
     * @brief Pointer to Interwebs object.
     */
    Interwebs* interwebs;

    /**
     * @brief Pointer to Bottle objects array.
     */
    std::vector<Bottle*>* bottles;
};

#endif
