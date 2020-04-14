import serial
import json
import matplotlib.pyplot as plt

plt.ion()
time = []
value = []
ADC_delay_ms = 100
plot_width_sec = 10
counter = 0

port = serial.Serial("/dev/ttyUSB0", baudrate=460800, timeout=3.0)

while counter < ( plot_width_sec * 1000 / ADC_delay_ms ):
    line = port.readline()
    print(line)
    
    try:
        decoded_line = json.loads(line)
        time.append(decoded_line['time'])
        value.append(decoded_line['value'])
        plt.clf()
        plt.plot(time,value)
        plt.ylim(0, 4096)
        plt.draw()
        plt.pause(0.0000001)
        counter = counter + 1
        print(counter)
    except (json.decoder.JSONDecodeError):
        pass

while True:
    line = port.readline()
    print(line)
    
    try:
        decoded_line = json.loads(line)
        time.append(decoded_line['time'])
        time = time[1:]
        value.append(decoded_line['value'])
        value = value[1:]
        plt.clf()
        plt.plot(time,value)
        plt.ylim(0, 4096)
        plt.draw()
        plt.pause(0.0000001)
    except (json.decoder.JSONDecodeError):
        pass
