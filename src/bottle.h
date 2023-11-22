#ifndef CRYPTID_BOTTLE_H
#define CRYPTID_BOTTLE_H

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
     */
    Bottle(Pxl8 *pxl8, uint8_t pin, uint16_t length);

    /**
     * @brief Glow animation.
     *
     * @param glowFrequency Speed of brightness pulse.
     * @param colorFrequency Speed of hue pulse.
     * @param hueStart Hue lower range in degrees.
     * @param hueEnd Hue upper range in degrees.
     * @param waveShape Waveshape. SINE will produce a smooth glow while SAWTOOTH will make the bottle sparkly.
     */
    void glow(float glowFrequency, float colorFrequency, uint16_t hueStart, uint16_t hueEnd, waveshape_t waveShape = SINE);

    /**
     * @brief Rain animation.
     */
    void rain(void);

    /**
     * @brief Warning animation.
     */
    void warning(void);

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
    bool showFaerie(uint8_t speed = 1, rgb_t color = { 255, 255, 255 });

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
