import time
import board
from analogio import AnalogIn
import serial
import matplotlib.pyplot as plt

lightSensor = AnalogIn(board.G1)
port = serial.Serial("/dev/ttyACM0", baudrate=4800, timeout=3.0)

def get_voltage(raw):
	return ( raw * 3.3 ) / 65535

plt.ion()
timestamps = []
value = []
counter = 0

while ( counter < 50 ):
	raw = lightSensor.value
	volts = get_voltage(raw)
	timestamps.append(time.perf_counter())
	value.append(volts)
	plt.clf()
	plt.plot(timestamps,value)
	plt.ylim(0, 3.3)
	plt.draw()
	plt.pause(0.0000001)
	counter = counter + 1

loop_time = time.perf_counter()
while True:
	start_time = time.perf_counter()
	loop_time_sum = 0
	loop_count = 0
	while( ( time.perf_counter() - start_time ) < 1 ):
		raw = lightSensor.value
		volts = get_voltage(raw)
		timestamps.append(time.perf_counter())
		timestamps = timestamps[1:]
		value.append(volts)
		value = value[1:]
		plt.clf()
		plt.plot(timestamps,value)
		plt.ylim(0, 3.3)
		plt.draw()
		plt.pause(0.0000001)
		loop_count = loop_count + 1
		loop_time_sum = loop_time_sum + ( time.perf_counter() - loop_time )
		loop_time = time.perf_counter()
	average_loop_time = loop_time_sum / loop_count
	average_loop_freq = 1 / average_loop_time
	print("Average loop time: {:1.9f} Average loop freq: {:6f}".format(average_loop_time, average_loop_freq))
	# while ( port.in_waiting ):
	# 	line = port.readline()
	# 	line = line.decode()
	# 	values = line.split(',')
	# 	if ( values[0] == '$GPRMC' ):
	# 		#if (values[2] == 'A'):
	# 		if (1):
	# 			GPS_time = float(values[1])
	# 			print("Seconds: {:2d}".format(int(GPS_time%100)))