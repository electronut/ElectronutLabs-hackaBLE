"""
bluey_beacon.py
This program connects to the hackaBLE Nordic nRF52 dev board 
made by Electronut Labs. The firmware on hackaBLE has to be running the 
"air_quality_monitor" code present in this directory. 
This code is expected to be run on a Raspverry Pi 3.
It assumes that you have bluez installed - it uses hcidump and hcitool.
The code reads advertisement packets from bluey, parses the 
Temperature, Humidity, and dust sensor values 
posts that to adafruit.io dashboard via adafruit.io cloud server
Electronut Labs
electronut.in
References:
1. ElectronutLabs-bluey/code/bluey-beacon/bluey_beacon.py
"""

import re
import sys
import os
import urllib2
import time
import subprocess

import time

# Import Adafruit IO MQTT client.
from Adafruit_IO import MQTTClient


# Set to your Adafruit IO key & username below.
ADAFRUIT_IO_KEY      = 'b9627e9bd96f4178975454873cb67491'
ADAFRUIT_IO_USERNAME = 'Vaishali94'  # See https://accounts.adafruit.com
                                                    # to find your username.


# Define callback functions which will be called when certain events happen.
def connected(client):
    print('Connected to Adafruit IO!  Listening for Dust_sensor changes...')
    # Subscribe to changes on a feed named DemoFeed.
    # client.subscribe('Dust_sensor')

def disconnected(client):
    # Disconnected function will be called when the client disconnects.
    print('Disconnected from Adafruit IO!')
    sys.exit(1)

def message(client, feed_id, payload):
    print('Feed {0} received new value: {1}'.format(feed_id, payload))

# Create an MQTT client instance.
client = MQTTClient(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY)


# Setup the callback functions defined above.
client.on_connect    = connected
client.on_disconnect = disconnected
client.on_message    = message

# Connect to the Adafruit IO server.



"""
This class uses hctool and hcidump to parse BLE adv data.
"""
class BLEScanner:

    hcitool = None
    hcidump = None

    def start(self):
        print('Start receiving broadcasts')
        DEVNULL = subprocess.DEVNULL if sys.version_info > (3, 0) else open(os.devnull, 'wb')

        subprocess.call('sudo hciconfig hci0 reset', shell = True, stdout = DEVNULL)
        self.hcitool = subprocess.Popen(['sudo', '-n', 'hcitool', 'lescan', '--duplicates'], stdout = DEVNULL)
        self.hcidump = subprocess.Popen(['sudo', '-n', 'hcidump', '--raw'], stdout=subprocess.PIPE)

    def stop(self):
        print('Stop receiving broadcasts')
        subprocess.call(['sudo', 'kill', str(self.hcidump.pid), '-s', 'SIGINT'])
        subprocess.call(['sudo', '-n', 'kill', str(self.hcitool.pid), '-s', "SIGINT"])

    def get_lines(self):
        data = None
        try:
            print("reading hcidump...\n")
            #for line in hcidump.stdout:
            while True:
                line = self.hcidump.stdout.readline()
                #print (line)
                line = line.decode()
                if line.startswith('> '):
                    yield data
                    data = line[2:].strip().replace(' ', '')
                elif line.startswith('< '):
                    data = None
                else:
                    if data:
                        data += line.strip().replace(' ', '')
        except KeyboardInterrupt as ex:
            print("kbi")
            return
        except Exception as ex:
            print(ex)
            return


# main() function
def main():
    # use sys.argv if needed
    if len(sys.argv) < 2:
        print('Usage: python bluey-beacon.py MACADDR')
        exit(0)
    print 'starting...'

    deviceId = sys.argv[1]
    scanner = BLEScanner()
    scanner.start()
    client.connect()
    client.loop_background()
    print('Publishing a new message every 10 seconds (press Ctrl-C to quit)...')
    data = None
    while True:
        for line in scanner.get_lines():
            if line:
                found_mac = line[14:][:12]
                reversed_mac = ''.join(
                    reversed([found_mac[i:i + 2] for i in range(0, len(found_mac), 2)]))
                mac = ':'.join(a+b for a,b in zip(reversed_mac[::2], reversed_mac[1::2]))
                #print(mac)
                data = line[26:]
                #print(data)
                if mac == deviceId and len(data) == 56:
                    #print(data)
                    if u'416972514D' in data:
                        data2 = data[24:40]
                        print(data2)
                        x1 = int(data2[0:4], 16)
                        x2 = int(data2[4:8], 16)
                        x3 = int(data2[8:12], 16)
                        x4 = int(data2[12:14], 16)
                        x5 = int(data2[14:16], 16)
                        if x1 <= 1000 and x2<=1000 and x3<=1000:
                          print("%x %x %x\n" % (x1, x2, x3))
                          #value = random.randint(0, 100)
                          #time.sleep(10)
                          #T, H, L = decodeData(x1, x2, x3, x4)
                          print('Publishing {0} to AIR QUALITY MONITOR.'.format(x1))
                          print('Publishing {0} to AIR QUALITY MONITOR.'.format(x2))
                          print('Publishing {0} to AIR QUALITY MONITOR.'.format(x3))
                          print('Publishing {0} to AIR QUALITY MONITOR.'.format(x4))
                          print('Publishing {0} to AIR QUALITY MONITOR.'.format(x5))
                          client.publish('pm0-1', x1)
                          client.publish('pm2-5', x2)
                          client.publish('pm10', x3)
                          client.publish('temperature', x4)
                          client.publish('humidity', x5)
                          time.sleep(20)
                          
        scanner.stop()
        exit(0)

# call main
if __name__ == '__main__':
    main()