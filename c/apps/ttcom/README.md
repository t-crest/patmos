# Time-Triggered Communication vs Message Passing

For the evaluation we have provided producer/consumer
based synthetic applications: 

 * `ttcom_pc.c` time-triggered single buffer producer-consumer application
 * `ttcom_pc_double_buf.c` time-triggered double buffer producer-consumer application
 * `mp_pc.c` message passing single/double buffer producer-consumer applications

To compile these applications set the variable `MAIN` at your compile make call.

The application programs can be configured via C based defines, 
which can also be passed to the compiler with command line arguments. 

 * `DATA_LEN` Bulk data size to be sent. Default value is 4096 words (16KB).
 * `BUFFER_SIZE` Buffer size in number of words. Default value is 32 words (128B)
 * `MP_CHAN_NUM_BUF` Number of buffers at the both of receiver and transmitter side. Default value is 2 (double buffering).
 * `MEASUREMENT_MODE` When defined the application takes timing measuments by reading a cycle accurate clock counter.
the measurements are stored on-chip local memory.
 * `LATENCY_CALC_MODE` when defined the application prints the overall latency and throughput of sending `DATA_LEN` of data.Configured as a default value.
 * `DATA_CHECK_MODE` when defined prints the data received at the receiver side for sanity check purpose.

Following definitions represent different numeric values for each `BUFFER_SIZE` and `MP_CHAN_NUM_BUF` configuration pair.
Values are obtained from a set of measuments,and represent the maximum computation time for both of producer and consumer.
`Figure X` in the paper includes a full set of period numbers.
 * `MINOR_PERIOD` Defines minor period in clock cycles for time-triggered double buffering. 
 * `WCET_COMP` Defines maximum computation period in clock cycles for time-triggered single buffering. 
 * `WCET_COMM` Defines maximum communication period in clock cycles for time-triggered single buffering.


E.g., compilation for a configuration is:


```bash
make -C patmos app APP=ttcom MAIN=ttcom_pc COPTS="-D BUFFER_SIZE=64 -D WCET_COMP=458 -D WCET_COMM=1112 -D DATA_CHECK_MODE"
```

The experiments can be execute on an FPGA board.


## FPGA Based Testing

To build platform run the following commands:  
```bash
cd ~/t-crest/aegean
make AEGEAN_PLATFORM=altde2-115-9core platform synth
```
The make command will generate a platform as described in the `config/altde2-115-9core.xml` file. 
When the platform description is generated, then it will be synthesised.

As a second option, you may download the `.sof` file from the following link in order to skip the build process:

`https://www.dropbox.com/s/65aatfggudoy5u9/aegean_top.sof?dl=0`

When the synthesis is finished the multicore platform can be configured 
into the FPGA using the following commands:
```bash
cd ~/t-crest
make -C aegean AEGEAN_PLATFORM=altde2-115-9core config
```
To compile the applications:
```bash
cd ~/t-crest
make -C patmos app APP=ttcom MAIN=mp_pc COPTS="-D BUFFER_SIZE=128 -D MP_CHAN_NUM_BUF=2 -D DATA_CHECK_MODE "
```


To download the program to the configured FPGA, run the following command:
```bash
make -C patmos APP=ttcom download
```

