#include "bottle.h"

Bottle::Bottle(
  Pxl8 *pxl8,
  uint8_t pin,
  uint16_t startPixel,
  uint16_t length,
  uint16_t hueStart,
  uint16_t hueEnd,
  rgb_t color
) :
  pxl8(pxl8),
  pin(pin),
  startPixel(startPixel),
  length(length),
  color(color),
  lastPixel(startPixel + length - 1)
{
  setHue(hueStart, hueEnd);
  pxl8->addStrand(pin, length);
  Serial.println("Bottle of " + String(length) + " pixels on pin " + String(pin) + " added.");
}

void Bottle::setHue(uint16_t start, uint16_t end) {
  start = normalizeHue(start);
  end = normalizeHue(end);
  hueRange = { start, end };
  endHueRange = { start, end };
}

void Bottle::setHue(uint16_t start, uint16_t end, uint32_t ms) {
  hueFadeStartTime = millis();
  hueFadeSpeed = ms;
  startHueRange = hueRange;
  endHueRange = { normalizeHue(start), normalizeHue(end) };
}

void Bottle::setColor(rgb_t newColor) {
  color = newColor;
  endColor = newColor;
}

void Bottle::setColor(rgb_t newColor, uint32_t ms) {
  colorFadeStartTime = millis();
  colorFadeSpeed = ms;
  startColor = newColor;
  endColor = newColor;
}

void Bottle::updateHue() {
  if (hueRange.first != endHueRange.first || hueRange.second != endHueRange.second) {
    float percent = float(millis() - hueFadeStartTime) / hueFadeSpeed;
    if (percent >= 1) {
      hueRange = endHueRange;
      return;
    }
    float s1 = startHueRange.first;
    float s2 = startHueRange.second;
    float e1 = endHueRange.first;
    float e2 = endHueRange.second;
    // adj looping around zero
    if (s2 < s1) s2 += 360;
    if (e2 < e1) e2 += 360;
    // widdershins
    if (e1 - s1 > 180) {
      s1 += 360;
      s2 += 360;
    } else if (s1 - e1 > 180) {
      e1 += 360;
      e2 += 360;
    }
    hueRange.first = normalizeHue(s1 + (e1 - s1) * percent);
    hueRange.second = normalizeHue(s2 + (e2 - s2) * percent);
  }
}

void Bottle::updateColor() {
  if (color.r != endColor.r || color.g != endColor.g || color.b != endColor.b) {
    float percent = float(millis() - colorFadeStartTime) / colorFadeSpeed;
    if (percent >= 1) {
      color = endColor;
      return;
    }
    color.r = normalizeRGB(startColor.r + float(endColor.r - startColor.r) * percent);
    color.g = normalizeRGB(startColor.g + float(endColor.g - startColor.g) * percent);
    color.b = normalizeRGB(startColor.b + float(endColor.b - startColor.b) * percent);
  }
}

void Bottle::glow(float glowFrequency, float colorFrequency, waveshape_t waveShape) {
  updateHue();
  // animation step
  float t = millis() * 0.0004 * PI;
  // if end range is below start, raise above; normalization will resolve
  float hrSecond = hueRange.second;
  if (hueRange.first > hrSecond) {
    hrSecond += 360;
  }
  // hueRange.first < h < hueRange.second
  float hLower = (hrSecond - hueRange.first) / 2;
  float hUpper = hrSecond - hLower;
  // lightness amplitude adjustments
  // lLower < l < 255
  uint8_t lLower = 86;
  uint8_t lUpper = 255 - lLower;

  for (uint16_t p = startPixel; p <= lastPixel; p++) {
    float h, l;
    switch (waveShape) {
      case SAWTOOTH:
        h = hLower * sin(colorFrequency * t + p * 2000 * colorFrequency) + hUpper;
        l = lLower * (2 * fmod(t * glowFrequency * 0.2 + p, 0.8) * 1.25 - 1) + lUpper;
        break;
      case SINE:
      default:
        h = hLower * sin(colorFrequency * t + p * 2000 * colorFrequency) + hUpper;
        l = lLower * sin(glowFrequency * t + p * 2000 * glowFrequency + pin * 1000) + lUpper;
        break;
    }
    uint32_t c = pxl8->colorHSV(normalizeHue16(h), 255U, l);
    setPixelColor(p, c);
  }
}

