# https://learn.sparkfun.com/tutorials/graph-sensor-data-with-python-and-matplotlib/update-a-graph-in-real-time

import datetime as dt
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import time
import board
from analogio import AnalogIn

lightSensor = AnalogIn(board.G1)

def get_voltage(raw):
    return ( raw * 3.3 ) / 65535

# Create figure for plotting
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = []
ys = []
counter = 0

# This function is called periodically from FuncAnimation
def animate(i, xs, ys):
    global counter
    global last_time

    # Read temperature (Celsius) from TMP102
    raw = lightSensor.value
    volts = get_voltage(raw)

    # Add x and y to lists
    xs.append(dt.datetime.now().strftime('%H:%M:%S.%f'))
    ys.append(volts)

    # Limit x and y lists to 20 items
    xs = xs[-10:]
    ys = ys[-10:]

    # Draw x and y lists
    ax.clear()
    ax.plot(xs, ys)

    # Format plot
    plt.xticks(rotation=45, ha='right')
    plt.subplots_adjust(bottom=0.30)
    plt.title('MCP2221 Analog Reading')
    plt.ylabel('Volts')
    plt.ylim(0, 3.3)

    counter = counter + 1

    time_diff = time.perf_counter() - last_time
    if ( time_diff > 1 ):
        print("{:4d} samples processed in {:1.9f} seconds. Update frequency: {:4.1f} Hz".format(counter,time_diff,(counter/time_diff)))
        last_time = time.perf_counter()
        counter = 0

# Set up plot to call animate() function periodically
last_time = time.perf_counter()
ani = animation.FuncAnimation(fig, animate, fargs=(xs, ys), interval=1)
plt.show()
