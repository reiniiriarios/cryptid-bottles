#ifndef CRYPTID_BOTTLE_H
#define CRYPTID_BOTTLE_H

#include "pxl8.h"

class Bottle {
  public:
    /**
     * @brief Construct a new Bottle object
     *
     * @param pxl8 Pointer to the Pxl8 object.
     * @param pin Pin index (not id on board)
     * @param length Number of pixels on strand.
     */
    Bottle(Pxl8 *pxl8, uint8_t pin, uint16_t length);

  private:
    /**
     * @brief Pointer to the pxl8 object for drawing.
     */
    Pxl8 *pxl8;

    /**
     * @brief Pin index. This is the index and does not
     *        directly reference pins on the board.
     */
    uint8_t pin;

    /**
     * @brief Number of pixels on the strand.
     */
    uint16_t length;
};

#endif
