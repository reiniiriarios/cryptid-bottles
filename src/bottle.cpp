#include "bottle.h"

Bottle::Bottle(Pxl8 *pxl8, uint8_t pin, uint16_t length, uint16_t hueStart, uint16_t hueEnd)
  : pxl8(pxl8), pin(pin), length(length) {
  setHue(hueStart, hueEnd);
}

void Bottle::setHue(uint16_t start, uint16_t end) {
  start = normalizeHue(start);
  end = normalizeHue(end);
  if (start > end) {
    uint16_t t = start;
    start = end;
    end = t;
  }
  hueRange = { start, end };
  endHueRange = { start, end };
}

void Bottle::setHue(uint16_t start, uint16_t end, uint32_t ms) {
  hueFadeStartTime = millis();
  start = normalizeHue(start);
  end = normalizeHue(end);
  if (start > end) {
    uint16_t t = start;
    start = end;
    end = t;
  }
  startHueRange = hueRange;
  endHueRange = { start, end };
}

void Bottle::updateHue() {
  if (hueRange.first != endHueRange.first || hueRange.second != endHueRange.second) {
    float percent = (millis() - hueFadeStartTime) / hueFadeSpeed;
    hueRange.first = normalizeHue(
      startHueRange.first + ((endHueRange.first - startHueRange.first) * percent)
    );
    hueRange.second = normalizeHue(
      startHueRange.second + ((endHueRange.second - startHueRange.second) * percent)
    );
  }
}

void Bottle::glow(float glowFrequency, float colorFrequency, waveshape_t waveShape) {
  updateHue();
  // amplitude of hue sine wave
  float hueAmp = (hueRange.second - hueRange.first) / 2;
  // animation step
  long t = micros();
  for (uint16_t pixel = 0; pixel < length; pixel++) {
    // adjust time for each to phase out of sync
    long tp = t + pixel * 200;

    float a, h, l;
    switch (waveShape) {
      // hueStart < h < hueEnd
      // 66 < l < 100
      case SAWTOOTH:
        // amplitude * (2 * (time % (1 / freq)) * freq - 1) + amplitude
        a = 2 * fmod(tp, 1 / colorFrequency) * colorFrequency - 1;
        h = hueAmp * a + hueRange.second - hueAmp;
        l = 33 * a + 50;
        break;
      case SINE:
      default:
        // amplitude * sin(time * 2 * PI * freq) + amplitude
        a = sin(tp * 2 * PI * colorFrequency);
        h = hueAmp * a + hueRange.second - hueAmp;
        l = 33 * a + 50;
        break;
    }

    rgb_t rgb = hsl2rgb(hsl_t{ h, 100.f, l });
    setPixelColor(pixel, rgb.r, rgb.g, rgb.b);
  }
}

bool Bottle::showFaerie(float speed, rgb_t color) {
  if (faerieAnimationStart == 0) {
    faerieAnimationStart = millis();
  }
  uint32_t frame = millis() - faerieAnimationStart;

  // Zoom over and fade in to a pixel.
  uint32_t keyframe1 = 1000 / speed;
  // Wait.
  uint32_t keyframe2 = 800 / speed + keyframe1;
  // Zoom over to another pixel.
  uint32_t keyframe3 = 700 / speed + keyframe2;
  // Wait.
  uint32_t keyframe4 = 600 / speed + keyframe3;
  // Zoom away and fade out.
  uint32_t keyframe5 = 1000 / speed + keyframe4;

  if (frame <= keyframe1) {
    float percent = (float)frame / (float)keyframe1 * 100.f;
    faerieFly(color, 0, length / 2, 0, 100, percent);
  }
  else if (frame <= keyframe2) {
    float percent = (float)frame / (float)keyframe2 * 100.f;
    faerieStop(color, length / 2, false, percent);
  }
  else if (frame <= keyframe3) {
    float percent = (float)frame / (float)keyframe3 * 100.f;
    faerieFly(color, length / 2, length, 100, 100, percent);
  }
  else if (frame <= keyframe4) {
    float percent = (float)frame / (float)keyframe4 * 100.f;
    faerieStop(color, length, false, percent);
  }
  else if (frame <= keyframe5) {
    float percent = (float)frame / (float)keyframe5 * 100.f;
    faerieFly(color, length, 0, 100, 0, percent);
  }
  else {
    faerieAnimationStart = 0;
    return false;
  }
  return true;
}

