#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import pyqtgraph as pg
from pyqtgraph.ptime import time
import serial

app = QtGui.QApplication(["Clock Synchronization Offset"])
pg.setConfigOption('background', 'w')
pg.setConfigOption('foreground', 'b')
pg.setConfigOptions(antialias=True)
win = pg.GraphicsWindow(title="Basic plotting examples")
win.resize(1920,1080)
win.setWindowTitle('pyqtgraph example: Plotting')
p = win.addPlot(title="Slave Clock Offset", pen='k')
p.showGrid(x = True, y = True, alpha = 0.3)
curve = p.plot(pen='b', symbol='o', brush=pg.mkBrush(color='k'))
curve1 = p.plot(pen='r')

p1 = win.addPlot(title="Clock Offset Std.Deviation")
curve2 = p1.plot(pen='g')

data = [0]
avgdata = [0]
stddata = [0]

try:
    serialPort = sys.argv[1]
except (ValueError, TypeError, IndexError):
    serialPort = "/dev/ttyUSB0"
    print "First parameter missing or not valid (default=/dev/ttyUSB0)"

try:
    thres = int(sys.argv[2])
except (ValueError, TypeError, IndexError):
    thres = 10000
    print "Second parameter missing or not valid for plot Y-axis limits (default=10000)"

try:
    csvname = sys.argv[3]
except (ValueError, TypeError, IndexError):
    csvname = "clock_offset_measurement.csv"
    print "Third parameter missing or not valid (default=clock_offset_measurement.csv"


raw=serial.Serial(serialPort,115200)
csvfile = open(csvname, 'w+')

def mean(numbers):
    return float(sum(numbers)) / max(len(numbers), 1)

def update():
    global curve, curve1, curve2, data, avgdata, stddata
    line = raw.readline()
    if(line.startswith("#")):
        csvfile.write(line)
        try:
            seq,sec,nano = line.split("\t")
            y = float(nano)
            data.append(max(min(y, thres), -thres))
            xdata = np.array(data, dtype='int32')
            curve.setData(xdata)
            runningAvg = mean(data)
            avgdata.append(runningAvg)
            xavgdata = np.array(avgdata)
            curve1.setData(xavgdata)
            print "Average=", runningAvg
            if(len(xdata) > 10):
                runningStdDev = np.std(xdata[10:])
                stddata.append(runningStdDev)
                xstddata = np.array(stddata, dtype='int32')
                curve2.setData(xstddata)
                app.processEvents()
                print "Std.Dev=", runningStdDev
        except (ValueError, TypeError):
            print "Oops I got: " + line
        except KeyboardInterrupt:
            print "Exiting..."
            csvfile.close()
            exit(0)
    elif(line.startswith("$")):
        print "Stream stopped"
        csvfile.close()
        timer.stop()
            

timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(0)

if __name__ == '__main__':
    if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
        QtGui.QApplication.instance().exec_()
        csvfile.close()
        raw.close()
