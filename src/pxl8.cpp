#include "pxl8.h"

Pxl8::Pxl8(void) {
  neopxl8 = new Adafruit_NeoPXL8(
    (uint16_t)LONGEST_STRAND_LENGTH,
    (int8_t*)PINS,
    (neoPixelType)NEOPIXEL_FORMAT
  );
}

bool Pxl8::begin(void) {
  Serial.print("Starting pixels...");
  if (!neopxl8->begin()) {
    Serial.println("fail");
    return false;
  }
  Serial.println("success");

  // @todo remove later
  neopxl8->setBrightness(33);

  return true;
}
