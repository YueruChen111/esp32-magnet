import time

import serial

class Magnet:
    _enable_cmd = "enable\n".encode('ascii')
    _is_enabled_cmd = "is_enabled\n".encode('ascii')
    _disable_cmd = "disable\n".encode('ascii')
    _is_disabled_cmd = "is_disabled\n".encode('ascii')
    _restart_cmd = "restart\n".encode('ascii')
    _status_cmd = "get_status\n".encode('ascii')


    def __init__(self, port: str, baudrate: int = 115200):

        self.ser = serial.Serial(port=port, baudrate=baudrate)

    def restart(self) -> bool:
        self.ser.write(self._restart_cmd)
        return True

    def enable(self) -> bool:
        self.ser.write(self._enable_cmd)
        return True

    def is_enabled(self) -> bool:
        time.sleep(0.01)
        self.read()
        self.ser.write(self._is_enabled_cmd)
        text = self.ser.readline()
        # print(self.ser.readline())
        if text == b"is_enabledMagnet_is_Enabled\n":
            print("Is Enabled")
            return True
        else:
            return None

    def disable(self):
        self.ser.write(self._disable_cmd)
        return True

    def is_disabled(self):
        time.sleep(0.01)
        self.read()
        self.ser.write(self._is_disabled_cmd)
        text = self.ser.readline()
        # print(self.ser.readline())
        if text == b"is_disabledMagnet_is_Disabled\n":
            print("Is Disabled")
            return False
        else:
            return None

    def get_status(self):
        time.sleep(0.01)
        self.read()
        self.ser.write(self._status_cmd)
        text = self.ser.readline()
        # print(self.ser.readline())
        if text == b"get_statusEnabled\n":
            print("Enabled")
            return True
        elif text == b"get_statusDisabled\n":
            print("Disabled")
            return False
        else:
            return None

    def read(self):
        return self.ser.read(self.ser.in_waiting)

    def close(self):
        self.ser.close()