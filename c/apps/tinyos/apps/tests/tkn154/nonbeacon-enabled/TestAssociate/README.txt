README for TestAssociate
Author/Contact: Jan Hauer <hauer@tkn.tu-berlin.de>

Description:

In this application one node takes the role of a PAN coordinator in a
nonbeacon-enabled 802.15.4 PAN, it switches its radio to receive mode and waits
for devices to request association to its PAN. Whenever a device tries to
associate, the PAN coordinator allows association and assigns to the device a
unique short address (starting from zero, incremented for every device
requesting association).  A second node acts as a device, it switches to the
pre-defined channel and tries to associate to the PAN. A short time after
association the device then disassociates from the PAN. 

Criteria for a successful test:

Assuming one coordinator and one device has been installed, both should
simultaneously switch on LED1. About 5 seconds later both should switch LED1
off. That's all.


Tools: NONE

Usage: 

1. Install the coordinator:

    $ cd coordinator; make <platform> install

2. Install one (or more) devices:

    $ cd device; make <platform> install

You can change some of the configuration parameters in app_profile.h

Known bugs/limitations:

- Many TinyOS 2 platforms do not have a clock that satisfies the
  precision/accuracy requirements of the IEEE 802.15.4 standard (e.g. 
  62.500 Hz, +-40 ppm in the 2.4 GHz band); in this case the MAC timing 
  is not standard compliant

$Id: README.txt,v 1.3 2010-01-05 17:12:56 janhauer Exp $o

