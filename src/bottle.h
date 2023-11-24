#ifndef CRYPTID_BOTTLE_H
#define CRYPTID_BOTTLE_H

#include <utility>
using namespace std;

#include "pxl8.h"
#include "color.h"

/**
 * @brief Waveshapes.
 * 
 */
typedef enum {
  SINE = 0,
  SAWTOOTH = 1,
} waveshape_t;

/**
 * @brief A strip of LEDs. In a bottle.
 */
class Bottle {
  public:
    /**
     * @brief Construct a new Bottle object
     *
     * @param pxl8 Pointer to the Pxl8 object.
     * @param pin Pin index (not id on board)
     * @param length Number of pixels on strand.
     * @param startHue Hue range lower bound.
     * @param endHue Hue range upper bound.
     */
    Bottle(Pxl8 *pxl8, uint8_t pin, uint16_t length, uint16_t startHue, uint16_t endHue);

    /**
     * @brief Set the hue range of the bottle in degrees.
     * 
     * @param start
     * @param end 
     */
    void setHue(uint16_t start, uint16_t end);

    /**
     * @brief Set the hue range of the bottle in degrees.
     * 
     * @param start
     * @param end
     * @param ms fade time in millis
     */
    void setHue(uint16_t start, uint16_t end, uint32_t ms);

    /**
     * @brief Glow animation.
     *
     * @param glowFrequency Speed of brightness pulse.
     * @param colorFrequency Speed of hue pulse.
     * @param waveShape Waveshape. SINE will produce a smooth glow while SAWTOOTH will make the bottle sparkly.
     */
    void glow(float glowFrequency = 1.25, float colorFrequency = 1, waveshape_t waveShape = SINE);

    /**
     * @brief Rain animation.
     */
    void rain(void);

    /**
     * @brief Rainbow animation.
     */
    void rainbow(void);

    /**
     * @brief Warning animation.
     */
    void warning(void);

    /**
     * @brief Warning animation for WiFi issue.
     */
    void warningWiFi(void);

    /**
     * @brief Warning animation for MQTT issue.
     */
    void warningMQTT(void);

    /**
     * @brief Warning animation at specific color.
     */
    void warning(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Test blink animation.
     */
    void testBlink(void);

    /**
     * @brief Animate a faerie. Call after other animation for set number of frames.
     *
     * @param speed animation speed multiplier
     * @param color RGB faerie color
     * @return bool: Animation continues, display again next loop.
     */
    bool showFaerie(float speed = 1, rgb_t color = { 255, 255, 255 });

  private:
    /**
     * @brief Pointer to the pxl8 object for drawing.
     */
    Pxl8 *pxl8;

    /**
     * @brief Pin index. This is the index and does not
     *        directly reference pins on the board.
     */
    uint8_t pin;

    /**
     * @brief Number of pixels on the strand.
     */
    uint16_t length;

    /**
     * @brief Millis at start of faerie animation.
     */
    uint32_t faerieAnimationStart = 0;

    /**
     * @brief Hue range.
     */
    std::pair<uint16_t, uint16_t> hueRange = { 0, 30 };

    /**
     * @brief Starting hue range for fade.
     */
    std::pair<uint16_t, uint16_t> startHueRange = { 0, 30 };

    /**
     * @brief Ending hue range for fade.
     */
    std::pair<uint16_t, uint16_t> endHueRange = { 0, 30 };

    /**
     * @brief Hue fade time in ms.
     */
    uint32_t hueFadeSpeed = 0;

    /**
     * @brief If fading hue, this is the start time.
     */
    uint32_t hueFadeStartTime = 0;

    /**
     * @brief Update hue step between start and end range, if different.
     */
    void updateHue();

    /**
     * @brief Fly faerie from one pixel to another.
     *
     * @param color faerie color at max brightness
     * @param startPos pixel on bottle strand
     * @param endPos pixel on bottle strand
     * @param startBright 0-100
     * @param endBright 0-100
     * @param percent 0-100 percent through animation
     */
    void faerieFly(rgb_t color, uint16_t startPos, uint16_t endPos, uint8_t startBright, uint8_t endBright, float percent);

    /**
     * @brief Stop flying at a pixel. The light trail will fade out.
     *
     * @param color faerie color at max brightness
     * @param pos pixel on bottle strand
     * @param reverse is the faerie flying backwards
     * @param percent 0-100 percent through animation
     */
    void faerieStop(rgb_t color, uint16_t pos, bool reverse, float percent);

    /**
     * @brief Set a pixel a specific color.
     * 
     * @param pixel Number of pixel on strand (zero-indexed).
     * @param color Packed color.
     */
    void setPixelColor(uint16_t pixel, uint32_t color);

    /**
     * @brief Set a pixel a specific color.
     * 
     * @param pixel Number of pixel on strand (zero-indexed).
     * @param r Red
     * @param g Green
     * @param b Blue
     */
    void setPixelColor(uint16_t pixel, uint8_t r, uint8_t g, uint8_t b);
};

#endif
