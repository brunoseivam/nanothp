#!/usr/bin/python

import socket
import sys
import serial
import datetime
from signal import signal, SIGINT


if len(sys.argv) != 2:
    print ('Usage: python %s <usbPort>' % (sys.argv[0]))
    sys.exit()
port=sys.argv[1]

def handler(signal_received, frame):
    # Handle any cleanup here
    print('SIGINT or CTRL-C detected. Exiting gracefully')
    sys.exit(0)

if __name__ == '__main__':
    # Tell Python to run the handler() function when SIGINT is received
    signal(SIGINT, handler)

    try:
        with serial.Serial(port,115200) as ser:
            print('Script ready, please reset receiver node')
            while True:
                try: 
                    line = ser.readline().decode('utf-8')
                    if (line == 'Receiver: Ready\r\n'):
                        break
                except UnicodeDecodeError:
                    pass
            filename=str(datetime.datetime.now().strftime('%Y-%m-%d_%H-%M')) + '_log.csv'
            print('Data collection started, writing to %s' % filename)
            with open(filename, 'w+') as f:	
                #data collection        
                while True:
                    # Read a line and convert it from b'xxx\r\n' to xxx
                    line = ser.readline().decode('utf-8')
                    if line.strip():  # If it isn't a blank line
                        #print('got: %s' % line)
                        #write header
                        if 'Address,' in line:
                            f.write('Date, Time, ')
                            f.write(line)
                        #write data
                        else:
                            f.write(str(datetime.datetime.now().strftime('%Y-%m-%d')))
                            f.write(',')
                            f.write(str(datetime.datetime.now().strftime('%H:%M:%S')))
                            f.write(',')
                            f.write(line)
                        f.flush()
    except Exception as e: 
        print(e)