# https://learn.sparkfun.com/tutorials/graph-sensor-data-with-python-and-matplotlib/speeding-up-the-plot-animation

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import serial
import datetime as dt

port = serial.Serial("/dev/ttyACM0", baudrate=4800, timeout=3.0)
now = dt.datetime.now()

# Parameters
x_len = 200         # Number of points to display
y_range = [0, 60]  # Range of possible Y values to display

# Create figure for plotting
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = list(range(0, 200))
ys = [0] * x_len
ax.set_ylim(y_range)

# Create a blank line. We will update the line in animate
line, = ax.plot(xs, ys)

# Add labels
plt.title('GPS Reading')
plt.xlabel('Samples')
plt.ylabel('Seconds')

# This function is called periodically from FuncAnimation
def animate(i, ys):

    while ( port.in_waiting ):
        GPS_data = port.readline()
        GPS_data = GPS_data.decode()
        values = GPS_data.split(',')
        if ( values[0] == '$GPRMC' ):
            GPS_time = float(values[1])
            GPS_seconds = GPS_time%100
            ys.append(GPS_seconds)
            if (values[2] == 'A'):
                with open(r"/media/nathancharlesjones/Storage/Scripts/STM32/F103C8/Downforce-measurement-device/Software/Host/Data/{}_GPS.csv".format(now.strftime("%Y-%m-%d_%H-%M-%S")), "a") as file:
                    file.write("{},{:4.4f},{},{:5.4f},{},{:3.3f}\n".format(dt.datetime.now().strftime('%H:%M:%S.%f'),float(values[3]),values[4],float(values[5]),values[6],float(values[7])))
            else:
                with open(r"/media/nathancharlesjones/Storage/Scripts/STM32/F103C8/Downforce-measurement-device/Software/Host/Data/{}_GPS.csv".format(now.strftime("%Y-%m-%d_%H-%M-%S")), "a") as file:
                    file.write("{},some_latitude,N/S,some_longitude,E/W,some_speed\n".format(dt.datetime.now().strftime('%H:%M:%S.%f')))


    # Limit y list to set number of items
    ys = ys[-x_len:]

    # Update line with new Y values
    line.set_ydata(ys)

    return line,

# Set up plot to call animate() function periodically
ani = animation.FuncAnimation(fig,
    animate,
    fargs=(ys,),
    interval=1000,
    blit=True)
plt.show()
