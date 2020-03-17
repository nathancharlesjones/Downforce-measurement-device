import serial
import json
import matplotlib.pyplot as plt

plt.ion()
time = []
value = []

port = serial.Serial("/dev/ttyUSB0", baudrate=115200, timeout=3.0)

while True:
    line = port.readline()
    print(line)
    
    try:
        decoded_line = json.loads(line)
        time.append(decoded_line['time'])
        value.append(decoded_line['value'])
        plt.clf()
        plt.plot(time,value)
        plt.draw()
        plt.pause(0.000001)
    except (json.decoder.JSONDecodeError):
        pass
