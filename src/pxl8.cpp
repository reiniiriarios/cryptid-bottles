#include "pxl8.h"

Pxl8::Pxl8(void) {
  neopxl8 = new Adafruit_NeoPXL8(
    (uint16_t)LONGEST_STRAND_LENGTH,
    (int8_t*)PINS,
    (neoPixelType)NEOPIXEL_FORMAT
  );
  neopxl8->begin();
}
