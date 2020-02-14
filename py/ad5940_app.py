import time
from datetime import datetime
from ad5940_app_serial import AD5940AppSerial
from definitions import *

class AD5940AppController:
    def __init__(self, filename="data.csv"):
        self.filename = filename
        self.ser = AD5940AppSerial()

    def timestamp():
        return int(time.time())

    def run(self):
        with open(self.filename, "a") as f:
            writer = csv.writer(f, delimiter=",")
        while True:
            try:
                raw_bytes = self.ser.readline()
                packet = raw_bytes[0:len(raw_bytes)-2].decode("utf-8").split(',')
                if int(packet[0]) == APP_ID_AMP:
                    decoded = parse_amp(packet)
                elif int(packet[1]) == APP_ID_IMP:
                    decoded = parse_imp(packet)
                else:
                    pass
                print(decoded)
                writer.writerow(decoded)
            pass

    def parse_amp(pkt):
        decoded = []
        decoded['time'] = self.timestamp()
        decoded['app_id'] = int(pkt[0])
        decoded['index'] = int(pkt[1])
        decoded['i_ua'] = float(pkt[2])
        return decoded

    def parse_imp(pkt):
        decoded = []
        decoded['time'] = self.timestamp()
        decoded['app_id'] = int(pkt[0])
        decoded['index'] = int(pkt[1])
        decoded['freq'] = float(pkt[2])
        decoded['mag'] = float(pkt[3])
        decoded['phase'] = float(pkt[4])
        return decoded

if __name__ == '__main__':
    date_suffix = datetime.now().strftime('%y%m%d%H%M%S')
    ctrl = AD5940AppController(f"ampimp_{date_suffix}.csv")
    ctrl.run()
