# https://pythonspot.com/matplotlib-update-plot/

import matplotlib.pyplot as plt
import numpy as np
import time

x = list(np.linspace(0,19,20))
y = list(np.zeros(20, dtype=float))

plt.ion()
fig = plt.figure()
ax = fig.add_subplot(111)
line1, = ax.plot(x, y, 'b-')
plt.ylim(0,1)
counter = 0

while True:
	x.append(counter)
	counter = counter + 1
	x = x[-20:]

	y.append(np.random.rand())
	y = y[-20:]

	line1.set_ydata(y)
	fig.canvas.draw()
	
	# time.sleep(0.001)