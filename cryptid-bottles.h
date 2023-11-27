#ifndef CRYPTID_BOTTLES_H
#define CRYPTID_BOTTLES_H

#include <functional>
#include <map>
#include <vector>
#include "src/color.h"
#include "src/control.h"
#include "src/pxl8.h"
#include "src/interwebs.h"
#include "src/bottle.h"

// !! @see src/pxl8.h for more gfx config !!

// Max frames per second.
#define MAX_FPS 120

/**
 * @brief Get a white RGB value at a random color temperature.
 * 
 * @return RGB
 */
rgb_t randomWhiteBalance(void);

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
