#include "voltage.h"

VoltageMonitor::VoltageMonitor(uint8_t addr) : Adafruit_INA219(addr) {}

float VoltageMonitor::getCurrent_mA(void) {
  float mA = Adafruit_INA219::getCurrent_mA();
  this->average_mA = this->average_mA + (mA - this->average_mA / ++this->mA_measure_count);
  return mA;
}

float VoltageMonitor::getCurrentAvg_mA(void) {
  return this->average_mA;
}

float VoltageMonitor::getCurrentAvg_mAH(void) {
  return this->average_mA / 3600;
}

float VoltageMonitor::getLoadVoltage(void) {
  return this->getBusVoltage_V() + (this->getShuntVoltage_mV() / 1000);
}


String VoltageMonitor::formatSIValue(float value, String units, uint8_t precision) {
  // Add milli prefix if low value.
  if (fabs(value) < 1.0) {
    units = "m" + units;
    value *= 1000.0;
    precision = max(0, precision-3);
  }
  String value_at_precision = String(floor((value * pow(10, precision) + 0.5)) / pow(10, precision));
  return value_at_precision + " " + units;
}
