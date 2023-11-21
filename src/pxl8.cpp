#include "pxl8.h"

Pxl8::Pxl8(void) {
  neopxl8 = new Adafruit_NeoPXL8(
    (uint16_t)LONGEST_STRAND_LENGTH,
    (int8_t*)(pins),
    (neoPixelType)NEOPIXEL_FORMAT
  );
}

bool Pxl8::begin(void) {
  Serial.print("Starting pixels...");
  if (!neopxl8->begin()) {
    Serial.println("fail");
    return false;
  }
  Serial.println("success");

  // @todo remove later
  neopxl8->setBrightness(33);

  return true;
}

void Pxl8::show(void) {
  neopxl8->show();
}

uint32_t Pxl8::color(uint8_t r, uint8_t g, uint8_t b) {
  return neopxl8->Color(
    neopxl8->gamma8(r),
    neopxl8->gamma8(g),
    neopxl8->gamma8(b)
  );
}

void Pxl8::setPixelColor(uint8_t pin, uint8_t strand_length, uint16_t pixel, uint32_t color) {
  neopxl8->setPixelColor(pin * strand_length + pixel, color);
}

void Pxl8::setPixelColor(uint8_t pin, uint8_t strand_length, uint16_t pixel, uint8_t r, uint8_t g, uint8_t b) {
  neopxl8->setPixelColor(pin * strand_length + pixel, color(r, g, b));
}
