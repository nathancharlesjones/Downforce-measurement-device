from PyQt5 import QtWidgets, QtCore
from pyqtgraph import PlotWidget, plot
import pyqtgraph as pg
import sys  # We need sys so that we can pass argv to QApplication
import os
import time
import datetime as dt
import math
import json
import numpy as np

# Globals
dirname = os.path.dirname(__file__)

# Opening and reading JSON for calibration data
try:
    calibration_values = open('calibration_values.json','r')
except IOError:
    sys.exit("Error opening the file 'calibration_values.json'.")

try:
    decoded_values = json.load(calibration_values)
    light_sensor_omega = decoded_values["light_sensor_omega"]
    light_sensor_offset = decoded_values["light_sensor_offset"]
    load_scale_omega = decoded_values["load_scale_omega"]
    load_scale_offset = decoded_values["load_scale_offset"]
    Izze_strain_gauge_amplifier_omega = decoded_values["Izze_strain_gauge_amplifier_omega"]
    Izze_strain_gauge_amplifier_offset = decoded_values["Izze_strain_gauge_amplifier_offset"]
except (json.decoder.JSONDecodeError):
    sys.exit("Error loading JSON values from 'calibration_values.json'.")

# Getting current time; used for filename of data log
now = dt.datetime.now()

class MainWindow(QtWidgets.QMainWindow):

    def __init__(self, *args, **kwargs):
        super(MainWindow, self).__init__(*args, **kwargs)

        self.window_width_s = 3
        self.sampling_period_ms = 50
        self.buffer_size = int(1000*self.window_width_s/self.sampling_period_ms)
        self.logging = False
        self.filename = ''
        self.counter = 0
        self.last_time = time.perf_counter()

        self.graphWidget = pg.PlotWidget()
        self.graphWidget.setLabel('left', 'Downforce (ft-lb)')
        self.graphWidget.setLabel('bottom', 'Time (relative sec)')
        self.graphWidget.setBackground('w')

        self.log_start_stop_button = QtWidgets.QPushButton("Start logging", self)
        self.log_start_stop_button.setCheckable(True)
        self.log_start_stop_button.clicked.connect(self.startStopLogging)

        self.setWindowTitle("Downforce Measurement Device, Carter's Customs, LLC")
        layout = QtWidgets.QVBoxLayout()
        layout.addWidget(self.graphWidget)
        layout.addWidget(self.log_start_stop_button)
        widget = QtWidgets.QWidget()
        widget.setLayout(layout)
        self.setCentralWidget(widget)

        self.time_vals = np.linspace(-self.window_width_s,0,self.buffer_size)
        self.light_sensor = [0] * self.buffer_size
        self.load_scale = [0] * self.buffer_size
        self.Izze_strain_gauge_amplifier = [0] * self.buffer_size

        red_pen = pg.mkPen(color=(255, 0, 0))
        self.light_sensor_line =  self.graphWidget.plot(self.time_vals, self.light_sensor, pen=red_pen)

        green_pen = pg.mkPen(color=(0, 255, 0))
        self.load_scale_line =  self.graphWidget.plot(self.time_vals, self.load_scale, pen=green_pen)

        blue_pen = pg.mkPen(color=(0, 0, 255))
        self.Izze_strain_gauge_amplifier_line =  self.graphWidget.plot(self.time_vals, self.Izze_strain_gauge_amplifier, pen=blue_pen)

        self.timer = QtCore.QTimer()
        self.timer.setInterval(self.sampling_period_ms)
        self.timer.timeout.connect(self.update_plot_data)
        self.timer.start()

    def update_plot_data(self):

        #self.time_vals = self.time_vals[1:]  # Remove the first y element.
        #self.time_vals.append(self.time_vals[-1]+1)  # Add a new value 1 higher than the last.

        self.light_sensor = self.light_sensor[1:]  # Remove the first
        newVal1 = math.sin(time.time()*light_sensor_omega + light_sensor_offset)
        self.light_sensor.append(newVal1)  # Add a new random value.
        self.light_sensor_line.setData(self.time_vals, self.light_sensor)  # Update the data.

        self.load_scale = self.load_scale[1:]  # Remove the first
        newVal2 = math.sin(time.time()*load_scale_omega + load_scale_offset)
        self.load_scale.append(newVal2)  # Add a new random value.
        self.load_scale_line.setData(self.time_vals, self.load_scale)  # Update the data.

        self.Izze_strain_gauge_amplifier = self.Izze_strain_gauge_amplifier[1:]  # Remove the first
        newVal3 = math.sin(time.time()*Izze_strain_gauge_amplifier_omega + Izze_strain_gauge_amplifier_offset)
        self.Izze_strain_gauge_amplifier.append(newVal3)  # Add a new random value.
        self.Izze_strain_gauge_amplifier_line.setData(self.time_vals, self.Izze_strain_gauge_amplifier)  # Update the data.        

        if self.logging:
            with open(r"{}".format(self.filename), "a") as file:
                file.write("{},{:1.3f}\n".format(dt.datetime.now().strftime('%H:%M:%S.%f'),newVal1))

        #self.counter += 1
        #time_diff = time.perf_counter() - self.last_time
        #if ( time_diff > 1 ):
            #print("{:4d} samples processed in {:1.9f} seconds. Update frequency: {:4.1f} Hz".format(self.counter,time_diff,(self.counter/time_diff)))
            #self.last_time = time.perf_counter()
            #self.counter = 0
    
    def startStopLogging(self):
        if self.log_start_stop_button.isChecked():
            self.logging = True
            self.filename = os.path.join(dirname, "{}.csv".format(dt.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")))
            self.log_start_stop_button.setText("Stop logging")
        else:
            self.logging = False
            self.log_start_stop_button.setText("Start logging")


app = QtWidgets.QApplication(sys.argv)
w = MainWindow()
w.show()
sys.exit(app.exec_())