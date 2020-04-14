# https://learn.sparkfun.com/tutorials/graph-sensor-data-with-python-and-matplotlib/speeding-up-the-plot-animation

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import time
import board
from analogio import AnalogIn
import datetime as dt

lightSensor = AnalogIn(board.G1)
now = dt.datetime.now()

def get_voltage(raw):
    return ( raw * 3.3 ) / 65535

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
plt.title('MCP2221 Analog Reading')
plt.xlabel('Samples')
plt.ylabel('Volts')

counter = 0

# This function is called periodically from FuncAnimation
def animate(i, ys):
    global counter
    global last_time

    # Read temperature (Celsius) from TMP102
    raw = lightSensor.value
    volts = get_voltage(raw)

    # Add y to list
    ys.append(volts)

    # Limit y list to set number of items
    ys = ys[-x_len:]

    # Update line with new Y values
    line.set_ydata(ys)

    counter = counter + 1

    time_diff = time.perf_counter() - last_time
    if ( time_diff > 1 ):
        print("{:4d} samples processed in {:1.9f} seconds. Update frequency: {:4.1f} Hz".format(counter,time_diff,(counter/time_diff)))
        last_time = time.perf_counter()
        counter = 0

    with open(r"/media/nathancharlesjones/Storage/Scripts/STM32/F103C8/Downforce-measurement-device/Software/Host/Data/{}_AnalogIn.csv".format(now.strftime("%Y-%m-%d_%H-%M-%S")), "a") as file:
        file.write("{},{:1.1f}\n".format(dt.datetime.now().strftime('%H:%M:%S.%f'),volts))

    return line,

last_time = time.perf_counter()
# Set up plot to call animate() function periodically
ani = animation.FuncAnimation(fig,
    animate,
    fargs=(ys,),
    interval=1,
    blit=True)
plt.show()
