# A I O N
## Time synchronization applications for real-time Ethernet using Patmos

# Experiments
* ```timestamp_compare.c``` : compares hardware timestamping against software timestamping
* ```tsn_timesyncdemo.c``` : implements the IEEE 1588 Precise Time Protocol. It can be configured to run either as PTP Master or Slave with run-time adjustable synchronization interval.
* ```tte_timesyncdemo.c``` : implements a TTEthernet synchronization client with support for
hardware timestamping. It can be started with logging enabled or disabled. It can report the avg. clock offset, task jitter and an analytic report on the received frames with scheduled times and actual reception times.
* ```tte_produceconsume.c```: implements a two-node communication, from the producer to the consumer, with synchronized cyclic executives of five tasks each node using TTEthernet.

Each of the experiments supports static worst-case timing analysis of its core functions using the tool ```platin```.