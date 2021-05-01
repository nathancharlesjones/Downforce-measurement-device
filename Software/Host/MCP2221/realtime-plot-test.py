import time, random
import math
from collections import deque

start = time.time()

class RealtimePlot:
    def __init__(self, axes, max_entries = 100):
        self.axis_x = deque(maxlen=max_entries)
        self.axis_y = deque(maxlen=max_entries)
        self.axes = axes
        self.max_entries = max_entries
        
        self.lineplot, = axes.plot([], [], "ro-")
        self.axes.set_autoscaley_on(True)

    def add(self, x, y):
        self.axis_x.append(x)
        self.axis_y.append(y)
        self.lineplot.set_data(self.axis_x, self.axis_y)
        self.axes.set_xlim(self.axis_x[0], self.axis_x[-1] + 1e-15)
        self.axes.relim(); self.axes.autoscale_view() # rescale the y-axis

    def animate(self, figure, callback, interval = 50):
        import matplotlib.animation as animation
        def wrapper(frame_index):
            self.add(*callback(frame_index))
            self.axes.relim(); self.axes.autoscale_view() # rescale the y-axis
            return self.lineplot
        animation.FuncAnimation(figure, wrapper, interval=interval)

def main():
    from matplotlib import pyplot as plt

    fig, axes = plt.subplots()
    display = RealtimePlot(axes)
    display.animate(fig, lambda frame_index: (time.time() - start, 50*math.sin(time.time())+50+random.randint(-10,10)))
    plt.show()

    fig, axes = plt.subplots()
    display = RealtimePlot(axes)

    counter = 0
    last_time = time.perf_counter()

    while True:
        display.add(time.time() - start, random.random() * 100)
        plt.pause(0.001)

        counter += 1
        time_diff = time.perf_counter() - last_time
        if ( time_diff > 1 ):
            print("{:4d} samples processed in {:1.9f} seconds. Update frequency: {:4.1f} Hz".format(counter,time_diff,(counter/time_diff)))
            last_time = time.perf_counter()
            counter = 0

if __name__ == "__main__": main()