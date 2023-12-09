#ifndef CRYPTID_VOLTAGE_H
#define CRYPTID_VOLTAGE_H

#include <Wire.h>
#include <Adafruit_INA219.h>
#include "def.h"

class VoltageMonitor : public Adafruit_INA219 {
  public:
    /**
     * @brief Constructor
     */
    VoltageMonitor(uint8_t addr = INA219_ADDRESS);

    /**
     *  @brief  Gets the current value in mA, taking into account the
     *          config settings and current LSB.
     *          Overloaded to calculate running average.
     *  @return the current reading convereted to milliamps
     */
    float getCurrent_mA(void);

    /**
     * @brief Return the average mA since the first measure.
     *
     * @return the average reading in milliamps
     */
    float getCurrentAvg_mA(void);

    /**
     * @brief Return the average mAH since the first measure.
     *
     * @return the average reading in milliamp hours.
     */
    float getCurrentAvg_mAH(void);

    /**
     * @brief Get load voltage as a sum of bus and shunt voltage.
     * 
     * @return float
     */
    float getLoadVoltage(void);

    /**
     * @brief Format SI value to a specific precision and add units.
     * 
     * @param value
     * @param units
     * @param precision decimal places
     * @return formatted string
     */
    String formatSIValue(float value, String units, uint8_t precision);

  private:
    /**
     * @brief Number of times mA has been measured.
     */
    uint32_t mA_measure_count = 0;

    /**
     * @brief Average mA measured.
     */
    float average_mA = 0.0;
};

#endif
