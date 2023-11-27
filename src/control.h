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

typedef enum {
  WB_RED         = -5,
  WB_HOT         = -4,
  WB_WARMER      = -3,
  WB_WARM        = -2,
  WB_WARM_BRIGHT = -1,
  WB_BRIGHT      =  0,
  WB_COOL_BRIGHT =  1,
  WB_COOL        =  2,
  WB_COOLER      =  3,
  WB_COLD        =  4,
  WB_BLUE        =  5
} white_balance_t;

/**
 * @brief Color temperatures for various whites.
 */
const std::map<white_balance_t, rgb_t> WHITE_TEMPERATURES = {
  { WB_RED,         rgb_t{ 255, 230, 155 } },
  { WB_HOT,         rgb_t{ 255, 235, 175 } },
  { WB_WARMER,      rgb_t{ 255, 240, 195 } },
  { WB_WARM,        rgb_t{ 255, 245, 215 } },
  { WB_WARM_BRIGHT, rgb_t{ 255, 250, 235 } },
  { WB_BRIGHT,      rgb_t{ 255, 255, 255 } },
  { WB_COOL_BRIGHT, rgb_t{ 235, 250, 255 } },
  { WB_COOL,        rgb_t{ 215, 245, 255 } },
  { WB_COOLER,      rgb_t{ 195, 240, 255 } },
  { WB_COLD,        rgb_t{ 175, 235, 255 } },
  { WB_BLUE,        rgb_t{ 155, 230, 255 } }
};

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
    Control(Pxl8* pxl8, Interwebs* interwebs, std::vector<Bottle>* bottles, uint8_t num_bottles);

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
     * @brief Send all MQTT discovery messages.
     * 
     * @return success
     */
    bool sendDiscoveryAll(void);

    /**
     * @brief Send MQTT discovery message.
     *
     * @param type device type
     * @param name device name
     * @param id device unique id
     * @param addl additional JSON
     * @return success
     */
    bool sendDiscovery(String type, String name, String id, String addl);

    /**
     * @brief Send MQTT discovery message for select option.
     * 
     * @tparam T map.first
     * @tparam R map.second
     * @param id unique id
     * @param options map of options (mqtt value as first (string))
     * @param name device name if different from id
     * @return success
     */
    template<typename T>
    bool sendDiscoverySelect(String id, std::map<String, T> options, String name = "");

    /**
     * @brief Send MQTT discovery message for number option.
     * 
     * @param id unique id
     * @param min minimum value
     * @param max maximum value
     * @param name device name if different from id
     * @return success
     */
    bool sendDiscoveryNumber(String id, uint32_t min, uint32_t max, String name = "");

    /**
     * @brief Send MQTT discovery message for boolean switch.
     * 
     * @param id unique id
     * @param name device name if different from id
     * @return success
     */
    bool sendDiscoverySwitch(String id, String name = "");

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
    std::vector<Bottle>* bottles;

    /**
     * @brief How many bottles there are.
     */
    uint8_t num_bottles;
};

#endif
