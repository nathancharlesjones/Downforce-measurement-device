# https://learn.sparkfun.com/tutorials/graph-sensor-data-with-python-and-matplotlib/speeding-up-the-plot-animation

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import time
import datetime as dt
import serial
import json

port = serial.Serial("/dev/ttyUSB0", baudrate=460800, timeout=3.0)
now = dt.datetime.now()

def get_voltage(raw):
    return ( raw * 3.3 ) / 4095

# Parameters
x_len = 200         # Number of points to display
y_range = [0, 3.3]  # Range of possible Y values to display

# Create figure for plotting
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = list(range(0, 200))
ys = [0] * x_len
ax.set_ylim(y_range)

# Create a blank line. We will update the line in animate
line, = ax.plot(xs, ys)

# Add labels
plt.title('STM32 Analog Reading')
plt.xlabel('Samples')
plt.ylabel('Volts')

counter = 0

# This function is called periodically from FuncAnimation
def animate(i, ys):
    global counter
    global last_time

    for i in range(1):
        data = port.readline()
        try:
            decoded_line = json.loads(data)
            raw = decoded_line['value']
            volts = get_voltage(raw)
            ys.append(volts)
            ys = ys[-x_len:]
            line.set_ydata(ys)

            with open(r"/media/nathancharlesjones/Storage/Scripts/STM32/F103C8/Downforce-measurement-device/Software/Host/Data/{}_AnalogIn.csv".format(now.strftime("%Y-%m-%d_%H-%M-%S")), "a") as file:
                file.write("{},{:1.1f}\n".format(dt.datetime.now().strftime('%H:%M:%S.%f'),volts))

            counter = counter + 1
        except (json.decoder.JSONDecodeError):
            pass

    time_diff = time.perf_counter() - last_time
    if ( time_diff > 1 ):
        print("{:4d} samples processed in {:1.9f} seconds. Update frequency: {:4.1f} Hz".format(counter,time_diff,(counter/time_diff)))
        print("Bytes unprocessed: {:6d}".format(port.in_waiting))
        last_time = time.perf_counter()
        counter = 0

    return line,

last_time = time.perf_counter()
# Set up plot to call animate() function periodically
ani = animation.FuncAnimation(fig,
    animate,
    fargs=(ys,),
    interval=1,
    blit=True)
plt.show()
