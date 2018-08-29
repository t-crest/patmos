#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import pyqtgraph as pg
from pyqtgraph.ptime import time
import serial

app = QtGui.QApplication(["PTP Slave Offset (/dev/ttyUSB0)"])
pg.setConfigOption('background', 'w')
pg.setConfigOption('foreground', 'b')
pg.setConfigOptions(antialias=True)
win = pg.GraphicsWindow(title="Basic plotting examples")
win.resize(1920,1080)
win.setWindowTitle('pyqtgraph example: Plotting')
p = win.addPlot(title="PTP Slave Offset", pen='k')
p.showGrid(x = True, y = True, alpha = 0.3)
curve = p.plot(pen='b', symbol='o', brush=pg.mkBrush(color='k'))
curve1 = p.plot(pen='r')

p1 = win.addPlot(title="PTP Slave Std.Deviation")
curve2 = p1.plot(pen='g')

data = [0]
avgdata = [0]
stddata = [0]
serialPort = "/dev/ttyUSB0"
raw=serial.Serial(serialPort,115200)
# raw.open()

try:
    thres = int(sys.argv[1])
except (ValueError, TypeError, IndexError):
    thres = 10000
    print "Parameter missing or not valid for plot Y-axis limits (default=10000)"

def mean(numbers):
    return float(sum(numbers)) / max(len(numbers), 1)

def update():
    global curve, curve1, curve2, data, avgdata, stddata
    line = raw.readline()
    if(line.startswith("#")):
        try:
            seq,sec,nano = line.split("\t")
            y = int(nano)
            data.append(max(min(y, thres), -thres))
            xdata = np.array(data, dtype='int32')
            curve.setData(xdata)
            runningAvg = mean(data)
            avgdata.append(runningAvg)
            xavgdata = np.array(avgdata)
            curve1.setData(xavgdata)
            print "Average=", runningAvg
            if(len(xdata) > 100):
                runningStdDev = np.std(xdata)
                stddata.append(runningStdDev)
                xstddata = np.array(stddata, dtype='int32')
                curve2.setData(xstddata)
                app.processEvents()
                print "Std.Dev=", runningStdDev
        except (ValueError, TypeError):
            print "Oops!"
	except KeyboardInterrupt:
            print "Exiting"
            exit(0)
            

timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(0)

if __name__ == '__main__':
    if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
        QtGui.QApplication.instance().exec_()
        raw.close()
