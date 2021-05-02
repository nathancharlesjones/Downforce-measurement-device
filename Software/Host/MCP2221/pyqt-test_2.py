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

# TODO: Add acquisition of GPS data and speed logging

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
            err_msg.setIcon(QtGui.QMessageBox.Information)
            err_msg.setWindowTitle("Unable to open file: calibration_values.json")
            err_msg.setText("The file 'calibration_values.json' is either missing or corrupted.\n\nThis is normal behavior if the system has never been calibrated.\n\nPress 'Ok' to create a JSON file with default calibration values.\n\nPress 'Cancel' to exit the program without creating a new JSON file.")
            err_msg.setStandardButtons(QtGui.QMessageBox.Ok | QtGui.QMessageBox.Cancel)
            retval = err_msg.exec_()
            if retval == QtGui.QMessageBox.Ok:
                with open(r"calibration_values.json", "w") as file:
                    file.write('''{
                                    "light_sensor_omega":2,
                                    "light_sensor_offset":0,
                                    "load_scale_omega":3,
                                    "load_scale_offset":10,
                                    "Izze_strain_gauge_amplifier_omega":4,
                                    "Izze_strain_gauge_amplifier_offset":20
                                }''')
                calibration_values = open('calibration_values.json','r')
            else:
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

        # Create main graph
        self.graphWidget = pg.PlotWidget()
        self.graphWidget.setLabel('left', 'Downforce (ft-lb)')
        self.graphWidget.setLabel('bottom', 'Time (relative sec)')
        self.graphWidget.setBackground('w')

        # Create calibrate button; first item on bottom toolbar
        self.calibrate_button = QtWidgets.QPushButton("Calibrate", self)
        self.calibrate_button.clicked.connect(self.calibrate)

        # Create window width numeric entry; second item on bottom toolbar
        self.window_width_s = 3
        self.window_width_s_field = QtGui.QLineEdit(str(self.window_width_s))
        self.window_width_s_field.setValidator(QtGui.QIntValidator(1,20))
        self.window_width_s_field.editingFinished.connect(self.newWindowWidth)
        self.window_width_s_field_label = QtGui.QLabel("Window width (sec; Min: 1, Max: 20)")
        self.window_width_s_field_layout = QtWidgets.QVBoxLayout()
        self.window_width_s_field_layout.addWidget(self.window_width_s_field_label)
        self.window_width_s_field_layout.addWidget(self.window_width_s_field)
        self.window_width_s_field_widget = QtWidgets.QWidget()
        self.window_width_s_field_widget.setLayout(self.window_width_s_field_layout)

        # Create sampling frequency numeric entry; third item on bottom toolbar
        self.sampling_freq_Hz = 20
        self.sampling_freq_Hz_field = QtGui.QLineEdit(str(self.sampling_freq_Hz))
        self.sampling_freq_Hz_field.setValidator(QtGui.QIntValidator(1,20))
        self.sampling_freq_Hz_field.editingFinished.connect(self.newSamplingFreq)
        self.sampling_freq_Hz_field_label = QtGui.QLabel("Sampling frequency (Hz; Min: 1, Max: 20)")
        self.sampling_freq_Hz_field_layout = QtWidgets.QVBoxLayout()
        self.sampling_freq_Hz_field_layout.addWidget(self.sampling_freq_Hz_field_label)
        self.sampling_freq_Hz_field_layout.addWidget(self.sampling_freq_Hz_field)
        self.sampling_freq_Hz_field_widget = QtWidgets.QWidget()
        self.sampling_freq_Hz_field_widget.setLayout(self.sampling_freq_Hz_field_layout)

        self.buffer_size = int(self.window_width_s*self.sampling_freq_Hz)
        
        # Create start/stop logging button; fourth item on bottom toolbar
        self.log_start_stop_button = QtWidgets.QPushButton("Start logging", self)
        self.log_start_stop_button.setCheckable(True)
        self.log_start_stop_button.clicked.connect(self.startStopLogging)

        # Create toolbar with horizontal layout
        # | Calibrate button | Window width numeric entry | Sampling freq numeric entry | Start/stop logging button | 
        self.toolbar_layout = QtWidgets.QHBoxLayout()
        self.toolbar_layout.addWidget(self.calibrate_button)
        self.toolbar_layout.addWidget(self.window_width_s_field_widget)
        self.toolbar_layout.addWidget(self.sampling_freq_Hz_field_widget)
        self.toolbar_layout.addWidget(self.log_start_stop_button)
        self.toolbar_widget = QtWidgets.QWidget()
        self.toolbar_widget.setLayout(self.toolbar_layout)

        # Combine graph and toolbar into a single widget
        self.graph_and_toolbar_layout = QtWidgets.QVBoxLayout()
        self.graph_and_toolbar_layout.addWidget(self.graphWidget)
        self.graph_and_toolbar_layout.addWidget(self.toolbar_widget)
        
        # Set up the main window
        self.setWindowTitle("Downforce Measurement Device, Carter's Customs, LLC")
        self.main_widget = QtWidgets.QWidget()
        self.main_widget.setLayout(self.graph_and_toolbar_layout)
        self.setCentralWidget(self.main_widget)

        self.time_vals = list(np.linspace(-self.window_width_s,0,self.buffer_size))
        self.light_sensor = [0] * self.buffer_size
        self.load_scale = [0] * self.buffer_size
        self.Izze_strain_gauge_amplifier = [0] * self.buffer_size

        self.red_pen = pg.mkPen(color=(255, 0, 0))
        self.light_sensor_line =  self.graphWidget.plot(self.time_vals, self.light_sensor, pen=self.red_pen)
        
        self.green_pen = pg.mkPen(color=(0, 255, 0))
        self.load_scale_line =  self.graphWidget.plot(self.time_vals, self.load_scale, pen=self.green_pen)

        self.blue_pen = pg.mkPen(color=(0, 0, 255))
        self.Izze_strain_gauge_amplifier_line =  self.graphWidget.plot(self.time_vals, self.Izze_strain_gauge_amplifier, pen=self.blue_pen)

        self.timer = QtCore.QTimer()
        self.timer.setInterval(int(1000/self.sampling_freq_Hz))
        self.timer.timeout.connect(self.update_plot_data)
        self.timer.start()

    def calibrate(self):
        print("Calibrating")    
        # TODO: Add this screen

    def newWindowWidth(self):
        self.window_width_s = int(self.window_width_s_field.text())
        self.updateBuffers()            

    def newSamplingFreq(self):
        self.sampling_freq_Hz = int(self.sampling_freq_Hz_field.text())
        self.timer.setInterval(int(1000/self.sampling_freq_Hz))
        self.updateBuffers()

    def updateBuffers(self):
        old_time_vals = self.time_vals
        self.buffer_size = int(self.window_width_s*self.sampling_freq_Hz)
        self.time_vals = list(np.linspace(-self.window_width_s,0,self.buffer_size))
        self.light_sensor = list(np.interp(self.time_vals,old_time_vals,self.light_sensor))
        self.load_scale = list(np.interp(self.time_vals,old_time_vals,self.load_scale))
        self.Izze_strain_gauge_amplifier = list(np.interp(self.time_vals,old_time_vals,self.Izze_strain_gauge_amplifier))

    def startStopLogging(self):
        if self.log_start_stop_button.isChecked():
            self.logging = True
            self.filename = os.path.join(self.dirname, "{}.csv".format(dt.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")))
            # If the file doesn't exist, open it and write the headers
            if not os.path.exists(self.filename):
                with open(r"{}".format(self.filename), "w") as file:
                    file.write("{},{},{},{}\n".format("Time","Light sensor values","Load scale values","Izze strain gauge amplifier values"))
            self.log_start_stop_button.setText("Stop logging")
        else:
            self.logging = False
            self.log_start_stop_button.setText("Start logging")

    def update_plot_data(self):
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
                file.write("{},{:1.3f},{:1.3f},{:1.3f}\n".format(dt.datetime.now().strftime('%H:%M:%S.%f'),newVal1,newVal2,newVal3))

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