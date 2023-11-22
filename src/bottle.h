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
};

#endif
