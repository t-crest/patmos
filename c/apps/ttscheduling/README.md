# TT Scheduling
## Experiment of time-triggered cyclic executive scheduling 

# Structure
The core functionality of the dispatcher is implemented in ```tt_minimal_scheduler.c```.
Two demos are presented on how the scheduler and dispatcher can be used on a single thread and a multi-threaded scenario. These are the ```tt_scheduling_demo.c``` and
the ```tt_scheduling_demo_threaded.c```.

# Usage
This experiment is meant to be used in conjuction with the [SimpleSMTScheduler](https://github.com/egk696/SimpleSMTScheduler) that is able to generate the required cyclic schedules for execution. The file ```demo_tasks.h``` implements four demo task that emulate a varied workload and reflects the tasks presented in (https://github.com/egk696/SimpleSMTScheduler/tree/master/examples/demo_tasks.csv).

A full static WCET analysis is supported for all the significant parts of the dispatcher. To WCET analyze the significant parts of the scheduler simply execute:
```make wcet_scheduler```
To WCET analyze the four demo tasks execute:
```make wcet_demo_tasks```

To build the single thread demo execute:
```make tt_scheduling_demo```
To build the multi-threaded demo execute:
```make tt_scheduling_demo_threaded```

The demo can be executed on a simulated enviroment as well as a clock cycle accurate emulated enviroment of the Patmos processor. For example to execute the single threaded
example any of the following commands can be used using the following two targets:
* ```make tt_scheduling_demo sim``` (Simulation)
* ```make tt_scheduling_demo emu``` (Emulation)
* ```make tt_scheduling_demo config download``` (Execute on the altera DE2-115 FPGA platform)