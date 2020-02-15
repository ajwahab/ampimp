# Based on code written by Peter Hinson.
import logging
import time
import serial
from serial.tools import list_ports

_log = logging.getLogger(__name__)
_log.setLevel(logging.DEBUG)

class AmpImpSerial:
    def __init__(self, port='/dev/cu.usbmodem145202', baudrate=230400):
        if not port:
            ports = list(list_ports.grep('usbmodem'))
            if not ports:
                _log.error('Port not found')
            else:
                port = ports[0]

        self.ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS
        )

        if not self.ser.isOpen():
            # may throw an exception if the port isn't ready
            self.ser.open()

    def __del__(self):
        try:
            self.ser.close()
        except:
            pass
