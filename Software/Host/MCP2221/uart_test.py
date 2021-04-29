import serial

port = serial.Serial("/dev/ttyACM0", baudrate=4800, timeout=3.0)

#while True:
#	line = port.readline()
#	print(line)

values = []
while True:
	line = port.readline()
	line = line.decode()
	values = line.split(',')
	if ( values[0] == '$GPRMC' ):
		# if (values[2] == 'A'):
		if (True):
			GPS_time = float(values[1])
			print("Seconds: {:2d}".format(int(GPS_time%100)))

# while True:
# 	print(port.in_waiting)