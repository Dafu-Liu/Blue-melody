/*
 * 2026 Plant Monitor - "The Cry for Water"
 * Hardware: ESP32-S2 Metro, PCM5102 I2S DAC, SD Card, ST7735 TFT, Monk Makes Plant Monitor
 * 
 * Updated to User Wiring:
 * TFT: CS=10, DC=9, RST=5, MOSI=MO, SCK=SCK
 * SD: CS=11, MISO=MISO, MOSI=MO, SCK=SCK
 * I2S: BCK=6, DIN=SDA(8), LCK=SCL(9) <-- POTENTIAL CONFLICT WITH DC(9)
 * SENSOR: TX=14, RX=15
 */

#include "Arduino.h"
#include "Audio.h"
#include "SD.h"
#include "SPI.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> 
#include <HardwareSerial.h>

// --- PIN DEFINITIONS (User Wiring) ---

// TFT Display (SPI)
#define TFT_CS    10
#define TFT_DC    9
#define TFT_RST   5

// SD Card (SPI)
#define SD_CS     11

// I2S DAC (PCM5102)
#define I2S_BCLK  6
#define I2S_DOUT  8 // SDA Pin
#define I2S_LRC   9 // SCL Pin (Conflict Warning: Also used by TFT_DC)

// Plant Monitor (UART)
#define SENSOR_TX 14 // Up Arrow
#define SENSOR_RX 15 // Down Arrow

// --- SETTINGS ---
const float WATER_THRESHOLD = 20.0; 
const int NORMAL_VOLUME = 12;
const int DISTORT_VOLUME_MIN = 4;
const int DISTORT_VOLUME_MAX = 21;

// --- GLOBALS ---
Audio audio;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
HardwareSerial PlantSerial(1); 

float currentMoisture = 0.0;
float currentTemp = 0.0;
bool isDistorted = false;
unsigned long lastSensorRead = 0;
unsigned long lastDistortTick = 0;

void updateDisplay(bool happy);

void setup() {
    Serial.begin(115200);
    
    // Initialize TFT (ST7735)
    tft.initR(INITR_BLACKTAB); 
    tft.setRotation(1);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.println("Plant Monitor Init...");

    // Initialize UART for Plant Monitor (Pins 14 & 15)
    PlantSerial.begin(9600, SERIAL_8N1, SENSOR_TX, SENSOR_RX);

    // Initialize SD Card (Uses default SPI pins: SCK, MISO, MOSI)
    if (!SD.begin(SD_CS)) {
        tft.println("SD Fail!");
        Serial.println("SD Card mount failed");
        while(1);
    }
    tft.println("SD Ready.");

    // Initialize Audio
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(NORMAL_VOLUME);
    
    // Search for first MP3
    File root = SD.open("/");
    File file = root.openNextFile();
    while(file && file.isDirectory()) file = root.openNextFile();
    
    if (file) {
        String fileName = "/" + String(file.name());
        audio.connecttoFS(SD, fileName.c_str());
        tft.print("Playing: ");
        tft.println(fileName);
    } else {
        tft.println("No MP3s found!");
    }

    delay(2000);
    updateDisplay(true);
}

void loop() {
    audio.loop();

    // Read Sensor
    if (millis() - lastSensorRead > 2000) {
        lastSensorRead = millis();
        if (PlantSerial.available()) {
            String data = PlantSerial.readStringUntil('\n');
            if (data.startsWith("W")) {
                currentMoisture = data.substring(2, data.indexOf(' ')).toFloat();
                int nextSpace = data.indexOf(' ', 3);
                currentTemp = data.substring(nextSpace + 3, data.indexOf(' ', nextSpace + 1)).toFloat();
                
                bool previouslyDistorted = isDistorted;
                isDistorted = (currentMoisture < WATER_THRESHOLD);
                
                if (previouslyDistorted != isDistorted) {
                    updateDisplay(!isDistorted);
                    if (!isDistorted) {
                        audio.setVolume(NORMAL_VOLUME);
                        audio.setTone(0, 0, 0); 
                    }
                }
            }
        }
    }

    // Distortion Logic
    if (isDistorted) {
        if (millis() - lastDistortTick > 100) {
            lastDistortTick = millis();
            audio.setVolume(random(DISTORT_VOLUME_MIN, DISTORT_VOLUME_MAX));
            audio.setTone(random(-20, 20), random(-20, 20), random(-20, 20));
            if (random(0, 10) > 8) audio.pauseResume();
        }
    }
}

void updateDisplay(bool happy) {
    tft.fillScreen(happy ? ST77XX_GREEN : ST77XX_RED);
    tft.setCursor(10, 30);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_BLACK);
    
    if (happy) {
        tft.println("I'M HAPPY!");
        tft.setCursor(50, 60);
        tft.println(":)");
    } else {
        tft.println("WATER ME!");
        tft.setCursor(50, 60);
        tft.println(":(");
    }
    
    tft.setTextSize(1);
    tft.setCursor(10, 100);
    tft.print("Moisture: ");
    tft.print(currentMoisture);
    tft.println("%");
    tft.setCursor(10, 120);
    tft.print("Temp: ");
    tft.print(currentTemp);
    tft.println("C");
}

void audio_eof_mp3(const char *info){
    // Restart playback
    File root = SD.open("/");
    File file = root.openNextFile();
    if (file) audio.connecttoFS(SD, ("/" + String(file.name())).c_str());
}
