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

/**
 * @brief Callback to display loading progress.
 */
void loading(status_t status = STATUS_LOADING);

/**
 * @brief Set LEDs for a specific status.
 */
void ledStatus(status_t status);

void setup(void);
void loop(void);

#endif
