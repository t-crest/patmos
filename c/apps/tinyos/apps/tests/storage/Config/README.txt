README for Config
Author/Contact: tinyos-help@millennium.berkeley.edu

Description:

Application to test the ConfigStorageC abstraction. There must be a
volumes-<chip>.xml file in this directory describing the a volume
named CONFIGTEST capable of storing 2kB of config data.

The mote id is of the form T*100 + k, where k is a random seed and
T specifies the test to be performed:

T = 0: do a bunch of writes, reads and commits
T != 0: check if the contents of the volume are consistent with 
        a previous run with T = 0 and the same random seed

A successful test will turn on LED 1. A failed test will turn on the
LED 0. LED 2 blinks to indicate test progress. A serial message whose
last byte is 0x80 for success and all other values indicate failure is
also sent at the end of the test.

Tools:

Known bugs/limitations:

None.

$Id: README.txt,v 1.5 2007-07-09 20:45:54 idgay Exp $
