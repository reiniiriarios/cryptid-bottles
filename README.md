# Cryptid Bottles

## Boards

- [Adafruit Feather M4 Express](docs/feather-m4.md)
- [Adafruit AirLift FeatherWing ESP32](docs/airlift-esp32.md)
- [Adafruit NeoPXL8 FeatherWing M4](docs/neopxl8-m4.md)

## Setup

1. Copy `wifi-config.example.h` to `wifi-config.h` and fill in values.
1. Install board [following instructions](https://learn.adafruit.com/adafruit-feather-m4-express-atsamd51/setup).
1. Install required libraries via Arduino IDE.
    1. Adafruit SAMD
    1. Adafruit NeoPXL8 - [see docs](https://learn.adafruit.com/adafruit-neopxl8-featherwing-and-library/neopxl8-arduino-library)
    1. Adafruit NeoPixel
    1. Adafruit ZeroDMA
    1. [Adafruit WiFiNiNA](https://github.com/adafruit/WiFiNINA/archive/master.zip) - _manual install_, forked from arduino, [see docs](https://learn.adafruit.com/adafruit-airlift-featherwing-esp32-wifi-co-processor-featherwing/arduino)
    1. MQTT - there are a few MQTT libraries, this one is just called "MQTT".
1. For VS Code, compile to finish intellisense setup.
    1. `.vscode/c_cpp_properties.json` may update.
1. Configure defines in `cryptid-bottles.h` and `src/pxl8.h` if relevant.
