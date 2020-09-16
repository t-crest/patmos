# RM Scheduling
## Experiment of rate-monotonic scheduling 

# Structure
The core functionality of the online scheduler is implemented in ```rm_minimal_scheduler.c```.

A full static WCET analysis is supported for all the significant parts of the dispatcher. To WCET analyze the significant parts of the scheduler simply execute:
```make wcet_scheduler```

To build the single thread demo execute:
```make rm_scheduling_demo```

The demo can be executed on a simulated enviroment as well as a clock cycle accurate emulated enviroment of the Patmos processor. For example to execute the single threaded
example any of the following commands can be used using the following two targets:
* ```make rm_scheduling_demo sim``` (Simulation)
* ```make rm_scheduling_demo emu``` (Emulation)
* ```make rm_scheduling_demo config download``` (Execute on the altera DE2-115 FPGA platform)
