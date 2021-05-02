from PyQt5 import QtWidgets, QtCore, QtGui
from pyqtgraph import PlotWidget, plot
import pyqtgraph as pg
import sys  # We need sys so that we can pass argv to QApplication
import os
import time
import datetime as dt
import math
import json
import numpy as np

class MainWindow(QtWidgets.QMainWindow):

    def __init__(self, *args, **kwargs):
        super(MainWindow, self).__init__(*args, **kwargs)

        self.logging = False
        self.dirname = os.path.dirname(__file__)
        self.filename = ''
        self.counter = 0
        self.last_time = time.perf_counter()

        # Opening and reading JSON for calibration data
        try:
            calibration_values = open('calibration_values.json','r')
        except IOError:
            err_msg = QtGui.QMessageBox()
            err_msg.setIcon(QtGui.QMessageBox.Critical)
            err_msg.setWindowTitle("Program Error")
            err_msg.setText("Error opening the file 'calibration_values.json'.")
            err_msg.setStandardButtons(QtGui.QMessageBox.Ok)
            retval = err_msg.exec_()
            sys.exit()

        try:
            decoded_values = json.load(calibration_values)
            self.light_sensor_omega = decoded_values["light_sensor_omega"]
            self.light_sensor_offset = decoded_values["light_sensor_offset"]
            self.load_scale_omega = decoded_values["load_scale_omega"]
            self.load_scale_offset = decoded_values["load_scale_offset"]
            self.Izze_strain_gauge_amplifier_omega = decoded_values["Izze_strain_gauge_amplifier_omega"]
            self.Izze_strain_gauge_amplifier_offset = decoded_values["Izze_strain_gauge_amplifier_offset"]
        except (json.decoder.JSONDecodeError):
            err_msg = QtGui.QMessageBox()
            err_msg.setIcon(QtGui.QMessageBox.Critical)
            err_msg.setWindowTitle("Program Error")
            err_msg.setText("Error loading JSON values from 'calibration_values.json'.")
            err_msg.setStandardButtons(QtGui.QMessageBox.Ok)
            retval = err_msg.exec_()
            sys.exit()

        self.graphWidget = pg.PlotWidget()
        self.graphWidget.setLabel('left', 'Downforce (ft-lb)')
        self.graphWidget.setLabel('bottom', 'Time (relative sec)')
        self.graphWidget.setBackground('w')

        self.calibrate_button = QtWidgets.QPushButton("Calibrate", self)
        self.calibrate_button.clicked.connect(self.calibrate)

        self.window_width_s = 3
        self.window_width_s_field = QtGui.QLineEdit(str(self.window_width_s))
        self.window_width_s_field.setValidator(QtGui.QIntValidator(1,20))
        self.window_width_s_field.editingFinished.connect(self.newWindowWidth)

        self.sampling_period_ms = 50
        self.sampling_period_ms_field = QtGui.QLineEdit(str(self.sampling_period_ms))
        self.sampling_period_ms_field.setValidator(QtGui.QIntValidator(20,1000))
        self.sampling_period_ms_field.editingFinished.connect(self.newSamplingPeriod)

        self.buffer_size = int(1000*self.window_width_s/self.sampling_period_ms)
        
        self.log_start_stop_button = QtWidgets.QPushButton("Start logging", self)
        self.log_start_stop_button.setCheckable(True)
        self.log_start_stop_button.clicked.connect(self.startStopLogging)

        self.setWindowTitle("Downforce Measurement Device, Carter's Customs, LLC")
        self.toolbar_layout = QtWidgets.QHBoxLayout()
        self.toolbar_layout.addWidget(self.calibrate_button)
        self.toolbar_layout.addWidget(self.window_width_s_field)
        self.toolbar_layout.addWidget(self.sampling_period_ms_field)
        self.toolbar_layout.addWidget(self.log_start_stop_button)
        self.toolbar_widget = QtWidgets.QWidget()
        self.toolbar_widget.setLayout(self.toolbar_layout)

        self.graph_and_toolbar_layout = QtWidgets.QVBoxLayout()
        self.graph_and_toolbar_layout.addWidget(self.graphWidget)
        self.graph_and_toolbar_layout.addWidget(self.toolbar_widget)
        
        self.main_widget = QtWidgets.QWidget()
        self.main_widget.setLayout(self.graph_and_toolbar_layout)
        self.setCentralWidget(self.main_widget)

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

    def calibrate(self):
        print("Calibrating")    

    def newWindowWidth(self):
        print("New window width",self.window_width_s_field.text())

    def newSamplingPeriod(self):
        print("New sampling period",self.sampling_period_ms_field.text())

    def startStopLogging(self):
        if self.log_start_stop_button.isChecked():
            self.logging = True
            self.filename = os.path.join(self.dirname, "{}.csv".format(dt.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")))
            self.log_start_stop_button.setText("Stop logging")
        else:
            self.logging = False
            self.log_start_stop_button.setText("Start logging")

    def update_plot_data(self):

        #self.time_vals = self.time_vals[1:]  # Remove the first y element.
        #self.time_vals.append(self.time_vals[-1]+1)  # Add a new value 1 higher than the last.

        self.light_sensor = self.light_sensor[1:]  # Remove the first
        newVal1 = math.sin(time.time()*self.light_sensor_omega + self.light_sensor_offset)
        self.light_sensor.append(newVal1)  # Add a new random value.
        self.light_sensor_line.setData(self.time_vals, self.light_sensor)  # Update the data.

        self.load_scale = self.load_scale[1:]  # Remove the first
        newVal2 = math.sin(time.time()*self.load_scale_omega + self.load_scale_offset)
        self.load_scale.append(newVal2)  # Add a new random value.
        self.load_scale_line.setData(self.time_vals, self.load_scale)  # Update the data.

        self.Izze_strain_gauge_amplifier = self.Izze_strain_gauge_amplifier[1:]  # Remove the first
        newVal3 = math.sin(time.time()*self.Izze_strain_gauge_amplifier_omega + self.Izze_strain_gauge_amplifier_offset)
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

app = QtWidgets.QApplication(sys.argv)
w = MainWindow()
w.show()
sys.exit(app.exec_())