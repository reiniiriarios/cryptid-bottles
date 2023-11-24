#ifndef CRYPTID_PXL8_H
#define CRYPTID_PXL8_H

#include <Adafruit_NeoPXL8.h>

// Length of longest strand.
// Memory usage determined by longest strang length * number of strands.
#define LONGEST_STRAND_LENGTH 50

// Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
#define NEOPIXEL_FORMAT NEO_GRB

// @see docs/neopxl8-m4.md
// 13, 12, 11 are unavailable
#define PINS PIN_SERIAL1_RX, PIN_SERIAL1_TX, 9, 6, 10

// Number of pins wired.
#define NUM_PINS 5

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
     * @brief Set brightness.
     * 
     * @param b 
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

    static uint32_t colorHSV(uint16_t hue, uint8_t sat = 255, uint8_t val = 255) {
      return Adafruit_NeoPixel::ColorHSV(hue, sat, val);
    }

    /**
     * @brief Set a pixel a specific color.
     * 
     * @param pin Pin (strand).
     * @param strand_length Number of pixels on strand.
     * @param pixel Number of pixel on strand (zero-indexed).
     * @param color Packed color.
     */
    void setPixelColor(uint8_t pin, uint8_t strand_length, uint16_t pixel, uint32_t color);

    /**
     * @brief Set a pixel a specific RGB (0-255) color.
     * 
     * @param pin Pin (strand).
     * @param strand_length Number of pixels on strand.
     * @param pixel Number of pixel on strand (zero-indexed).
     * @param r Red
     * @param g Green
     * @param b Blue
     */
    void setPixelColor(uint8_t pin, uint8_t strand_length, uint16_t pixel, uint8_t r, uint8_t g, uint8_t b);

    void addStrand(uint16_t length);

  private:
    /**
     * @brief The NeoPXL8 object used to control the pixels.
     */
    Adafruit_NeoPXL8 *neopxl8 = nullptr;

    /**
     * @brief Pinouts for pixel LEDs on board.
     */
    int8_t pins[NUM_PINS] = { PINS };

    /**
     * @brief 
     */
    uint16_t longest_strand = 0;

    uint16_t num_pixels = 0;

    uint16_t num_strands = 0;
};

#endif
