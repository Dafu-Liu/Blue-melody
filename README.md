# 2026 Plant Monitor - "The Cry for Water"

Project built for **Metro ESP32-S2** using a capacitive moisture sensor.

## Audio Behavior
- **Wet (>20%)**: Normal high-quality MP3 playback.
- **Dry (<20%)**: "Crying" effect via rapid volume oscillation, EQ randomization, and stuttering.

## Wiring Summary (Your Setup)

### ST7735 TFT & SD Card
| Pin Label | Metro ESP32-S2 Pin | Color |
| :--- | :--- | :--- |
| VCC | 3.3V | Red |
| GND | GND | Black |
| SCK | SCK | Yellow |
| MOSI | MO | Green |
| MISO | MISO | White |
| TFT_CS | Pin 10 | Blue |
| SD_CS | Pin 11 | Grey |
| DC / RS | Pin 9 | Purple |
| RESET | Pin 5 | Brown |

### PCM5102 Audio DAC (I2S)
| Pin Label | Metro ESP32-S2 Pin | Notes |
| :--- | :--- | :--- |
| BCK | Pin 6 | Bit Clock |
| DIN | SDA (GPIO 8) | Data In |
| LCK | SCL (GPIO 9) | **CONFLICT** (Shared with TFT DC) |

> **⚠️ PIN CONFLICT:** Your wiring uses **GPIO 9** for both the Display DC and the Audio LCK. If the screen is blank or audio is silent, move the **LCK** wire to **Pin 12** and update the `#define I2S_LRC` in the code.

### Monk Makes Plant Monitor
| Pin Label | Metro ESP32-S2 Pin | Notes |
| :--- | :--- | :--- |
| Up Arrow (↑) | Pin 14 | TX -> ESP32 RX |
| Down Arrow (↓) | Pin 15 | RX -> ESP32 TX |
| Graph (~) | A0 | Analog Moisture (Alternative) |

## Library Requirements
- **ESP32-audioI2S**
- **Adafruit ST7735 and ST7789 Library**
- **Adafruit GFX**
