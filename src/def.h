#ifndef CRYPTID_DEF_H
#define CRYPTID_DEF_H

#include <functional>
#include <vector>
#include <map>
using namespace std;
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "color.h"

// Max frames per second.
#define MAX_FPS 120

// Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
#define NEOPIXEL_FORMAT NEO_GRB + NEO_KHZ800

// @see docs/neopxl8-m4.md
// 13, 12, 11 are unavailable
#define NEOPIXEL_PINS PIN_SERIAL1_RX, PIN_SERIAL1_TX, 9, 6, 10

// Number of pins wired.
#define NEOPIXEL_NUM_PINS 5

// Note: These pin definitions leave the the ESP32's `GPIO0` pin undefined (-1). If you wish to use
// this pin - solder the pad on the bottom of the FeatherWing and set `#define ESP32_GPIO0` to the
// correct pin for your microcontroller.
// @see https://learn.adafruit.com/adafruit-airlift-featherwing-esp32-wifi-co-processor-featherwing/arduino
#define SPIWIFI       SPI  // The SPI port
#define SPIWIFI_SS    13   // Chip select pin
#define ESP32_RESETN  12   // Reset pin
#define SPIWIFI_ACK   11   // a.k.a BUSY or READY pin
#define ESP32_GPIO0   -1

#define MQTT_CLIENT_ID "cryptidBottles"
#define MQTT_USER "cryptid"
#define MQTT_PASS "public"

/**
 * @brief Current status of *waves hands* things.
 */
typedef enum {
  STATUS_UNKNOWN_ERROR = -2,
  STATUS_FATAL = -1,
  STATUS_OK = 0,
  STATUS_WIFI_OFFLINE = 10,
  STATUS_MQTT_OFFLINE = 11,
  STATUS_MQTT_SENDING = 20,
} status_t;

/**
 * @brief Loading callback.
 */
typedef std::function<void(status_t)> loading_callback_t;

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
 * @brief Waveshapes.
 * 
 */
typedef enum {
  SINE = 0,
  SAWTOOTH = 1,
} waveshape_t;

#endif
