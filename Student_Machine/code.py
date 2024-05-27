import time
import board
import neopixel
import digitalio
import os
import wifi
import adafruit_connection_manager
import adafruit_requests

# wifi

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

# basic request setup
id = "1"
REQUEST_URL = "https://attention-getter.vercel.app/student"
count = int(requests.get(REQUEST_URL, data=str(id)).text)
print(count)

# LED setup.
pixels = neopixel.NeoPixel(board.NEOPIXEL, 1)

# Motor setup.
motor = digitalio.DigitalInOut(board.A0)
motor.direction = digitalio.Direction.OUTPUT

"""# Button setup.
button = digitalio.DigitalInOut(board.A2)
button.direction = digitalio.Direction.INPUT
button.pull = digitalio.Pull.UP

while True:
    print(button.value)"""

while True:
    with requests.get(REQUEST_URL, data=str(id)) as response:
        if int(response.text) == count:
            pass
        else:
            while True:
                motor.value = True
                pixels.fill((255, 0, 0))
                time.sleep(0.5)
                pixels.fill((0, 255, 0))
                time.sleep(0.5)
                pixels.fill((0, 0, 255))
                time.sleep(0.5)
                pixels.fill((255, 0, 255))
                time.sleep(0.5)

                """if button.value:  # Button is pressed (assuming active low)
                    motor.value = False  # Turn off the motor
                    pixels.fill((0, 0, 0))  # Turn off the LEDs
                    break  # Exit the inner loop"""
        count = int(response.text)

