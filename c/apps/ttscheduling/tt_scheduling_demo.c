#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <machine/rtc.h>
#include "tt_minimal_scheduler.h"
#include "demo_tasks.h"


#define HYPER_PERIOD 100000
#define NUM_OF_TASKS 5
#define EXEC_HYPERPERIODS 2
#define RUN_INFINITE false
#define RUNS 1

/*
Utilization = 0.31979

Schedule hyper period = 100000

Satisfied by the following activation times in μs:
[ACT _pit = 1695,
 RCV _pit = 4699,
 MON _pit = 6715,
 SND _pit = 6476,
 SYN _pit = 0]
*/

/*
       0   1   2   4   5
 LEDS[ACT SND RCV MON SYN]
*/

int main()
{
    printf("\nPatmos Time-Triggered Executive Demo\n\n");
    
    uint16_t numExecTasks;
    unsigned long long scheduleTime;
    unsigned long long startTime;
    MinimalTTTask taskSet[NUM_OF_TASKS];
    MinimalTTSchedule schedule;

    for(int i=0; i<RUNS; i++){
        // Tasks are defined in a set with the activation times according to the schedule generation
        init_minimal_tttask(&taskSet[0], 4000, 0, &task_syn);
        init_minimal_tttask(&taskSet[1], 20000, 1695, &task_act);
        init_minimal_tttask(&taskSet[2], 5000, 4699, &task_rcv);
        init_minimal_tttask(&taskSet[3], 10000, 6476, &task_snd);
        init_minimal_tttask(&taskSet[4], 100000, 6715, &task_mon);

        sort_asc_minimal_tttasks(taskSet, NUM_OF_TASKS);

        schedule = init_minimal_ttschedule(HYPER_PERIOD, NUM_OF_TASKS, taskSet, &get_cpu_usecs);

        maxJitter = 0;
        sumJitter = 0;

        printf("Execute Schedule @time(us)=%llu\n\n", get_cpu_usecs());

        startTime = get_cpu_usecs();

        numExecTasks = tt_minimal_schedule_loop(&schedule, EXEC_HYPERPERIODS, RUN_INFINITE);

        scheduleTime = get_cpu_usecs() - startTime;

        printf("\nGathered Statistics (Run# %d)\n", i);
        printf("--No. of hyper periods = %d\n", EXEC_HYPERPERIODS);
        printf("--Theoritic duration = %d μs\n", EXEC_HYPERPERIODS*HYPER_PERIOD);
        printf("--Total execution time = %llu μs\n", scheduleTime);
        printf("--Total no. of executed tasks = %d\n", numExecTasks);
        printf("--Max. Jitter = %+lld μs\n", maxJitter);
        printf("--Avg. Jitter = %+f μs\n", (double) sumJitter/numExecTasks);
    }

    return 0;
}