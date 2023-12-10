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
 * @brief Blend one RGB value with another and normalize.
 * 
 * @param current
 * @param target
 * @param amount 0-1
 * @return blended value
 */
static inline uint8_t blendValue(uint8_t current, uint8_t target, float amount) {
  return normalizeRGB((float)current + amount * (target - current));
}

/**
 * @brief Blend one RGB color with another and normalize.
 * 
 * @param current
 * @param target
 * @param amount 0-1
 * @return blended RGB
 */
static inline rgb_t blendRGB(rgb_t current, rgb_t target, float amount) {
  return rgb_t {
    blendValue(current.r, target.r, amount),
    blendValue(current.g, target.g, amount),
    blendValue(current.b, target.b, amount),
  };
}

#endif
