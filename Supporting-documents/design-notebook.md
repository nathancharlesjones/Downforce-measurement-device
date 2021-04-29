# Thursday, 29 Apr 2021
- Trying to get back into this with a DTSTTCPW mentality!
- Starting with MCP2221. "adc_test_v3.py" was the latest version, IIRC. Update rate is ~150 Hz for the default window size and ~40 Hz when full-screen
- Profiling adc_test_v3.py
    - With no ADC measurement and no plot updates: ~300 Hz
    - With ADC measurement and no plot updates: ~140 Hz
    - With no ADC measurement and plot updates: ~290 Hz
    - Okay, so the problem clearly seems to be how long it takes to make an ADC measurement
- So I'll need to use the STM32 as my base (which I was expecting anyways, since the MCP2221 doesn't have SPI or CAN)
- Possible alternatives:
    - STM32 <--> SPI (API: "readADC", "setFreq", etc) <--> RPi / matplotlib / PyQT / tkinter
    - STM32 <--> SPI (API: "setReg" and "readReg") <--> RPi / matplotlib / PyQT / tkinter
    - STM32 <--> USB serial (JSON or other serialization scheme; API: "readADC", "setFreq", etc) <--> matplotlib / PyQT / tkinter
    - STM32 <--> USB serial (JSON or other serialization scheme; API: "setReg" and "readReg") <--> matplotlib / PyQT / tkinter
    - STM32 <--> UART (JSON or other serialization scheme; API: "readADC", "setFreq", etc) <--> matplotlib / PyQT / tkinter
    - STM32 <--> UART (JSON or other serialization scheme; API: "setReg" and "readReg") <--> matplotlib / PyQT / tkinter
    - STM32 <--> J-Link <--> uC/Probe
    - STM32 <--> USB <--> uC/Probe
    - STM32 <--> J-Link <--> pyGDB / matplotlib / PyQT / tkinter
        - Possible to control STM32 from pyGDB entirely, without the need for any firmware??
        - Possible to call HAL functions from pyGDB?
- Considered and rejected/tabled:
    - StandardFirmata: Seems to have Analog/Digital/PWM/I2C built in, but no SPI or CAN (I'd have to patch those in myself). Also, pyfirmata only seems to support Analog/Digital/PWM/Servo (no I2C).
    - Ozone Data Sampling: Seems limited to just a time-series, not actual plotting. Plus, I'm not sure I want to walk a person through setting up and running Ozone.
    - Possible to use SWO in the future?
    - STM32 <--> UART (DebugMonitor / MRI) <--> matplotlib / PyQT || STM32 <--> USB serial (DebugMonitor / MRI) ??? <--> matplotlib / PyQT
        - Links: [DebugMonitor](https://interrupt.memfault.com/blog/cortex-m-debug-monitor) | [MRI](https://github.com/adamgreen/mri)
        - Not exactly sure of the added benefit of DebugMonitor here; it seems like just a fancy serial interface, which I can use to catch debug events (like breakpoints) and to single-step through the code.
    - [gnuplot](https://mcuoneclipse.com/2020/02/09/visualizing-data-with-eclipse-gdb-and-gnuplot/) (more info [here](https://sourceware.org/gdb/wiki/PlottingFromGDB)) / [gdb-plot](https://github.com/bthcode/gdb-plot) / [gdbplotlib](https://github.com/X-Neon/gdbplotlib) (get it [here](https://pypi.org/project/gdbplotlib/)): These seem REALLY great for generating data plots, but not so much for real-time data.
    - Possible to use STM32 HAL w/o entirely re-writing it?
        - Somehow overload/intercept the register reads/writes to use pyGDB/DebugMon/MRI/CLI?
        - Would need to compile HAL for x86 (not sure if _that's_ possible) into a static library. Then I could call it from either a C/C++ or Python program on the host computer.
        - Possible to load desired functions onto STM32 with an empty while(1) loop and then call the functions from pyGDB?
- Do I need real-time graphing anyways? I kind of just need a measure of liveliness, so a digital readout might also work.
- Faster Python
    - [PyPy](https://www.pypy.org/)
    - [Auto PY to EXE](https://dev.to/eshleron/how-to-convert-py-to-exe-step-by-step-guide-3cfi)
    - [Nutika](https://github.com/Nuitka/Nuitka)
        - Possible to use this to compile for Arm??
- [Possible faster real-time plot with matplotlib](https://gist.github.com/Uberi/283a13b8a71a46fb4dc8)
- [Tkinter tutorial](https://pythonprogramming.net/plotting-live-bitcoin-price-data-tkinter-matplotlib/)
- [Live update graphs with PyQT](https://www.mfitzp.com/tutorials/plotting-pyqtgraph/)