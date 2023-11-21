#include "bottle.h"

Bottle::Bottle(Pxl8 *pxl8, uint8_t pin, uint16_t length)
  : pxl8(pxl8), pin(pin), length(length) {}

void Bottle::glow(float glowFrequency, float colorFrequency, uint16_t hueStart, uint16_t hueEnd) {
  // normalize hue values
  hueStart = normalizeHue(hueStart);
  hueEnd = normalizeHue(hueEnd);
  if (hueEnd < hueStart) {
    float t = hueStart;
    hueStart = hueEnd;
    hueEnd = t;
  }
  // amplitude of hue sine wave
  float hueAmp = (hueEnd - hueStart) / 2;
  // animation step
  long t = micros();
  for (uint16_t pixel = 0; pixel < length; pixel++) {
    // adjust time for each to phase out of sync
    long tp = t + pixel * 200;

    // amplitude * sin(time * 2 * PI * freq * scale) + amplitude
    float h = hueAmp * sin(tp * 2 * PI * colorFrequency * 0.0001) + hueEnd - hueAmp; // 0 < h < 360 && hueStart < h < hueEnd
    float l = 100 * sin(tp * 2 * PI * glowFrequency * 0.0001) + 100; // 0 < l < 100

    rgb_t rgb = hsl2rgb(hsl_t{ h, 100, l });
    pxl8->setPixelColor(pin, length, pixel, rgb.r, rgb.g, rgb.b);
  }
}

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
