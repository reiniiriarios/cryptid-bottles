#ifndef CRYPTID_COLOR_H
#define CRYPTID_COLOR_H

#include "color-data.h"

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

/**
 * @brief Convert a color temperature in Kelvin to RGB.
 *        Adapted from 'RGB VALUES FOR HOT OBJECTS' by William T. Bridgman, NASA, 2000
 *        http://www.physics.sfasu.edu/astro/color/blackbodyc.txt
 *        A black body approximation is used where the temperature, T, is given in
 *        Kelvin.  The XYZ values are determined by "integrating" the product of the
 *        wavelength distribution of energy and the XYZ functions for a uniform source.
 *
 * @param k Degrees kelvin.
 */
static inline rgb_t kelvin2rgb(uint16_t k) {
  float xx, yy, zz,
        con = 14390158.988f;

  // loop over wavelength bands
  // integration by trapezoid method
  for (int band = 0; band < KELVIN_VECTORS_SIZE; band++) {
    float weight = 1.f;
    if (band == 0 || band == KELVIN_VECTORS_SIZE - 1) weight = 0.5f;

    float wavelength = 380 + band * 5;

    // generate a black body spectrum
    float dis = (0.000000000000000374183f * (1.f / pow(wavelength, 5.f))) / (exp(con / (wavelength * (float)k)) - 1.f);

    // simple integration over bands
    xx += weight * dis * KELVIN_VECTORS[band].at(0);
    yy += weight * dis * KELVIN_VECTORS[band].at(1);
    zz += weight * dis * KELVIN_VECTORS[band].at(2);
  }

  // re-normalize
  float xxyyzzMax = max(xx, max(yy, zz));
  float x = xx / xxyyzzMax,
        y = yy / xxyyzzMax,
        z = zz / xxyyzzMax;

  // x*, y* are chromaticity coordinates
  // white: 0.3127, 0.3291
  float xr = 0.64f, yr = 0.33f,
        xg = 0.29f, yg = 0.60f,
        xb = 0.15f, yb = 0.06f;
  float zr = 1.f - xr - yr,
        zg = 1.f - xg - yg,
        zb = 1.f - xb - yb;

  // convert to rgb
  float denominator = (xr * yg - xg * yr) * zb +
                      (xb * yr - xr * yb) * zg +
                      (xg * yb - xb * yg) * zr;

  float r = ((x * yg - xg * y) * zb +
             (xg * yb - xb * yg) * z +
             (xb * y - x * yb) * zg) /
            denominator,
        g = ((xr * y - x * yr) * zb +
             (xb * yr - xr * yb) * z +
             (x * yb - xb * y) * zr) /
            denominator,
        b = ((xr * yg - xg * yr) * z +
             (x * yr - xr * y) * zg +
             (xg * y - x * yg) * zr) /
            denominator;

  r = min(max(r, 0), 1);
  g = min(max(g, 0), 1);
  b = min(max(b, 0), 1);

  // adjust gamma
  float rangeMax = max(0.00000000010f,max(r,max(g,b)));
  r = pow(r / rangeMax, 0.8f); // gamma = 0.8
  g = pow(g / rangeMax, 0.8f);
  b = pow(b / rangeMax, 0.8f);

  // adjust to 8-bit
  uint8_t r8 = min(round(r * 255), 255),
          g8 = min(round(g * 255), 255),
          b8 = min(round(b * 255), 255);

  return rgb_t{ r8, g8, b8 };
}

#endif
