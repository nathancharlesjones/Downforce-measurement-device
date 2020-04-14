import time
import board
from analogio import AnalogIn

lightSensor = AnalogIn(board.G1)

def get_voltage(raw):
    return ( raw * 3.3 ) / 65535

#while True:
#   raw = lightSensor.value
#   volts = get_voltage(raw)
#   print("raw = {:5d} volts = {:5.2f}".format(raw, volts))
#   time.sleep(1)

#loop_time = time.perf_counter()
#while True:
#   start_time = time.perf_counter()
#   loop_time_sum = 0
#   loop_count = 0
#   while( ( time.perf_counter() - start_time ) < 1 ):
#       raw = lightSensor.value
#       volts = get_voltage(raw)
#       loop_count = loop_count + 1
#       loop_time_sum = loop_time_sum + ( time.perf_counter() - loop_time )
#       loop_time = time.perf_counter()
#   #rint("loop_time_sum = {:1.9f} loop_count = {:4d}".format(loop_time_sum, loop_count))
#   average_loop_time = loop_time_sum / loop_count
#   average_loop_freq = 1 / average_loop_time
#   print("Average loop time: {:1.9f} Average loop freq: {:6f}".format(average_loop_time, average_loop_freq))

import matplotlib.pyplot as plt

plt.ion()
timestamps = []
values = []

loop_time = time.perf_counter()
while True:
  start_time = time.perf_counter()
  loop_time_sum = 0
  loop_count = 0
  while( ( time.perf_counter() - start_time ) < 1 ):
      raw = lightSensor.value
      volts = get_voltage(raw)
      timestamps.append(time.perf_counter())
      timestamps = timestamps[-50:]
      values.append(volts)
      values = values[-50:]
      plt.clf()
      plt.plot(timestamps,values)
      plt.ylim(0, 3.3)
      plt.draw()
      plt.pause(0.0000001)
      loop_count = loop_count + 1
      loop_time_sum = loop_time_sum + ( time.perf_counter() - loop_time )
      loop_time = time.perf_counter()
  average_loop_time = loop_time_sum / loop_count
  average_loop_freq = 1 / average_loop_time
  print("Average loop time: {:1.9f} Average loop freq: {:6f}".format(average_loop_time, average_loop_freq))
