# Blue Melody: Plant Monitor

A moisture-responsive audio player that keeps your plants entertained. When the soil is moist, it plays a sequential playlist of music. When the soil is dry, the audio stops.

## Features
- **Moisture-Responsive Playback:** Uses an analog moisture sensor to toggle audio.
- **Sequential Playlist:** Cycles through all `.wav` files found on the SD card.
- **High-Performance Audio:** Optimized I2S playback (PCM5102) for smooth, high-quality output without CPU stuttering.
- **Dedicated Player Architecture:** Streamlined for audio-only performance to eliminate SPI bus interference.

## Hardware Components
- **Microcontroller:** ESP32-S2 Metro
- **Audio DAC:** PCM5102 I2S DAC Breakout
- **Sensor:** Monk Makes Plant Monitor (Analog output connected to A0)
- **Storage:** MicroSD Card (SPI interface)
- **Speaker:** Generic 3W speaker connected to PCM5102 LOUT/ROUT

## Setup Requirements
- **CircuitPython:** 10.x
- **Audio Files:** Files must be `.wav`, encoded as **16-bit PCM Mono** at **22,050Hz** or **32,000Hz** for optimal stability.
- **Pin Mapping:**
  - **I2S:** BCK=13, WSEL=12, DIN=A5
  - **SD Card:** Standard SPI (SCK, MOSI, MISO) + CS=IO11
  - **Moisture Sensor:** A0

## Git Setup
The project is initialized as a Git repository tracking the core logic and configuration files.
