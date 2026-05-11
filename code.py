import board
import busio
import digitalio
import storage
import adafruit_sdcard
import audiobusio
import audiocore
import os
import time
import random
from analogio import AnalogIn

# 1. SPI for SD Card
spi = busio.SPI(board.SCK, board.MOSI, board.MISO)
sd_cs = digitalio.DigitalInOut(board.IO11)
sdcard = adafruit_sdcard.SDCard(spi, sd_cs)
storage.mount(storage.VfsFat(sdcard), "/sd")

# 2. Audio (BCK=13, WSEL=12, DIN=A5)
i2s = audiobusio.I2SOut(bit_clock=board.IO13, word_select=board.IO12, data=board.A5)

# 3. Moisture Sensor (A0)
sensor = AnalogIn(board.A0)

def is_wet():
    return sensor.value > 10000

# Get list of files
files = [f for f in os.listdir("/sd") if f.lower().endswith(".wav") and not f.startswith("._")]
files.sort()
current_file_idx = 0

print("Starting mode: Constantly playing, AGGRESSIVE crying when dry.")

while True:
    f = open("/sd/" + files[current_file_idx], "rb")
    wav = audiocore.WaveFile(f)
    i2s.play(wav)
    
    while i2s.playing:
        if not is_wet():
            # AGGRESSIVE crying mode:
            # 1. Massive pitch shifts
            # 2. Frequent abrupt pauses/stutters
            wav.sample_rate = random.randint(8000, 44100)
            
            if random.random() > 0.4:
                i2s.pause()
                time.sleep(random.uniform(0.05, 0.2))
                i2s.resume()
                
            time.sleep(random.uniform(0.05, 0.15))
        else:
            # Normal mode
            if wav.sample_rate != 22050:
                wav.sample_rate = 22050
            time.sleep(0.5)
            
    current_file_idx = (current_file_idx + 1) % len(files)
    f.close()
