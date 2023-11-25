#include "pxl8.h"

Pxl8::Pxl8(void) {}

void Pxl8::addStrand(uint8_t pin, uint16_t length) {
  if (neopxl8 != nullptr) {
    Serial.println("ERROR: Cannot add strands after init.");
    return;
  }
  if (pin > NUM_PINS) {
    Serial.println("ERROR: Pin out of range.");
    return;
  }
  if (strands[pin] == 0) {
    num_strands++;
  }
  strands[pin] += length;
  if (strands[pin] > longest_strand) {
    longest_strand = strands[pin];
  }
  num_pixels = longest_strand * num_strands;
  Serial.println("Added strand of " + String(length) + ", total: " + String(num_pixels));
}

bool Pxl8::init(void) {
  neopxl8 = new Adafruit_NeoPXL8(longest_strand, pins, (neoPixelType)NEOPIXEL_FORMAT);
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
