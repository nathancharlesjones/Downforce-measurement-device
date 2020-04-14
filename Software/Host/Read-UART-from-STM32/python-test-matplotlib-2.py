import serial
import json
import matplotlib.pyplot as plt
from time import sleep

plt.ion()

while True:
    plt.clf()
    plt.plot([1, 2, 3, 4])
    plt.ylabel('some numbers')
    plt.draw()
    plt.pause(0.1)