void Bottle::glowColor(float glowFrequency) {
  updateColor();
  // sin(frequency * time * PI + pin_adjustment + pixel_adjustment * frequency* fluctuation_amount + lift)
  // pin_adjustment: adjustment per pin to misalign animations
  // pixel_adjustment: adjustment per pixel to misalign pixels
  // 0 < fluctuation_amount < 1 (%)
  // lift = 1 - fluctuation_amount (%)
  float t = glowFrequency * millis() * 0.0004 * PI + pin * 1000;
  float pgf = 2000 * glowFrequency;
  for (uint16_t p = startPixel; p <= lastPixel; p++) {
    float adj = sin(t + p * pgf) * 0.4 + 0.6;
    uint8_t r = max((float)color.r * adj, 255);
    uint8_t g = max((float)color.g * adj, 255);
    uint8_t b = max((float)color.b * adj, 255);
    setPixelColor(p, r, g, b);
  }
}

void Bottle::spawnFaerie(float speed, rgb_t c) {
  faerieColor = c;
  faerieKeyframes[0] = 300 / speed;
  faerieKeyframes[1] = 600 / speed + faerieKeyframes[0];
  faerieKeyframes[2] = 200 / speed + faerieKeyframes[1];
  faerieKeyframes[3] = 400 / speed + faerieKeyframes[2];
  faerieKeyframes[4] = 300 / speed + faerieKeyframes[3];
  faerieAnimationStart = millis();
  Serial.println("+ faerie spawned at " + String(speed) + "x; will fly for " + String(faerieKeyframes[4]) + " ms");
}

bool Bottle::showFaerie() {
  uint32_t frame = millis() - faerieAnimationStart;
  uint16_t midpoint = startPixel + length / 2;

  if (frame <= faerieKeyframes[0]) {
    float percent = (float)frame / faerieKeyframes[0];
    faerieFly(startPixel, midpoint, 0, 100, percent);
  }
  else if (frame <= faerieKeyframes[1]) {
    float percent = (float)frame / faerieKeyframes[1];
    faerieStop(midpoint, false, percent);
  }
  else if (frame <= faerieKeyframes[2]) {
    float percent = (float)frame / faerieKeyframes[2];
    faerieFly(midpoint, lastPixel, 100, 100, percent);
  }
  else if (frame <= faerieKeyframes[3]) {
    float percent = (float)frame / faerieKeyframes[3];
    faerieStop(lastPixel, false, percent);
  }
  else if (frame <= faerieKeyframes[4]) {
    float percent = (float)frame / faerieKeyframes[4];
    faerieFly(lastPixel, startPixel, 100, 0, percent);
  }
  else {
    faerieAnimationStart = 0;
    return false;
  }
  return true;
}

void Bottle::faerieFly(uint16_t startPos, uint16_t endPos, uint8_t startBright, uint8_t endBright, float percent) {
  // faerie
  uint16_t pos = startPos + ((endPos - startPos) * percent);
  // scale rgb by brightness from currentColor -> faerieColor
  float blend = (startBright + ((endBright - startBright) * percent)) / 100;
  setPixelColor(pos, blendRGB(getPixelColor(pos), faerieColor, blend));
  // light trail
  uint16_t pos2 = pos;
  if (startPos > endPos) pos2 += 1;
  else pos2 -= 1;
  if (pixelInBottle(pos2)) {
    setPixelColor(pos2, blendRGB(getPixelColor(pos2), faerieColor, blend * 0.5f));
    // softer light trail
    uint16_t pos3 = pos2;
    if (startPos > endPos) pos3 += 1;
    else pos3 -= 1;
    if (pixelInBottle(pos3)) {
      setPixelColor(pos3, blendRGB(getPixelColor(pos3), faerieColor, blend * 0.25f));
    }
  }
}

