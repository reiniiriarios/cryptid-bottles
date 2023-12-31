# Cryptid Bottles

<figure>
  <img src="./docs/wip01.jpg" alt="">
  <figcaption>WIP board and LED strands</figcaption>
</figure> 

## Boards

- [Adafruit Feather M4 Express](docs/feather-m4.md)
- [Adafruit AirLift FeatherWing ESP32](docs/airlift-esp32.md)
- [Adafruit NeoPXL8 FeatherWing M4](docs/neopxl8-m4.md)
- [Adafruit INA219 FeatherWing](docs/ina219.md)

## Setup

1. Copy `wifi-config.example.h` to `wifi-config.h` and fill in values.
1. Install board [following instructions](https://learn.adafruit.com/adafruit-feather-m4-express-atsamd51/setup).
1. Install required libraries (and their dependencies) via Arduino IDE.
   1. Adafruit SAMD
   1. Adafruit SleepyDog
   1. Adafruit NeoPXL8 - [see docs](https://learn.adafruit.com/adafruit-neopxl8-featherwing-and-library/neopxl8-arduino-library)
   1. Adafruit NeoPixel
   1. Adafruit ZeroDMA
   1. [Adafruit WiFiNiNA](https://github.com/adafruit/WiFiNINA/archive/master.zip) - _manual install_, forked from arduino, [see docs](https://learn.adafruit.com/adafruit-airlift-featherwing-esp32-wifi-co-processor-featherwing/arduino)
   1. Adafruit INA219
   1. [MQTT_Looped](https://github.com/reiniiriarios/arduino-mqtt-looped)
1. For VS Code, compile to finish intellisense setup.
   1. `.vscode/c_cpp_properties.json` may update.
1. Configure defines in `cryptid-bottles.h` and `src/pxl8.h` if relevant.

## HW Config

### NeoPXL8 Connections

- Output #0 comes from `RX`  (Available; shared by `ESPRX`)
- Output #1 comes from `TX`  (Available; shared by `ESPTX`)
- Output #2 comes from `D9`  (Available)
- Output #3 comes from `D6`  (Available)
- ~~Output #4 comes from~~ `D13` (Unavailable, used by `ESPCS`!)
- ~~Output #5 comes from~~ `D12` (Unavailable, used by `ESPRST`!)
- ~~Output #6 comes from~~ `D11` (Unavailable, used by `ESPBUSY`!)
- Output #7 comes from `D10` (Available; shared by `ESPGPIO0`)

### Airlift Connections

- `SPIWIFI` from `SPI`
- `SPIWIFI_SS` from `D13`
- `ESP32_RESETN` from `D12`
- `SPIWIFI_ACK` from `D11`
- `ESP32_GPIO0` disabled

### Other Connections

- Pin `D8` is connected to the onboard mini NeoPixel.
- Analog pin `A0` used for random seed.
- Current sensor INA219 connects over I2C at its default address, `0x40`.

## MQTT Control

Bottles can be controlled over MQTT.

- Birth and LWT messages sent on `cryptid/bottles/status` as `online`/`offline`.
- Status messages sent on `cryptid/bottles/state` in JSON.
- Discovery (auto-config) messages published for [Home Assistant](https://www.home-assistant.io/)
  (prefix `homeassistant/`) on startup, reconnection, and Home Assistant birth messages.
- Commands for:

  | Topic           | Payload                                                                                                      |
  | --------------- | ------------------------------------------------------------------------------------------------------------ |
  | `on`            | `ON`/`OFF`                                                                                                   |
  | `brightness`    | `0-255`                                                                                                      |
  | `rgb`           | `0-255,0-255,0-255`, e.g. `0,128,200`                                                                        |
  | `white_balance` | `30-90` in [mireds](https://en.wikipedia.org/wiki/Mired), e.g. `40`, `65`                                    |
  | `white`         | `0-255`                                                                                                      |
  | `effect`        | `Default`, `Glow`, `Glow White`, `Faeries`, `Rain`, `Rainbow`, `Test`, `Test White`, `Illuminate`, `Warning` |
  | `glow_speed`    | `Slow`,`Medium`,`Fast`                                                                                       |
  | `faerie_speed`  | `Slow`,`Medium`,`Fast`                                                                                       |

- See [src/control.cpp](./src/control.cpp) for individual command details.

## Status LEDs 🚥

The two RGB LEDs on both the M4 and ESP32 boards will display:

- 💚 **green**: Bootloader active (M4 only) (double tap Reset to activate)
- 🧡 **orange**: WiFi disconnected, connecting
- 💜 **purple**: MQTT disconnected, connecting
- 💙 **cyan**: Sending MQTT message
- 🛑 **red**: Unknown error

🔌 The large green LED on the board with the INA219 indicates power, behind main capacitors.
