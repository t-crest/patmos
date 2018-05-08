#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import pyqtgraph as pg
from pyqtgraph.ptime import time
import serial

app = QtGui.QApplication([])
pg.setConfigOption('background', 'w')
pg.setConfigOption('foreground', 'b')

p = pg.plot(pen='r')
p.setWindowTitle('live plot from serial')
curve = p.plot(pen='r')

data = [0]
raw=serial.Serial("/dev/ttyUSB0",115200)
# raw.open()

try:
    thres = int(sys.argv[1])
except (ValueError, TypeError, IndexError):
    thres = 10000
    print "Parameter missing or not valid for plot Y-axis limits (default=10000)"

def update():
    global curve, data
    line = raw.readline()
    if(line.startswith("#")):
        try:
            seq,sec,nano = line.split("\t")
            y = int(nano)
            data.append(max(min(y, thres), -thres))
            xdata = np.array(data, dtype='int32')
            curve.setData(xdata)
            app.processEvents()
        except (ValueError, TypeError):
            print "Oops!"

timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(0)

if __name__ == '__main__':
    import sys
    if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
        QtGui.QApplication.instance().exec_()