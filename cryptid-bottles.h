#ifndef CRYPTID_BOTTLES_H
#define CRYPTID_BOTTLES_H

#include <functional>
#include <map>

// !! @see src/pxl8.h for more gfx config !!

// Max frames per second.
#define MAX_FPS 120

// Number of bottles.
#define NUM_BOTTLES 4

const rgb_t prettyWhiteColors[] = {
  rgb_t{ 255, 175,  10 }, // ~2k
  rgb_t{ 255, 195,  35 }, // ~2.25k
  rgb_t{ 255, 210,  50 }, // ~2.5k
  rgb_t{ 255, 225,  70 }, // ~2.75k
  rgb_t{ 255, 245, 150 }, // ~3.5k
  rgb_t{ 255, 255, 160 }, // ~3.75k
  rgb_t{ 255, 255, 175 }, // mid---
  rgb_t{ 255, 255, 206 }, // mid--
  rgb_t{ 255, 255, 225 }, // mid-
  rgb_t{ 255, 255, 255 }, // mid
  rgb_t{ 225, 255, 255 }, // mid+
  rgb_t{ 206, 255, 255 }, // ~5.5k
  rgb_t{ 175, 255, 255 }, // ~6.5k
  rgb_t{ 150, 250, 255 }, // ~7.5k
  rgb_t{ 135, 225, 255 }, // ~8.5k
  rgb_t{ 127, 220, 225 }, // ~9k
  rgb_t{ 125, 215, 225 }  // ~9.5k
};

/**
 * @brief Loop all bottles.
 */
inline void allBottles(std::function<void(int)> f) {
  for (int i = 0; i < NUM_BOTTLES; ++i) f(i);
}

/**
 * @brief Random bottle index.
 * 
 * @return index
 */
static inline uint8_t randBottleId(void) {
  return (uint8_t)random(0, NUM_BOTTLES);
}

/**
 * @brief Whether to change a bottle's glow color.
 */
bool shouldChangeGlow(void);

/**
 * @brief Update bottle hues.
 */
void updateBottleHues(void);

/**
 * @brief Update bottle white balances.
 */
void updateBottleWhiteBalance(void);

/**
 * @brief Whether to render a faerie.
 */
bool shouldShowFaerie(void);

/**
 * @brief Randomly spawn and render faeries.
 */
void spawnFaeries(void);

/**
 * @brief Call if fatal crash.
 */
void err(void);

void setup(void);
void loop(void);

#endif
