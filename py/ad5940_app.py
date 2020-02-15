import time
from datetime import datetime
from ad5940_app_serial import AD5940AppSerial
from definitions import *
import csv

class AD5940AppController:
    def __init__(self, filename="data.csv"):
        self.filename = filename
        self.ser = AD5940AppSerial()

    def timestamp(self):
        return int(round(time.time() * 1000))

    def run(self):

        self.cmd_switch_app(APP_ID_AMP)
        self.ser.ser.flushOutput()
        self.ser.ser.flushInput()
        while True:
            try:
                with open(self.filename, "a") as fout:
                    writer = csv.writer(fout, delimiter=',')
                    raw_bytes = self.ser.ser.readline()
                    packet = raw_bytes[0:len(raw_bytes)-2].decode("utf-8").split(',')
                    decoded = dict()
                    if packet[0] == str(APP_ID_AMP):
                        decoded = self.parse_amp(packet)
                    elif packet[0] == str(APP_ID_IMP):
                        decoded = self.parse_imp(packet)
                    else:
                        pass
                    if decoded:
                        print(decoded)
                        writer.writerow(list(decoded.values()))
            except KeyboardInterrupt:
                # print("Keyboard Interrupt")
                self.cmd_stop()
                break

    def parse_amp(self, pkt):
        decoded = dict()
        decoded['time'] = self.timestamp()
        decoded['app_id'] = int(pkt[0])
        decoded['index'] = int(pkt[1])
        decoded['i_ua'] = float(pkt[2])
        return decoded

    def parse_imp(self, pkt):
        decoded = dict()
        decoded['time'] = self.timestamp()
        decoded['app_id'] = int(pkt[0])
        decoded['index'] = int(pkt[1])
        decoded['freq'] = float(pkt[2])
        decoded['mag'] = float(pkt[3])
        decoded['phase'] = float(pkt[4])
        return decoded

    def cmd_start(self):
        self.ser.ser.write(b"start\n")

    def cmd_stop(self):
        self.ser.ser.write(b"stop\n")

    def cmd_switch_app(self, app_id):
        self.ser.ser.write(b"switch %i\n" % int(app_id))

    def cmd_help(self):
        self.ser.ser.write("help\n")

if __name__ == '__main__':
    date_suffix = datetime.now().strftime('%y%m%d%H%M%S')
    ctrl = AD5940AppController(f"ampimp_{date_suffix}.csv")
    ctrl.run()
