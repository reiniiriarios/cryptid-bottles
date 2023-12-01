#ifndef CRYPTID_BOTTLES_H
#define CRYPTID_BOTTLES_H

#include <functional>
#include <map>
#include <vector>
#include "src/def.h"
#include "src/color.h"
#include "src/control.h"
#include "src/pxl8.h"
#include "src/interwebs.h"
#include "src/bottle.h"

// Instead of using a timer, these approximations ensure the
// same delay between actions. This means things don't execute
// as precicely on schedule, but the animation is, in theory,
// smoother.
#define every_n_seconds(n) if (loopCounter % (MAX_FPS * n) == 0)
#define every_n_loops(n) if (loopCounter % n == 0)

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
void err(uint32_t ledColor = 0xFF0000);

/**
 * @brief Set LEDs for a specific status.
 */
void ledStatus(status_t status);

void setup(void);
void loop(void);

/**
 * @brief Calculate SRAM free.
 *
 * @return int bytes
 * @see https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
 */
int freeMemory(void);

#endif
