#include "bottle.h"

Bottle::Bottle(Pxl8 *pxl8, uint8_t pin, uint16_t length)
  : pxl8(pxl8), pin(pin), length(length) {}

void Bottle::rain(void) {
  for (uint16_t pixel = 0; pixel < length; pixel++) {
    uint16_t v = 256 - ((millis() / 4 - pin * 32 + pixel * 256 / length) & 0xFF);
    pxl8->setPixelColor(pin, length, pixel, v >> 8, v * 160 >> 8, v * 255 >> 8);
  }
}

void Bottle::warning(void) {
  uint32_t c = 0;
  if ((millis() / 500) & 1) {
    c = pxl8->color(255, 0, 0);
  }
  for (uint16_t pixel = 0; pixel < length; pixel++) {
    pxl8->setPixelColor(pin, length, pixel, c);
  }
}

void Bottle::testBlink(void) {
  uint32_t c = 0;
  if ((millis() / 500) & 1) {
    c = pxl8->color(255, 255, 255);
  }
  for (uint16_t pixel = 0; pixel < length; pixel++) {
    pxl8->setPixelColor(pin, length, pixel, c);
  }
}
