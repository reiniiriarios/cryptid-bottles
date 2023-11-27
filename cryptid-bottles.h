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
