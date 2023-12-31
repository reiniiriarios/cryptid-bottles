#ifndef CRYPTID_PXL8_H
#define CRYPTID_PXL8_H

#include <Adafruit_NeoPXL8.h>
#include "def.h"

/**
 * @brief Driver for NeoPixels.
 */
class Pxl8 {
  public:
    /**
     * @brief Construct a new Pxl8 object.
     */
    Pxl8(void);

    /**
     * @brief Init pixels.
     *
     * @return Success
     */
    bool init(void);

    /**
     * @brief Cycle through red, green, blue, once.
     */
    void cycle(void);

    /**
     * @brief Render.
     */
    void show(void);

    /**
     * @brief Set brightness. Only call as a config setting, not as part of animation.
     * 
     * @param b 0-255
     */
    void setBrightness(uint8_t b);

    /**
     * @brief Get a pxl8 color for a given RGB (0-255) value.
     * 
     * @param r Red
     * @param g Green
     * @param b Blue
     * @return Packed color.
     */
    static uint32_t color(uint8_t r, uint8_t g, uint8_t b) {
      return Adafruit_NeoPXL8::Color(
        Adafruit_NeoPXL8::gamma8(r),
        Adafruit_NeoPXL8::gamma8(g),
        Adafruit_NeoPXL8::gamma8(b)
      );
    }

    /**
     * @brief Get a pxl8 color for a given HSV color. HSV should be in max bit depth.
     * 
     * @param hue 0-65535
     * @param sat 0-255
     * @param val 0-255
     * @return Packed color.
     */
    static uint32_t colorHSV(uint16_t hue, uint8_t sat = 255, uint8_t val = 255) {
      return Adafruit_NeoPixel::ColorHSV(hue, sat, val);
    }

    /**
     * @brief Set a pixel a specific color.
     * 
     * @param pin Pin (strand).
     * @param pixel Number of pixel on strand (zero-indexed).
     * @param color Packed color.
     */
    void setPixelColor(uint8_t pin, uint16_t pixel, uint32_t color);

    /**
     * @brief Set a pixel a specific RGB (0-255) color.
     * 
     * @param pin Pin (strand).
     * @param pixel Number of pixel on strand (zero-indexed).
     * @param r Red
     * @param g Green
     * @param b Blue
     */
    void setPixelColor(uint8_t pin, uint16_t pixel, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Get a pixel's color in RGB.
     * 
     * @param pin Pin (strand).
     * @param pixel Number of pixel on strand (zero-indexed).
     * @return RGB
     */
    rgb_t getPixelColor(uint8_t pin, uint16_t pixel);

    /**
     * @brief Add strand of LEDs. MUST be called before init().
     * 
     * @param pin Pin (strand).
     * @param length Number of pixels.
     */
    void addStrand(uint8_t pin, uint16_t length);

  private:
    /**
     * @brief The NeoPXL8 object used to control the pixels.
     */
    Adafruit_NeoPXL8 *neopxl8 = nullptr;

    /**
     * @brief Pinouts for pixel LEDs on board.
     */
    int8_t pins[NEOPIXEL_NUM_PINS] = { NEOPIXEL_PINS };

    /**
     * @brief Pixel strands. Limited to 5 pins.
     */
    uint16_t strands[NEOPIXEL_NUM_PINS] = { 0, 0, 0, 0, 0 };

    /**
     * @brief Length of longest strand of pixels.
     */
    uint16_t longest_strand = 0;

    /**
     * @brief Total actual pixels.
     */
    uint16_t num_pixels = 0;

    /**
     * @brief "Total" number of pixels. This number is used to reference pixel ids and is
     *        the longest strand * the number of strands. The total processing power NeoPXL8
     *        will consume will be the longest strand * 8.
     */
    uint16_t num_calc_pixels = 0;

    /**
     * @brief Number of strands added.
     */
    uint16_t num_strands = 0;
};

#endif
