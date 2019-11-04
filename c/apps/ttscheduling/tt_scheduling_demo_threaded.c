#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <machine/rtc.h>
#include "libcorethread/corethread.h"
#include "tt_minimal_scheduler.h"
#include "demo_tasks.h"
#include <unistd.h>
#include <string.h>
#include <math.h>

#define HYPER_PERIOD 100000
#define NUM_OF_TASKS 5
#define EXEC_HYPERPERIODS 2
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
    int numExecTasks[2];
    unsigned long long scheduleTime;
    unsigned long long startTime;

    // Define set of tasks
    MinimalTTTask taskSet_1[3];
    init_minimal_tttask(&taskSet_1[0], 20000, 1695, &task_act);
    init_minimal_tttask(&taskSet_1[1], 5000, 4699, &task_rcv);
    init_minimal_tttask(&taskSet_1[2], 10000, 6476, &task_snd);
    MinimalTTSchedule scheduleSet_1 = init_minimal_ttschedule(HYPER_PERIOD, 3, taskSet_1, &get_cpu_usecs);

    MinimalTTTask taskSet_2[2];
    init_minimal_tttask(&taskSet_2[0], 100000, 6715, &task_mon);
    init_minimal_tttask(&taskSet_2[1], 4000, 0, &task_syn);
    MinimalTTSchedule scheduleSet_2 = init_minimal_ttschedule (HYPER_PERIOD, 2, taskSet_2, &get_cpu_usecs);

    maxJitter = 0;
    sumJitter = 0;

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
    printf("--Total no. of executed tasks =\t%d\n", (numExecTasks[0]+numExecTasks[1]));
    printf("----Thread #1 no. of executed tasks =\t%d\n", numExecTasks[0]);
    printf("----Thread #2 no. of executed tasks =\t%d\n", numExecTasks[1]);
    printf("--Max. Jitter =\t%+lld μs\n", maxJitter);
    printf("--Avg. Jitter =\t%+f μs\n", (double) sumJitter/(numExecTasks[0]+numExecTasks[1]));

    return 0;
}