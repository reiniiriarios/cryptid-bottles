#ifndef CRYPTID_COLOR_H
#define CRYPTID_COLOR_H

/**
 * @brief RGB normalized between 0 and 255.
 */
typedef struct rgb_t {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  rgb_t(uint8_t r, uint8_t g, uint8_t b)
    : r(r), g(g), b(b) {}
  rgb_t(int r, int g, int b)
    : r(r & 0xFF), g(g & 0xFF), b(b & 0xFF) {}
  rgb_t(float r, float g, float b)
    : r((int)r & 0xFF), g((int)g & 0xFF), b((int)b & 0xFF) {}
} rgb_t;

/**
 * @brief HSL normalized between 0-360 (H) and 0-100 (SL).
 */
typedef struct hsl_t {
  uint16_t h;
  uint8_t s;
  uint8_t l;
  hsl_t(uint16_t h, uint8_t s, uint8_t l)
    : h(normalizeHue(h)), s(s & 0x64), l(l & 0x64) {}
} hsl_t;

/**
 * @brief Convert HSL to RGB.
 * 
 * @param hsl Hue, Saturation, Lightness
 * @return RGB
 */
static rgb_t hsl2rgb(hsl_t hsl) {
  float h = hsl.h / 60.f;
  float s = hsl.s / 100.f;
  float l = hsl.l / 100.f;
  float chroma = (1 - abs(2 * l - 1)) * s;
  float x = chroma * (1 - abs((h - floor(h / 2) * 2) - 1));
  float m = l - chroma / 2;
  float r, g, b;
  if (h < 1) {
    r = chroma + m;
    g = x + m;
    b = m;
  } else if (h < 2) {
    r = x + m;
    g = chroma + m;
    b = m;
  } else if (h < 3) {
    r = m;
    g = chroma + m;
    b = x + m;
  } else if (h < 4) {
    r = m;
    g = x + m;
    b = chroma + m;
  } else if (h < 5) {
    r = x + m;
    g = m;
    b = chroma + m;
  } else if (h < 6) {
    r = chroma + m;
    g = m;
    b = x + m;
  } else {
    r = m;
    g = m;
    b = m;
  }
  r *= 255;
  g *= 255;
  b *= 255;

  return rgb_t { r, g, b };
}

/**
 * @brief Normalize hue between 0 and 360. Useful for cases when hue calculations may escape range.
 * 
 * @param hue 
 * @return hue
 */
static inline uint16_t normalizeHue(uint16_t hue) {
  while (hue > 360) hue -= 360;
  while (hue < 0) hue += 360;
  return hue;
}

#endif
