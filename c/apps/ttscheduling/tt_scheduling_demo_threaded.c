#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <machine/rtc.h>
#include "libcorethread/corethread.h"
#include "tt_minimal_scheduler.h"
#include "demo_tasks.h"

#define HYPER_PERIOD 20000
#define NUM_OF_TASKS 4
#define EXEC_HYPERPERIODS 1
#define RUN_INFINITE false

void threaded_schedule_work(void *params)
{
    sort_asc_minimal_tttasks(((MinimalTTSchedule *)params)->tasks, ((MinimalTTSchedule *)params)->num_tasks);

    uint32_t executedTasks = tt_minimal_schedule_loop((MinimalTTSchedule *)params, EXEC_HYPERPERIODS, RUN_INFINITE);

    corethread_exit((void*) executedTasks);
}

int main()
{
    printf("\nPatmos Time-Triggered Executive Demo (Threaded)\n\n");

    int err = 0;
    int sumTasksThread1 = 0;
    int sumTasksThread2 = 0;
    uint32_t numExecTasks[2];
    uint64_t scheduleTime;
    uint64_t startTime;

    // Define set of tasks
    MinimalTTTask taskSet_1[2];
    init_minimal_tttask(&taskSet_1[0], 5000, T1_sched_insts, 4, &task_1);
    init_minimal_tttask(&taskSet_1[1], 10000, T2_sched_insts, 2, &task_2);
    MinimalTTSchedule scheduleSet_1 = init_minimal_ttschedule(HYPER_PERIOD, 2, taskSet_1, &get_cpu_usecs);

    MinimalTTTask taskSet_2[2];
    init_minimal_tttask(&taskSet_2[0], 2500, T3_sched_insts, 8, &task_3);
    init_minimal_tttask(&taskSet_2[1], 20000, T4_sched_insts, 1, &task_4);
    MinimalTTSchedule scheduleSet_2 = init_minimal_ttschedule (HYPER_PERIOD, 2, taskSet_2, &get_cpu_usecs);

    printf("Creating threads\n");

    startTime = get_cpu_usecs();

    // Create threads
    err |= corethread_create(1, &threaded_schedule_work, (void *) &scheduleSet_1);
    err |= corethread_create(2, &threaded_schedule_work, (void *) &scheduleSet_2);

    if(err != 0){
        printf("Threads could not be created\nExiting...\n");
        return 1;
    }
    
    // Wait for threads to finish
    if(corethread_join(1, (void*) &numExecTasks[0]) != 0){
        printf("Thread 1 could not be joined");
        return 2;
    }
    
    if(corethread_join(2, (void*) &numExecTasks[1]) != 0){
        printf("Thread 2 could not be joined");
        return 3;
    }

    scheduleTime = get_cpu_usecs() - startTime;

    printf("Threads finished\n");

    printf("\nGathered Statistics\n");
    printf("--No. of hyper periods =\t%d\n", EXEC_HYPERPERIODS);
    printf("--Theoritic duration =\t%d μs\n", EXEC_HYPERPERIODS*HYPER_PERIOD);
    printf("--Total execution time =\t%llu μs\n", scheduleTime);
    printf("--Total no. of executed tasks =\t%ld\n", (numExecTasks[0]+numExecTasks[1]));
    printf("----Thread #1 no. of executed tasks =\t%ld\n", numExecTasks[0]);
    for(int i=0; i<2; i++){
        uint64_t avgDelta = scheduleSet_1.tasks[i].delta_sum/scheduleSet_1.tasks[i].exec_count;
        printf("--task[%d].period = %lld, executed with avg. dt = %llu (avg. jitter = %d) from a total of %lu executions\n", i, scheduleSet_1.tasks[i].period, 
        avgDelta, (int) scheduleSet_1.tasks[i].period - (int) avgDelta, scheduleSet_1.tasks[i].exec_count);
    }
    printf("----Thread #2 no. of executed tasks =\t%ld\n", numExecTasks[1]);
    for(int i=0; i<2; i++){
        uint64_t avgDelta = scheduleSet_2.tasks[i].delta_sum/scheduleSet_2.tasks[i].exec_count;
        printf("--task[%d].period = %lld, executed with avg. dt = %llu (avg. jitter = %d) from a total of %lu executions\n", i, scheduleSet_2.tasks[i].period, 
        avgDelta, (int) scheduleSet_2.tasks[i].period - (int) avgDelta, scheduleSet_2.tasks[i].exec_count);
    }

    return 0;
}