#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <machine/rtc.h>
#include "tt_minimal_scheduler.h"
#include "demo_tasks.h"

#define HYPER_PERIOD 20000
#define NUM_OF_TASKS 4
#define HYPER_ITERATIONS 1
#define RUN_INFINITE false
#define RUNS 1

int main()
{
    printf("\nPatmos Time-Triggered Executive Demo\n\n");
    
    uint16_t numExecTasks;
    uint64_t scheduleTime;
    uint64_t startTime;
    MinimalTTTask taskSet[NUM_OF_TASKS];
    MinimalTTSchedule schedule;

    for(int i=0; i<RUNS; i++){
        // Tasks are defined in a set with the activation times according to the schedule generation
        init_minimal_tttask(&taskSet[0], 5000, T1_sched_insts, 4, &task_1);
        init_minimal_tttask(&taskSet[1], 10000, T2_sched_insts, 2, &task_2);
        init_minimal_tttask(&taskSet[2], 2500, T3_sched_insts, 8, &task_3);
        init_minimal_tttask(&taskSet[3], 20000, T4_sched_insts, 1, &task_4);

        sort_asc_minimal_tttasks(taskSet, NUM_OF_TASKS);

        schedule = init_minimal_ttschedule(HYPER_PERIOD, NUM_OF_TASKS, taskSet, &get_cpu_usecs);

        printf("Execute Schedule @start_time(us)=%llu\n\n", get_cpu_usecs());

        startTime = get_cpu_usecs();

        numExecTasks = tt_minimal_schedule_loop(&schedule, HYPER_ITERATIONS, RUN_INFINITE);

        scheduleTime = get_cpu_usecs() - startTime;

        printf("\nGathered Statistics (Run# %d)\n", i);
        printf("--No. of hyper period iterations = %d\n", HYPER_ITERATIONS);
        printf("--Theoritic duration = %d μs\n", HYPER_ITERATIONS*HYPER_PERIOD);
        printf("--Total execution time = %llu μs\n", scheduleTime);
        printf("--Total no. of executed tasks = %d\n", numExecTasks);
        for(int i=0; i<NUM_OF_TASKS; i++){
            uint64_t avgDelta = schedule.tasks[i].delta_sum/schedule.tasks[i].exec_count;
            printf("--task[%d].period = %lld, executed with avg. dt = %llu (avg. jitter = %d) from a total of %lu executions\n", i, schedule.tasks[i].period, 
            avgDelta, (int) schedule.tasks[i].period - (int) avgDelta, schedule.tasks[i].exec_count);
        }
    }

    return 0;
}