void Bottle::faerieFly(rgb_t color, uint16_t startPos, uint16_t endPos, uint8_t startBright, uint8_t endBright, float percent) {
  // limit 0-100
  endBright = normalizeSL(endBright);
  startBright = normalizeSL(startBright);
  // scale rgb by brightness from 0-color.*
  float a = startBright + ((endBright - startBright) * percent / 100);
  uint8_t r = normalizeRGB(a * color.r);
  uint8_t g = normalizeRGB(a * color.g);
  uint8_t b = normalizeRGB(a * color.b);
  // faerie
  uint16_t pos = startPos + ((endPos - startPos) / percent);
  setPixelColor(pos, r, g, b);
  if (pos > 0) {
    // light trail
    uint8_t r2 = r * 0.5;
    uint8_t g2 = g * 0.5;
    uint8_t b2 = b * 0.5;
    setPixelColor(pos - 1, r2, g2, b2);
    if (pos > 1) {
      // softer light trail
      uint8_t r3 = r * 0.25;
      uint8_t g3 = g * 0.25;
      uint8_t b3 = b * 0.25;
      setPixelColor(pos - 2, r3, g3, b3);
    }
  }
}

void Bottle::faerieStop(rgb_t color, uint16_t pos, bool reverse, float percent) {
  // faerie
  setPixelColor(pos, color.r, color.g, color.b);
  // one pixel behind
  uint16_t pos2 = pos;
  if (reverse) pos2 += 1;
  else pos2 -= 1;
  if (percent < 30 && pos2 <= length && pos2 >= 0) {
    float a1 = 0.5 * (30 - percent);
    uint8_t r2 = color.r * a1;
    uint8_t g2 = color.g * a1;
    uint8_t b2 = color.b * a1;
    setPixelColor(pos2, r2, g2, b2);
    // two pixels behind
    uint16_t pos3 = pos2;
    if (reverse) pos3 += 1;
    else pos3 -= 1;
    if (percent < 15 && pos3 <= length && pos3 >= 0) {
      float a2 = 0.25 * (15 - percent);
      uint8_t r3 = color.r * a2;
      uint8_t g3 = color.g * a2;
      uint8_t b3 = color.b * a2;
      setPixelColor(pos3, r3, g3, b3);
    }
  }
}

void Bottle::rain(void) {
  for (uint16_t pixel = 0; pixel < length; pixel++) {
    uint16_t v = 256 - ((millis() / 4 - pin * 32 + pixel * 256 / length) & 0xFF);
    setPixelColor(pixel, v * 2 >> 8, v * 160 >> 8, v * 255 >> 8);
  }
}

void Bottle::rainbow(void) {
  uint8_t t = millis(); // overflow loop
  for (uint16_t p = 0; p < length; p++) {
    uint16_t hue = p * 256 / length + t;
    rgb_t c = hsl2rgb(hsl_t{ hue, 100U, 100U });
    setPixelColor(p, c.r, c.g, c.b);
  }
}

void Bottle::warning() {
  warning(255, 0, 0);
}

void Bottle::warningWiFi() {
  warning(0, 0, 255);
}

void Bottle::warningMQTT() {
  warning(255, 127, 0);
}

void Bottle::warning(uint8_t r, uint8_t g, uint8_t b) {
  float br = sin(millis() / 2 * PI * 0.001);
  float r2 = r / 2;
  float g2 = g / 2;
  float b2 = b / 2;
  uint8_t rs = r2 * br + r2;
  uint8_t gs = g2 * br + g2;
  uint8_t bs = b2 * br + b2;
  for (uint16_t pixel = 0; pixel < length; pixel++) {
    setPixelColor(pixel, normalizeRGB(rs), normalizeRGB(gs), normalizeRGB(bs));
  }
}

void Bottle::testBlink(void) {
  uint32_t c = pxl8->color(0, 0, 0);
  if (millis() / 500 & 1) {
    c = pxl8->color(255, 255, 255);
  }
  for (uint16_t pixel = 0; pixel < length; pixel++) {
    setPixelColor(pixel, c);
  }
}

void Bottle::setPixelColor(uint16_t pixel, uint32_t color) {
  pxl8->setPixelColor(pin, length, pixel, color);
}

void Bottle::setPixelColor(uint16_t pixel, uint8_t r, uint8_t g, uint8_t b) {
  pxl8->setPixelColor(pin, length, pixel, r, g, b);
}
