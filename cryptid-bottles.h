#ifndef CRYPTID_BOTTLES_H
#define CRYPTID_BOTTLES_H

#include <functional>
#include <map>
#include <vector>
#include <Adafruit_SleepyDog.h>
#include <WiFiNINA.h>
#include <MQTT_Looped.h>
#include "src/def.h"
#include "src/color.h"
#include "src/control.h"
#include "src/pxl8.h"
#include "src/bottle.h"
#include "src/voltage.h"
#include "wifi-config.h"

// Instead of using a timer, these approximations ensure the
// same delay between actions. This means things don't execute
// as precicely on schedule, but the animation is, in theory,
// smoother.
#define every_n_seconds(n, offset) if (loopCounter % (MAX_FPS * n) == offset)
#define every_n_loops(n, offset) if (loopCounter % n == offset)

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
