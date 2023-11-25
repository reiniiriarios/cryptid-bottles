#ifndef CRYPTID_BOTTLES_H
#define CRYPTID_BOTTLES_H

#include <functional>
#include <map>

// !! @see src/pxl8.h for more gfx config !!

// Max frames per second.
#define MAX_FPS 120

// Number of bottles.
#define NUM_BOTTLES 4

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
 * @brief Whether to change a bottle's hue.
 */
bool shouldChangeHue(void);

/**
 * @brief Update bottle hues.
 */
void updateBottleHues(void);

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
