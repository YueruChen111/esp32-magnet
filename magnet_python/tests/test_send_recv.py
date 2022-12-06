import time

from esp32_magnet import Magnet

mag = Magnet('COM5', 115200)
mag.enable()
mag.is_enabled()
mag.get_status()
time.sleep(2)
mag.disable()
mag.is_disabled()
mag.get_status()
