# DO NOT DOWNLOAD AND USE THIS VERSION OF THE CODE. I HAVE YET TO ADD LCD SUPPORT FOR MULTIPLE DEVICES
# USE code.py FOR 1-TO-1 STUDENT TEACHER CONNECTION.

import rotaryio
import digitalio
import board
import neopixel
import os
import wifi
import adafruit_connection_manager
import adafruit_requests

print()
print("Connecting to WiFi")

#  connect to your SSID
wifi.radio.connect(os.getenv('CIRCUITPY_WIFI_SSID'), os.getenv('CIRCUITPY_WIFI_PASSWORD'))

print("Connected to WiFi")

# Initalize Wifi, Socket Pool, Request Session
pool = adafruit_connection_manager.get_radio_socketpool(wifi.radio)
ssl_context = adafruit_connection_manager.get_radio_ssl_context(wifi.radio)
requests = adafruit_requests.Session(pool, ssl_context)
rssi = wifi.radio.ap_info.rssi

# Request URL

REQUEST_URL = "https://attention-getter.vercel.app/teacher"

# other stuff starts now

pixel = neopixel.NeoPixel(board.NEOPIXEL, 1)

button = digitalio.DigitalInOut(board.A0)
button.direction = digitalio.Direction.INPUT
button.pull = digitalio.Pull.UP

encoder = rotaryio.IncrementalEncoder(board.A2, board.A3)

button_state = None
last_position = None

while True:
    position = encoder.position
    name_pos = (position % 20) + 20 % 20;
    if last_position is None or position != last_position:
        print(name_pos)
    last_position = position

    if not button.value and button_state is None:

        pixel.fill((255, 255, 255))
        button_state = "pressed"

    if button.value and button_state == "pressed":

        pixel.fill((0, 0, 0))
        print("Button pressed.")
        with requests.get(REQUEST_URL, data=str(name_pos)) as response:
            print(f"Response: {response.text}")
        button_state = None
