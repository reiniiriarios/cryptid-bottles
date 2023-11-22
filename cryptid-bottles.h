#ifndef CRYPTID_BOTTLES_H
#define CRYPTID_BOTTLES_H

#include <functional>
#include <map>

/**
 * @brief Loop all bottles.
 */
inline void allBottles(std::function<void(int)> f) {
  for (int i = 0; i < 8; ++i) f(i);
}

/**
 * @brief Bottle animations.
 */
typedef enum {
  // Default animation. Selected on startup.
  BOTTLE_ANIMATION_DEFAULT = 0,
  // Faeries!
  BOTTLE_ANIMATION_FAERIES = 1,
  // Rain rain, go away.
  BOTTLE_ANIMATION_RAIN = 2,
  // Rainbow.
  BOTTLE_ANIMATION_RAINBOW = 4,
  // Gentle glow.
  BOTTLE_ANIMATION_GLOW = 3,
  // Test animation.
  BOTTLE_ANIMATION_TEST = 10,
  // Special animation if something goes wrong.
  BOTTLE_ANIMATION_WARNING = 11,
} bottle_animation_t;

/**
 * @brief Map of strings for MQTT commands to bottle animations.
 */
static std::map<String, bottle_animation_t> BOTTLE_ANIMATIONS = {
  { "default", BOTTLE_ANIMATION_DEFAULT },
  { "faeries", BOTTLE_ANIMATION_FAERIES },
  { "rain",    BOTTLE_ANIMATION_RAIN    },
  { "rainbow", BOTTLE_ANIMATION_RAINBOW },
  { "glow",    BOTTLE_ANIMATION_GLOW    },
  { "test",    BOTTLE_ANIMATION_TEST    },
  { "warning", BOTTLE_ANIMATION_WARNING },
};

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
