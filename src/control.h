#ifndef CRYPTID_CONTROL_H
#define CRYPTID_CONTROL_H

#include <vector>
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
  // Bright white.
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
  { "default",    BOTTLE_ANIMATION_DEFAULT },
  { "faeries",    BOTTLE_ANIMATION_FAERIES },
  { "rain",       BOTTLE_ANIMATION_RAIN    },
  { "rainbow",    BOTTLE_ANIMATION_RAINBOW },
  { "glow",       BOTTLE_ANIMATION_GLOW    },
  { "glow-white", BOTTLE_ANIMATION_GLOW_W  },
  { "illuminate", BOTTLE_ANIMATION_ILLUM   },
  { "test",       BOTTLE_ANIMATION_TEST    },
  { "test-white", BOTTLE_ANIMATION_TEST_WB },
  { "warning",    BOTTLE_ANIMATION_WARNING },
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
  { "slow",   FAERIE_SPEED_SLOW   },
  { "medium", FAERIE_SPEED_MEDIUM },
  { "fast",   FAERIE_SPEED_FAST   },
};

/**
 * @brief Faerie animation timeout MQTT values (INV).
 */
const static std::map<faerie_speed_t, String> FAERIE_SPEED_INV = {
  { FAERIE_SPEED_SLOW,   "slow"   },
  { FAERIE_SPEED_MEDIUM, "medium" },
  { FAERIE_SPEED_FAST,   "fast"   },
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
  { "slow",   GLOW_SPEED_SLOW   },
  { "medium", GLOW_SPEED_MEDIUM },
  { "fast",   GLOW_SPEED_FAST   },
};

/**
 * @brief Glow animation timeout MQTT values (INV).
 */
const static std::map<glow_speed_t, String> GLOW_SPEED_INV = {
  { GLOW_SPEED_SLOW,   "slow"   },
  { GLOW_SPEED_MEDIUM, "medium" },
  { GLOW_SPEED_FAST,   "fast"   },
};

// Minimum mired value for white balance.
#define MIN_WB_MIRED 40

// Maximum mired value for white balance.
#define MAX_WB_MIRED 90

/**
 * @brief Color temperature list. Values in (very) approximate mireds.
 *
 * @see https://en.wikipedia.org/wiki/Mired
 */
typedef enum {
  WB_HOTTEST     = 40,
  WB_HOT         = 45,
  WB_WARMER      = 50,
  WB_WARM        = 55,
  WB_WARM_BRIGHT = 60,
  WB_BRIGHT      = 65,
  WB_COOL_BRIGHT = 70,
  WB_COOL        = 75,
  WB_COOLER      = 80,
  WB_COLD        = 85,
  WB_COLDEST     = 90
} white_balance_t;

/**
 * @brief Color temperatures for various whites.
 */
const std::map<white_balance_t, rgb_t> WHITE_TEMPERATURES = {
  { WB_HOTTEST,     rgb_t{ 255, 200, 150 } },
  { WB_HOT,         rgb_t{ 255, 211, 171 } },
  { WB_WARMER,      rgb_t{ 255, 222, 192 } },
  { WB_WARM,        rgb_t{ 255, 233, 213 } },
  { WB_WARM_BRIGHT, rgb_t{ 255, 244, 234 } },
  { WB_BRIGHT,      rgb_t{ 255, 255, 255 } },
  { WB_COOL_BRIGHT, rgb_t{ 246, 255, 255 } },
  { WB_COOL,        rgb_t{ 232, 252, 255 } },
  { WB_COOLER,      rgb_t{ 218, 248, 255 } },
  { WB_COLD,        rgb_t{ 204, 244, 255 } },
  { WB_COLDEST,     rgb_t{ 190, 240, 255 } }
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
 * @brief Round mired value to the nearest value that has an enum.
 *
 * @param n mireds
 * @return mired, rounded
 */
constexpr int roundmired(int n) {
  return n + abs((n % 5) - 5);
}

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
    white_balance_t white_balance = WB_BRIGHT;

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
