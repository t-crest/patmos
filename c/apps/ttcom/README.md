# Time-Triggered Intercore Communication

For the evaluation we have provided a pipelined chain of three tasks
for synthetic applications: 

 * `ttcom_pipe.c` time-triggered single buffer application
 * `ttcom_pipe_dub.c` time-triggered double buffer application
 * `mp_pipe.c` message passing single/double buffer application

To compile these applications set the variable `MAIN` at your compile make call.

The application programs can be configured via C based defines, 
which can also be passed to the compiler with command line arguments. 

 * `DATA_LEN` Bulk data size to be sent. Default value is 4096 words (16KB).
 * `BUFFER_SIZE` Buffer size in number of words. Default value is 32 words (128B)
 * `MP_CHAN_NUM_BUF` The buffering schema (double/single buffering). Default value is 2 (double buffering).
 * `MEASUREMENT_MODE` When defined, the application takes timing measuments by reading a clock cycle counter.
the measurements are stored on-chip local memory.
 * `LATENCY_CALC_MODE` when defined, the application prints the end-to-end latency and throughput of sending `DATA_LEN` of data.Configured 'defined' as a default value.
 * `DATA_CHECK_MODE` when defined prints the data received at the receiver side for sanity check purpose.

Following definitions represent different numeric values for each `BUFFER_SIZE` and `MP_CHAN_NUM_BUF` configuration pair.
Values are obtained via statical analysis, and represent the worst-case computation/communication time intervals for any core in the communication chain.
`Table 3` in the paper includes a full set of global period values for each `BUFFER_SIZE` and `MP_CHAN_NUM_BUF` configuration pair.
 * `GLOBAL_PERIOD` Defines the global period (`T<sup>min</sup>` in the `Table 3`) for time-triggered double buffering. 
 * `WCET_COMP` Defines worst-case computation time (`T<sup>comp</sup>` in the `Table 3`) in clock cycles for any core in time-triggered single buffering.
 * `WCET_COMM` Defines worst-case communication time (`T<sup>comm</sup>` in the `Table 3`) in clock cycles for time-triggered single buffering.
 * `TRIGGER_PERIOD` Defines the global period (`T<sup>min</sup>` in the `Table 3`) for time-triggered single buffering.


E.g., the compilation for the default configuration is:


```bash
make -C patmos app APP=ttcom MAIN=ttcom_pipe_dub
```

For the compilation for a specific configuration that includes following parameters:

* Single buffering (MP_CHAN_NUM_BUF=1)
* Buffer size = 64 words (256B)
* Computation time frame = 1287 clock cycle (obtained from Table 3 in the paper)
* Communication time frame = 1135 clock cycle (obtained from Table 3 in the paper)


```bash
make -C patmos app APP=ttcom MAIN=ttcom_pipe COPTS="-D BUFFER_SIZE=64 -D MP_CHAN_NUM_BUF=1 -D WCET_COMP=1287 -D WCET_COMM=1135"
```


The experiments can be executed on an FPGA board.


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
To compile the default configuration:
```bash
cd ~/t-crest
make -C patmos app APP=ttcom MAIN=ttcom_pipe_dub
```


To download the program to the configured FPGA, run the following command:
```bash
make -C patmos APP=ttcom download
```

