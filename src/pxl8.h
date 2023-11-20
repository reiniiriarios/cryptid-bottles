#ifndef CRYPTID_PXL8_H
#define CRYPTID_PXL8_H

#include <Adafruit_NeoPXL8.h>

// @see docs/neopxl8-m4.md
// The last 4 can be changed with some cutting/soldering:
// (13, 12, 11, 10, PIN_SERIAL1_RX, PIN_SERIAL1_TX, SCL, SDA)
#define PINS (13, 12, 11, 10, SCK, 5, 9, 6)

// Length of longest strand.
// Memory usage determined by longest strang length * number of strands.
#define LONGEST_STRAND_LENGTH 7

// Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
#define NEOPIXEL_FORMAT NEO_GRB

/**
 * @brief Driver for NeoPixels.
 */
class Pxl8 {
  public:
    Pxl8(void);

  private:
    Adafruit_NeoPXL8 *neopxl8;
};

#endif