void Bottle::faerieStop(uint16_t pos, bool reverse, float percent) {
  // faerie
  setPixelColor(pos, faerieColor.r, faerieColor.g, faerieColor.b);
  // one pixel behind
  uint16_t pos2 = pos;
  if (reverse) pos2 += 1;
  else pos2 -= 1;
  if (percent < 0.3f && pixelInBottle(pos2)) {
    setPixelColor(pos2, blendRGB(getPixelColor(pos2), faerieColor, 0.5f * (0.3f - percent)));
    // two pixels behind
    uint16_t pos3 = pos2;
    if (reverse) pos3 += 1;
    else pos3 -= 1;
    if (percent < 0.15f && pixelInBottle(pos3)) {
      setPixelColor(pos3, blendRGB(getPixelColor(pos3), faerieColor, 0.25f * (0.13f - percent)));
    }
  }
}

void Bottle::rain(void) {
  for (uint16_t p = startPixel; p <= lastPixel; p++) {
    uint16_t v = 256 - ((millis() / 4 - pin * 32 + p * 256 / length) & 0xFF);
    setPixelColor(p, v * 2 >> 8, v * 160 >> 8, v * 255 >> 8);
  }
}

void Bottle::rainbow(void) {
  uint16_t t = millis() * 3;
  for (uint16_t p = startPixel; p <= lastPixel; p++) {
    uint16_t hue = p * 65535 / length + t;
    uint32_t c = pxl8->colorHSV(hue, 255U, 255U);
    // Serial.println(String(c, HEX));
    setPixelColor(p, c);
  }
}

void Bottle::blank(void) {
  for (uint16_t p = startPixel; p <= lastPixel; p++) {
    setPixelColor(p, 0);
  }
}

void Bottle::illuminate(uint32_t staticColor) {
  for (uint16_t p = startPixel; p <= lastPixel; p++) {
    setPixelColor(p, staticColor);
  }
}

void Bottle::illuminate(void) {
  for (uint16_t p = startPixel; p <= lastPixel; p++) {
    setPixelColor(p, color);
  }
}

void Bottle::warning(void) {
  warning(255, 0, 0);
}

void Bottle::warningWiFi(void) {
  warning(0, 0, 255);
}

void Bottle::warningMQTT(void) {
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
  for (uint16_t p = startPixel; p <= lastPixel; p++) {
    setPixelColor(p, normalizeRGB(rs), normalizeRGB(gs), normalizeRGB(bs));
  }
}

void Bottle::testBlink(void) {
  uint32_t c = pxl8->color(0, 0, 0);
  if (millis() / 500 & 1) {
    c = pxl8->color(255, 255, 255);
  }
  for (uint16_t p = startPixel; p <= lastPixel; p++) {
    setPixelColor(p, c);
  }
}

void Bottle::setPixelColor(uint16_t pixel, uint32_t c) {
  pxl8->setPixelColor(pin, pixel, c);
}

void Bottle::setPixelColor(uint16_t pixel, uint8_t r, uint8_t g, uint8_t b) {
  pxl8->setPixelColor(pin, pixel, r, g, b);
}

void Bottle::setPixelColor(uint16_t pixel, rgb_t rgb) {
  pxl8->setPixelColor(pin, pixel, rgb.r, rgb.g, rgb.b);
}

rgb_t Bottle::getPixelColor(uint16_t pixel) {
  return pxl8->getPixelColor(pin, pixel);
}

bool Bottle::pixelInBottle(uint16_t pixel) {
  return pixel >= startPixel && pixel <= lastPixel;
}
