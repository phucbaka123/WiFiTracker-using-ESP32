# ESP32 WiFi Tracker

A pocket WiFi analyzer built with an ESP32, a 16x2 I2C LCD, and four buttons. It shows the signal strength of your own network, scans nearby networks, and counts how crowded each 2.4 GHz channel is, then suggests the best channel for your router (1, 6, or 11).

> Add a photo or GIF of the device here: `docs/demo.jpg`

## Features

- **Screen 1: Live signal.** Shows the connected network name and its RSSI in dBm, updated live.
- **Screen 2: Network browser.** Scroll through every nearby network with Up/Down (name and RSSI).
- **Screen 3: Channel analyzer.** Scroll through channels 1 to 13 and see, for each one:
  - how many networks use it (`Ch6: 9`)
  - a weighted congestion score (`W: 25`) that includes interference from neighboring channels
  - the best channel among 1, 6, and 11 (`Best Channel: 1`)
- **Rescan button.** Press to scan again and recompute everything, so you can walk to another room and measure there.
- Buttons use debounced edge detection: holding a button triggers only one action.

## Parts

| Part | Notes |
|---|---|
| ESP32 dev board | ESP32-WROOM (`esp32dev`) |
| 16x2 LCD with I2C backpack | Address `0x27` |
| 4 tactile push buttons | Mode, Up, Down, Rescan |
| Breadboard and jumper wires | |
| USB data cable | Must be a data cable, not charge-only |

## Wiring

**LCD (I2C)**

| LCD pin | ESP32 pin |
|---|---|
| GND | GND |
| VCC | VIN (5V) |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

**Buttons** (one leg to the GPIO, the opposite leg to GND; internal pull-ups are used, so no resistors are needed)

| Button | ESP32 pin |
|---|---|
| Mode | GPIO 27 |
| Up | GPIO 26 |
| Down | GPIO 25 |
| Rescan | GPIO 33 |

> Add your wiring diagram here: `docs/wiring.png`

## Setup

1. Install [VS Code](https://code.visualstudio.com/) with the [PlatformIO extension](https://platformio.org/).
2. Clone this repo and open the folder in VS Code.
3. Create `include/secrets.h` with your WiFi details (this file is git-ignored, so your password never gets uploaded):

   ```cpp
   #define WIFI_SSID "your_network_name"
   #define WIFI_PASS "your_password"
   ```

   You can copy `include/secrets.example.h` as a starting point.
4. Use the **2.4 GHz** network name. The ESP32 does not support 5 GHz.
5. Connect the ESP32 and click **Upload** in PlatformIO. If it says "Connecting...", hold the BOOT button on the board.

`platformio.ini`:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    marcoschwartz/LiquidCrystal_I2C
```

## Controls

| Button | Screen 1 | Screen 2 | Screen 3 |
|---|---|---|---|
| Mode | Next screen | Next screen | Next screen |
| Up | - | Previous network | Previous channel |
| Down | - | Next network | Next channel |
| Rescan | Rescan | Rescan | Rescan |

## How it works

**RSSI** is the received signal strength in dBm. It is always negative, and closer to 0 is stronger. Every 3 dBm drop halves the power.

| RSSI | Quality |
|---|---|
| -30 to -50 | Excellent |
| -50 to -67 | Good |
| -67 to -75 | Okay |
| -75 to -85 | Weak |
| below -85 | Barely usable |

**Channels.** Each network uses one 2.4 GHz channel, but a channel is about 20 MHz wide while channel numbers are only 5 MHz apart. So neighboring channels overlap, and only channels 1, 6, and 11 do not overlap with each other.

**Weighted congestion score.** For every network found, the code adds weight to its own channel and the three channels on each side:

| Distance in channels | Weight added |
|---|---|
| 0 (same channel) | 4 |
| 1 | 3 |
| 2 | 2 |
| 3 | 1 |

The best channel is whichever of 1, 6, and 11 has the lowest total weight.

**Button handling.** Each button remembers its previous state and only reports a press when it changes from released to pressed, after a 40 ms debounce time. The main loop never blocks with `delay()`, and the LCD is redrawn on a 200 ms timer, so quick taps are not missed.

## Results

> Fill this in with your own measurements.

| Location | RSSI (dBm) |
|---|---|
| Next to router | |
| Living room | |
| Bedroom | |
| Behind a wall | |

**Channel scan at home:** Ch 1: _, Ch 6: _, Ch 11: _

**Before and after:** my router was on channel _ and I changed it to _. RSSI and speed before/after: _

## What I learned

> Write 3 to 5 honest sentences. Examples: bugs you hit and how you fixed them, something that surprised you in the scan, what you would do differently.

## Limitations

- Counts how many networks use a channel, not how much traffic each one carries.
- Only scans 2.4 GHz (the ESP32 does not support 5 GHz).
- The weights are a simple approximation of channel overlap, not a measured spectrum.

## Future work

- Log RSSI and ping latency over time and plot RSSI against latency
- Measure current draw and optimize with deep sleep
- Battery power with a charging circuit and battery level monitor

## Project structure

```
.
├── include/
│   ├── secrets.h          (not in git; your WiFi details)
│   └── secrets.example.h
├── src/
│   └── main.cpp
├── docs/
│   ├── wiring.png
│   └── demo.jpg
├── platformio.ini
└── README.md
```

## License

MIT