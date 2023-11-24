#include "pxl8.h"

Pxl8::Pxl8(void) {
  neopxl8 = new Adafruit_NeoPXL8(
    (uint16_t)LONGEST_STRAND_LENGTH,
    pins,
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
  return true;
}

void Pxl8::cycle(void) {
  // Cycle all pixels red/green/blue. If you see a different
  // sequence, COLOR_ORDER doesn't match your particular NeoPixel type.
  // If you get a jumble of colors, you're using RGBW NeoPixels with an
  // RGB order. Try different COLOR_ORDER values until code and hardware
  // are in harmony.
  for (uint32_t color = 0xFF0000; color > 0; color >>= 8) {
    neopxl8->fill(color);
    neopxl8->show();
    delay(500);
  }
}

void Pxl8::show(void) {
  neopxl8->show();
}

void Pxl8::setBrightness(uint8_t b) {
  neopxl8->setBrightness(b);
}

void Pxl8::setPixelColor(uint8_t pin, uint8_t strand_length, uint16_t pixel, uint32_t color) {
  neopxl8->setPixelColor(pin * strand_length + pixel, color);
}

void Pxl8::setPixelColor(uint8_t pin, uint8_t strand_length, uint16_t pixel, uint8_t r, uint8_t g, uint8_t b) {
  neopxl8->setPixelColor(pin * strand_length + pixel, color(r, g, b));
}
