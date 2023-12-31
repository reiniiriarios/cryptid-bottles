#ifndef CRYPTID_BOTTLE_H
#define CRYPTID_BOTTLE_H

#include <utility>
#include <vector>
#include "pxl8.h"
#include "def.h"

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
     * @param startPixel First pixel on strand that belongs to this bottle.
     * @param length Number of pixels on strand.
     */
    Bottle(Pxl8 *pxl8, uint8_t pin, uint16_t startPixel, uint16_t length);

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
     * @brief Set the color of the bottle.
     * 
     * @param newColor RGB 
     */
    void setColor(rgb_t newColor);

    /**
     * @brief Set the color of the bottle.
     * 
     * @param newColor RGB
     * @param ms fade time in millis
     */
    void setColor(rgb_t newColor, uint32_t ms);

    /**
     * @brief Glow animation.
     *
     * @param glowFrequency Speed of brightness pulse.
     * @param colorFrequency Speed of hue pulse.
     * @param waveShape Waveshape. SINE will produce a smooth glow while SAWTOOTH will make the bottle sparkly.
     */
    void glow(float glowFrequency = 1.25, float colorFrequency = 1, waveshape_t waveShape = SINE);

    /**
     * @brief Glow a specific color.
     *
     * @param glowFrequency Speed of brightness pulse.
     */
    void glowColor(float glowFrequency = 1.25);

    /**
     * @brief Rain animation.
     */
    void rain(void);

    /**
     * @brief Rainbow animation.
     */
    void rainbow(void);

    /**
     * @brief Blank all pixels.
     */
    void blank(void);

    /**
     * @brief Illuminate bottles a specific color.
     *
     * @param staticColor RGB
     */
    void illuminate(rgb_t staticColor);

    /**
     * @brief Illuminate bottles the current bottle color.
     */
    void illuminate(void);

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
     * @brief Loop a series of colors.
     *
     * @param colors pointer to vector of RGB colors
     */
    void loopColors(const std::vector<const rgb_t*>* colors);

    /**
     * @brief Test blink animation.
     */
    void testBlink(void);

    /**
     * @brief Animate a faerie. Call each loop until it returns false.
     *
     * @return bool: Animation continues, display again next loop.
     */
    bool showFaerie(void);

    /**
     * @brief Spawn a new faerie.
     * 
     * @param speed animation speed multiplier
     * @param c RGB faerie color
     */
    void spawnFaerie(float speed = 1, rgb_t c = { 255, 255, 255 });

  private:
    /**
     * @brief Pointer to the pxl8 object for drawing.
     */
    Pxl8 *pxl8;

    /**
     * @brief Pin index. This is the index, the nth bottle, and does not
     *        directly reference pins on the board.
     */
    uint8_t pin;

    /**
     * @brief First pixel on LED strand that belongs to this bottle.
     */
    uint16_t startPixel = 0;

    /**
     * @brief Last pixel on LED strand that belongs to this bottle.
     */
    uint16_t lastPixel = 0;

    /**
     * @brief Number of pixels on the strand that belong to the bottle.
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
     * @brief Color.
     */
    rgb_t color = { 255, 255, 255 };

    /**
     * @brief Starting color.
     */
    rgb_t startColor = { 255, 255, 255 };

    /**
     * @brief Ending color.
     */
    rgb_t endColor = { 255, 255, 255 };

    /**
     * @brief Color fade time in ms.
     */
    uint32_t colorFadeSpeed = 0;

    /**
     * @brief If fading color, this is the start time.
     */
    uint32_t colorFadeStartTime = 0;

    /**
     * @brief Faerie keyframe timing.
     */
    uint32_t faerieKeyframes[5] = {};

    /**
     * @brief Faerie color.
     */
    rgb_t faerieColor = { 255, 255, 255 };

    /**
     * @brief White color.
     */
    uint32_t white = 0xFFFFFF;

    /**
     * @brief Update hue step between start and end range, if different.
     */
    void updateHue(void);

    /**
     * @brief Update color step between start and end, if different.
     */
    void updateColor(void);

    /**
     * @brief Whether a given pixel is out of bounds for this bottle.
     *
     * @return (bool) pixel is in bottle
     */
    inline bool pixelInBottle(uint16_t pixel);

    /**
     * @brief Fly faerie from one pixel to another.
     *
     * @param startPos pixel on bottle strand
     * @param endPos pixel on bottle strand
     * @param startBright 0-100
     * @param endBright 0-100
     * @param percent 0-100 percent through animation
     */
    void faerieFly(uint16_t startPos, uint16_t endPos, uint8_t startBright, uint8_t endBright, float percent);

    /**
     * @brief Stop flying at a pixel. The light trail will fade out.
     *
     * @param pos pixel on bottle strand
     * @param reverse is the faerie flying backwards
     * @param percent 0-100 percent through animation
     */
    void faerieStop(uint16_t pos, bool reverse, float percent);

    /**
     * @brief Set a pixel a specific color.
     * 
     * @param pixel Number of pixel on strand (zero-indexed).
     * @param c Packed color.
     */
    void setPixelColor(uint16_t pixel, uint32_t c);

    /**
     * @brief Set a pixel a specific color.
     * 
     * @param pixel Number of pixel on strand (zero-indexed).
     * @param r Red
     * @param g Green
     * @param b Blue
     */
    void setPixelColor(uint16_t pixel, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Set a pixel a specific color.
     * 
     * @param pixel Number of pixel on strand (zero-indexed).
     * @param rgb
     */
    void setPixelColor(uint16_t pixel, rgb_t rgb);

    /**
     * @brief Get a pixel's color.
     * 
     * @param pixel Number of pixel on strand (zero-indexed).
     * @return RGB
     */
    rgb_t getPixelColor(uint16_t pixel);
};

#endif
