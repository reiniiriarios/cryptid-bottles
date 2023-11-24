#ifndef CRYPTID_COLOR_H
#define CRYPTID_COLOR_H

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

/**
 * @brief Normalize hue between 0 and 360. Useful for cases when hue calculations may escape range.
 * 
 * @param hue 
 * @return hue
 */
static inline uint16_t normalizeHue(float hue) {
  while (hue > 360) hue -= 360;
  while (hue < 0) hue += 360;
  return (uint16_t)hue;
}

/**
 * @brief Normalize hue between 0 and 360. Useful for cases when hue calculations may escape range.
 * 
 * @param hue 
 * @return hue
 */
static inline uint16_t normalizeHue(int hue) {
  while (hue > 360) hue -= 360;
  while (hue < 0) hue += 360;
  return (uint16_t)hue;
}

/**
 * @brief Normalize hue between 0 and 65535. Useful for cases when hue calculations may escape range.
 * 
 * @param hue 
 * @return hue
 */
static inline uint16_t normalizeHue16(float hue) {
  while (hue > 360) hue -= 360;
  while (hue < 0) hue += 360;
  return (uint16_t)(hue / 360 * 65535);
}

/**
 * @brief Normalize value between 0 and 100. Useful when saturation or lightness may escape range.
 * 
 * @param value 
 * @return normalized value
 */
static inline uint8_t normalizeSL(float v) {
  if (v > 100) v = 100;
  if (v < 0) v = 0;
  return (uint8_t)v;
}

/**
 * @brief Normalize value between 0 and 100. Useful when saturation or lightness may escape range.
 * 
 * @param value 
 * @return normalized value
 */
static inline uint8_t normalizeSL(int v) {
  if (v > 100) v = 100;
  if (v < 0) v = 0;
  return (uint8_t)v;
}

/**
 * @brief Normalize value between 0 and 100. Useful when saturation or lightness may escape range.
 * 
 * @param value 
 * @return normalized value
 */
static inline uint8_t normalizeSL(long v) {
  if (v > 100) v = 100;
  if (v < 0) v = 0;
  return (uint8_t)v;
}

/**
 * @brief Normalize value between 0 and 100. Useful when saturation or lightness may escape range.
 * 
 * @param value 
 * @return normalized value
 */
static inline uint8_t normalizeSL(uint8_t v) {
  return v & 0x64;
}

/**
 * @brief Normalize value between 0 and 100. Useful when saturation or lightness may escape range.
 * 
 * @param value 
 * @return normalized value
 */
static inline uint8_t normalizeSL8(float v) {
  if (v > 100) v = 100;
  if (v < 0) v = 0;
  return (uint8_t)(v * 2.55);
}

/**
 * @brief Normalize value between 0 and 255. Useful when RGB value may escape range.
 * 
 * @param value 
 * @return normalized value
 */
static inline uint8_t normalizeRGB(float v) {
  if (v > 255) v = 255;
  if (v < 0) v = 0;
  return (uint8_t)v;
}

/**
 * @brief Normalize value between 0 and 255. Useful when RGB value may escape range.
 * 
 * @param value 
 * @return normalized value
 */
static inline uint8_t normalizeRGB(int v) {
  if (v > 255) v = 255;
  if (v < 0) v = 0;
  return (uint8_t)v;
}

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
    : r(normalizeRGB(r)), g(normalizeRGB(g)), b(normalizeRGB(b)) {}
  rgb_t(float r, float g, float b)
    : r(normalizeRGB(r)), g(normalizeRGB(g)), b(normalizeRGB(b)) {}
} rgb_t;

/**
 * @brief HSL normalized between 0-360 (H) and 0-100 (SL).
 */
typedef struct hsl_t {
  uint16_t h;
  uint8_t s;
  uint8_t l;
  hsl_t(uint16_t h, uint8_t s, uint8_t l)
    : h(normalizeHue(h)), s(normalizeSL(s)), l(normalizeSL(l)) {}
  hsl_t(int h, int s, int l)
    : h(normalizeHue(h)), s(normalizeSL(s)), l(normalizeSL(l)) {}
  hsl_t(float h, float s, float l)
    : h(normalizeHue(h)), s(normalizeSL(s)), l(normalizeSL(l)) {}
} hsl_t;

#endif
