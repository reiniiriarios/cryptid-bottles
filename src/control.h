#ifndef CRYPTID_CONTROL_H
#define CRYPTID_CONTROL_H

#include <vector>
#include <sstream>
#include "interwebs.h"
#include "bottle.h"

/**
 * @brief Bottle animations.
 */
typedef enum {
  // Default animation. Selected on startup.
  BOTTLE_ANIMATION_DEFAULT = 0,
  // Faeries!
  BOTTLE_ANIMATION_FAERIES = 1,
  // Rain rain, go away.
  BOTTLE_ANIMATION_RAIN = 2,
  // Rainbow.
  BOTTLE_ANIMATION_RAINBOW = 4,
  // Gentle glow.
  BOTTLE_ANIMATION_GLOW = 3,
  // Random white glow.
  BOTTLE_ANIMATION_GLOW_W = 6,
  // Static color.
  BOTTLE_ANIMATION_ILLUM = 5,
  // Test animation.
  BOTTLE_ANIMATION_TEST = 10,
  // Loop through white balance colors.
  BOTTLE_ANIMATION_TEST_WB = 12,
  // Special animation if something goes wrong.
  BOTTLE_ANIMATION_WARNING = 11,
} bottle_animation_t;

/**
 * @brief Map of strings for MQTT commands to bottle animations.
 */
const static std::map<String, bottle_animation_t> BOTTLE_ANIMATIONS = {
  { "Default",    BOTTLE_ANIMATION_DEFAULT },
  { "Faeries",    BOTTLE_ANIMATION_FAERIES },
  { "Rain",       BOTTLE_ANIMATION_RAIN    },
  { "Rainbow",    BOTTLE_ANIMATION_RAINBOW },
  { "Glow",       BOTTLE_ANIMATION_GLOW    },
  { "Glow White", BOTTLE_ANIMATION_GLOW_W  },
  { "Illuminate", BOTTLE_ANIMATION_ILLUM   },
  { "Test",       BOTTLE_ANIMATION_TEST    },
  { "Test White", BOTTLE_ANIMATION_TEST_WB },
  { "Warning",    BOTTLE_ANIMATION_WARNING },
};

/**
 * @brief Map of bottle animations to MQTT string values.
 */
const static std::map<bottle_animation_t, String> BOTTLE_ANIMATIONS_INV = []() -> std::map<bottle_animation_t, String> {
  std::map<bottle_animation_t, String> inv;
  for (auto const& x : BOTTLE_ANIMATIONS) inv[x.second] = x.first;
  return inv;
}();

/**
 * @brief Faerie animation timeout in ms.
 */
typedef enum {
  FAERIE_SPEED_SLOW   = 32000,
  FAERIE_SPEED_MEDIUM = 16000,
  FAERIE_SPEED_FAST   = 6000,
} faerie_speed_t;

/**
 * @brief Faerie animation timeout MQTT values.
 */
const static std::map<String, faerie_speed_t> FAERIE_SPEED = {
  { "Slow",   FAERIE_SPEED_SLOW   },
  { "Medium", FAERIE_SPEED_MEDIUM },
  { "Fast",   FAERIE_SPEED_FAST   },
};

/**
 * @brief Faerie animation timeout MQTT values (INV).
 */
const static std::map<faerie_speed_t, String> FAERIE_SPEED_INV = {
  { FAERIE_SPEED_SLOW,   "Slow"   },
  { FAERIE_SPEED_MEDIUM, "Medium" },
  { FAERIE_SPEED_FAST,   "Fast"   },
};

/**
 * @brief Glow animation timeout in ms.
 */
typedef enum {
  GLOW_SPEED_SLOW   = 30000,
  GLOW_SPEED_MEDIUM = 15000,
  GLOW_SPEED_FAST   = 5000,
} glow_speed_t;

/**
 * @brief Glow animation timeout MQTT values.
 */
const static std::map<String, glow_speed_t> GLOW_SPEED = {
  { "Slow",   GLOW_SPEED_SLOW   },
  { "Medium", GLOW_SPEED_MEDIUM },
  { "Fast",   GLOW_SPEED_FAST   },
};

/**
 * @brief Glow animation timeout MQTT values (INV).
 */
const static std::map<glow_speed_t, String> GLOW_SPEED_INV = {
  { GLOW_SPEED_SLOW,   "Slow"   },
  { GLOW_SPEED_MEDIUM, "Medium" },
  { GLOW_SPEED_FAST,   "Fast"   },
};

// Minimum mired value for white balance.
#define MIN_WB_MIRED 30

// Maximum mired value for white balance.
#define MAX_WB_MIRED 90

/**
 * @brief Color temperature in mireds.
 *
 * @see https://en.wikipedia.org/wiki/Mired
 */
typedef uint8_t white_balance_t;

/**
 * @brief Color temperatures for various whites.
 */
const std::map<white_balance_t, rgb_t> WHITE_TEMPERATURES = {
  { 30, rgb_t{ 255, 164,  87 } },
  { 35, rgb_t{ 255, 177, 111 } },
  { 40, rgb_t{ 255, 190, 135 } },
  { 45, rgb_t{ 255, 203, 159 } },
  { 50, rgb_t{ 255, 216, 183 } },
  { 55, rgb_t{ 255, 227, 207 } },
  { 60, rgb_t{ 255, 241, 231 } },
  { 65, rgb_t{ 255, 255, 255 } },
  { 70, rgb_t{ 246, 255, 255 } },
  { 75, rgb_t{ 232, 252, 255 } },
  { 80, rgb_t{ 218, 248, 255 } },
  { 85, rgb_t{ 204, 244, 255 } },
  { 90, rgb_t{ 190, 240, 255 } }
};

/**
 * @brief Vector of pointers to white colors.
 */
const std::vector<const rgb_t*> WHITE_TEMPERATURES_VECTOR = []() -> std::vector<const rgb_t*> {
  std::vector<const rgb_t*> t;
  for (auto const& x : WHITE_TEMPERATURES) t.push_back(&x.second);
  return t;
}();

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
  String json = R"JSON({
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
    "min_mirs":)JSON" + String(MIN_WB_MIRED) + R"JSON(,
    "max_mirs":)JSON" + String(MAX_WB_MIRED) + R"JSON(,
    "fx_cmd_t":"~/effect/set",
    "fx_list":)JSON" + jsonStr(BOTTLE_ANIMATIONS) + R"JSON(,
    "fx_val_tpl":"{{ value_json.effect }}",
    "dev":{"ids":["cryptidBottles"],"name":"Cryptid Bottles"}})JSON";
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
