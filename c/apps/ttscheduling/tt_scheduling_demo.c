#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <machine/rtc.h>
#include "tt_minimal_scheduler.h"
#include "demo_tasks.h"
#include "schedule.h"

#define HYPER_ITERATIONS 100
#define RUN_INFINITE false

void convert_sched_to_timebase(uint64_t *sched_insts, uint32_t nr_insts, double timebase){
    for(int i=0; i<nr_insts; i++){
        sched_insts[i] = (uint64_t) (sched_insts[i] * timebase);
    }
}

int main()
{
    LED = 0;
    printf("\nPatmos Time-Triggered Executive Demo\n");
    
    uint16_t numExecTasks;
    uint64_t scheduleTime;
    uint64_t startTime;
    MinimalTTTask taskSet[NUM_OF_TASKS];
    MinimalTTSchedule schedule;

    static void (*tasks_func_ptrs[NUM_OF_TASKS])(const void*) = {task_1, task_2, task_3, task_4, task_5, task_6, task_7, task_8};

    // Tasks are defined in a set with the activation times according to the schedule generation
    for(unsigned int i=0; i<NUM_OF_TASKS; i++){
        convert_sched_to_timebase(tasks_schedules[i], tasks_insts_counts[i], NS_TO_US);
        init_minimal_tttask(&taskSet[i], (uint64_t)(tasks_periods[i] * NS_TO_US), tasks_schedules[i], tasks_insts_counts[i], tasks_func_ptrs[i]);
    }

    schedule = init_minimal_ttschedule((uint64_t)(HYPER_PERIOD * NS_TO_US), NUM_OF_TASKS, taskSet, &get_cpu_usecs);

    printf("\nSchedule start_time(us)=%llu\n", get_cpu_usecs());

    startTime = get_cpu_usecs();

    numExecTasks = tt_minimal_schedule_loop(&schedule, HYPER_ITERATIONS, RUN_INFINITE);

    scheduleTime = get_cpu_usecs() - startTime;

    printf("\nGathered Statistics\n");
    printf("--No. of hyper period iterations = %u\n", HYPER_ITERATIONS);
    printf("--Theoritic duration = %llu μs\n", (uint64_t) HYPER_ITERATIONS * schedule.hyper_period);
    printf("--Total execution time = %llu μs\n", scheduleTime);
    printf("--Total no. of executed tasks = %d\n", numExecTasks);
    for(int i=0; i<NUM_OF_TASKS; i++){
        double avgDelta = (double) (schedule.tasks[i].delta_sum/ (double)schedule.tasks[i].exec_count);
        printf("--task[%d].period = %lld μs, executed with avg. dt = %.3f μs (jitter = %.3f μs) and a total of %lu executions\n", i, schedule.tasks[i].period, 
        avgDelta, schedule.tasks[i].period - avgDelta, schedule.tasks[i].exec_count);
    }

    return 0;
}