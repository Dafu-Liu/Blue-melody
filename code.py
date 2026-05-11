import board
import busio
import digitalio
import time
import random
import storage
import adafruit_sdcard
import audiocore
import audiobusio
import displayio
import terminalio
from adafruit_display_text import label
from adafruit_st7735r import ST7735R

# Release any existing displays
displayio.release_displays()

# --- PIN SETUP (Matching your wiring summary) ---
spi = busio.SPI(board.SCK, board.MOSI, board.MISO)

# TFT Setup
tft_cs = board.D10
tft_dc = board.D9
tft_rst = board.D5

# SD Card Setup
sd_cs = digitalio.DigitalInOut(board.D11)

# I2S Audio Setup (PCM5102)
# Using your pins: BCK=6, DIN=SDA(8), LCK=SCL(9)
# NOTE: LCK (9) conflicts with TFT_DC (9). 
# If it crashes, move LCK to Pin 12 and change board.D9 below to board.D12
i2s_bclk = board.D6
i2s_lrc = board.D9 
i2s_dout = board.D8

# Plant Monitor (UART)
uart = busio.UART(board.D14, board.D15, baudrate=9600)

# --- INITIALIZE HARDWARE ---

# 1. Display
display_bus = displayio.FourWire(spi, command=tft_dc, chip_select=tft_cs, reset=tft_rst)
display = ST7735R(display_bus, width=128, height=160, rotation=90)

# 2. SD Card
try:
    sdcard = adafruit_sdcard.SDCard(spi, sd_cs)
    vfs = storage.VfsFat(sdcard)
    storage.mount(vfs, "/sd")
    print("SD Card Mounted")
except Exception as e:
    print("SD Card Failed:", e)

# 3. Audio
i2s = audiobusio.I2SOut(bit_clock=i2s_bclk, word_select=i2s_lrc, data=i2s_dout)

# --- UI SETUP ---
splash = displayio.Group()
display.show(splash)

# Background (Green initially)
bg_bitmap = displayio.Bitmap(160, 128, 1)
bg_palette = displayio.Palette(1)
bg_palette[0] = 0x00FF00 
bg_sprite = displayio.TileGrid(bg_bitmap, pixel_shader=bg_palette)
splash.append(bg_sprite)

# Status Label
status_group = displayio.Group(scale=2, x=20, y=60)
status_label = label.Label(terminalio.FONT, text="HAPPY :)", color=0x000000)
status_group.append(status_label)
splash.append(status_group)

# --- HELPER FUNCTIONS ---

def get_moisture():
    if uart.in_waiting:
        try:
            line = uart.readline().decode().strip()
            if line.startswith("W"):
                # Format: W=xx.x T=xx.x H=xx.x
                parts = line.split(" ")
                w_val = float(parts[0].split("=")[1])
                return w_val
        except:
            pass
    return None

# --- MAIN LOOP ---

# Use a WAV file on the SD card named 'music.wav'
# Must be 16-bit Mono or Stereo, 22kHz or 44kHz
try:
    with open("/sd/music.wav", "rb") as f:
        wave = audiocore.WaveFile(f)
        
        while True:
            moisture = get_moisture()
            
            if moisture is not None:
                if moisture < 20.0:  # THIRSTY!
                    bg_palette[0] = 0xFF0000  # Red
                    status_label.text = "WATER ME!"
                    
                    if not i2s.playing:
                        i2s.play(wave, loop=True)
                    
                    # --- GLITCH EFFECT ---
                    # Briefly pause or stutter to sound "sick"
                    if random.random() > 0.7:
                        i2s.pause()
                        time.sleep(random.uniform(0.01, 0.1))
                        i2s.resume()
                else:  # HAPPY!
                    bg_palette[0] = 0x00FF00  # Green
                    status_label.text = "HAPPY :)"
                    if not i2s.playing:
                        i2s.play(wave, loop=True)
                    i2s.resume()
            
            time.sleep(0.1)
except Exception as e:
    print("Audio/Loop Error:", e)
    status_label.text = "FILE ERR"
