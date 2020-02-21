import time
from datetime import datetime
from ampimp_serial import AmpImpSerial
from definitions import *
import csv

# @todo move serial comms to separate thread
# @todo add live plotting, possibly in own thread
# @todo standardized packet definitions and processing

class AmpImpController:
    def __init__(self, filename="data.csv"):
        self.filename = filename
        self.ser = AmpImpSerial(port='/dev/ttyACM0')
        self.status = 0

    def timestamp(self):
        return time.time()

    def run(self):

        self.ser.ser.flushOutput()
        self.ser.ser.flushInput()

        while True:
            try:
                with open(self.filename, "a") as fout:
                    writer = csv.writer(fout, delimiter=',')
                    if self.status == 0:
                        self.cmd_status()
                        time.sleep(1)
                    raw_bytes = self.ser.ser.readline()
                    packet = raw_bytes.decode("utf-8").split(',')
                    if len(packet) >= 3 and packet[0] == 'F09F91BF':
                        if packet[1] == str(APP_ID_AMP):
                            decoded = self.parse_amp(packet)
                        elif packet[1] == str(APP_ID_IMP):
                            decoded = self.parse_imp(packet)
                        elif packet[1] == "$+@+":
                            decoded = self.parse_status(packet)
                            print(decoded)
                            pass
                        else:
                            pass
                        if decoded:
                            print(decoded)
                            writer.writerow(list(decoded.values()))
            except KeyboardInterrupt:
                self.cmd_stop()
                break

    def parse_amp(self, pkt):
        decoded = dict.fromkeys(app_amp_keys)
        decoded['time'] = self.timestamp()
        decoded['app_id'] = int(pkt[1])
        decoded['index'] = int(pkt[2])
        decoded['i_ua'] = float(pkt[3])
        return decoded

    def parse_imp(self, pkt):
        decoded = dict.fromkeys(app_imp_keys)
        decoded['time'] = self.timestamp()
        decoded['app_id'] = int(pkt[1])
        decoded['index'] = int(pkt[2])
        decoded['freq'] = float(pkt[3])
        decoded['mag'] = float(pkt[4])
        decoded['phase'] = float(pkt[5])
        return decoded

    def parse_status(self, pkt):
        decoded = dict.fromkeys(app_stat_keys)
        decoded['$+@+'] = pkt[1]
        decoded['status'] = int(pkt[2])
        if self.status == 0 and self.status != decoded['status']:
            self.cmd_switch_app(APP_ID_AMP)
            self.status = decoded['status']
        return decoded

    def cmd_start(self):
        self.ser.ser.write(b"start\n")

    def cmd_stop(self):
        self.ser.ser.write(b"stop\n")

    def cmd_switch_app(self, app_id):
        self.ser.ser.write(b"switch %i\n" % int(app_id))

    def cmd_help(self):
        self.ser.ser.write(b"help\n")

    def cmd_status(self):
        self.ser.ser.write(b"status\n")

if __name__ == '__main__':
    date_suffix = datetime.now().strftime('%y%m%d%H%M%S')
    ctrl = AmpImpController(filename=f"ampimp_{date_suffix}.csv")
    ctrl.run()
