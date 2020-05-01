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
#include "schedule.h"

#define NUM_OF_THREADS 4
#define TASKS_PER_THREAD (unsigned) (NUM_OF_TASKS/NUM_OF_THREADS)
#define HYPER_ITERATIONS 100
#define RUN_INFINITE false

void thread_worker(void *params)
{
    uint32_t executedTasks = 0;
    sort_asc_minimal_tttasks(((MinimalTTSchedule *)params)->tasks, ((MinimalTTSchedule *)params)->num_tasks);
    executedTasks = tt_minimal_schedule_loop((MinimalTTSchedule *)params, HYPER_ITERATIONS, RUN_INFINITE);
    corethread_exit((void*) executedTasks);
}

void convert_sched_to_timebase(uint64_t *sched_insts, uint32_t nr_insts, double timebase){
    for(int i=0; i<nr_insts; i++){
        sched_insts[i] = (uint64_t) (sched_insts[i] * timebase);
    }
}

int main()
{
    LED = 0;
    printf("\nPatmos Time-Triggered Executive Demo (Threaded)\n");
    
    uint32_t numExecTasks[TASKS_PER_THREAD];
    MinimalTTTask threadTaskSet[NUM_OF_THREADS][TASKS_PER_THREAD];
    MinimalTTSchedule threadSchedules[NUM_OF_THREADS];

    static void (*tasks_func_ptrs[NUM_OF_TASKS])(const void*) = {task_1, task_2, task_3, task_4, task_5, task_6, task_7, task_8};

    printf("\nAssigning tasks to threads (id,task): "); 
    // Define set of tasks
    for(unsigned t=0; t<NUM_OF_THREADS; t++)
    {
        for(unsigned int i=t*TASKS_PER_THREAD; i<(t+1)*TASKS_PER_THREAD; i++)
        {
            printf("(%d, %d)", t, i);
            convert_sched_to_timebase(tasks_schedules[i], tasks_insts_counts[i], NS_TO_US);
            init_minimal_tttask(&threadTaskSet[t][i-t*TASKS_PER_THREAD], (uint64_t)(tasks_periods[i] * NS_TO_US), 
                                tasks_schedules[i], tasks_insts_counts[i], tasks_func_ptrs[i]);
        }
        threadSchedules[t] = init_minimal_ttschedule((uint64_t) (HYPER_PERIOD * NS_TO_US), TASKS_PER_THREAD, threadTaskSet[t], &get_cpu_usecs);
    }

    printf("\n\nScheduled threads start_time(us)=%llu\n", get_cpu_usecs());

    uint64_t startTime = get_cpu_usecs();

    // Create threads
    for(unsigned t=0; t<NUM_OF_THREADS; t++){
        if(corethread_create(t+1, &thread_worker, (void *) &threadSchedules[t])){
            return 1;
        } else {
            LED += 1;
        }
    }
    
    // Wait for threads to finish
    for(unsigned t=0; t<NUM_OF_THREADS; t++){
        if(corethread_join(t+1, (void*) &numExecTasks[t]) != 0){
            return 1+t+1;
        } else {
            LED -= 1;
        }
    }

    uint64_t endTime = get_cpu_usecs() - startTime;

    printf("\nGathered Statistics\n");
    printf("--No. of hyper periods = %u\n", HYPER_ITERATIONS);
    printf("--Theoritic duration = %llu μs\n", HYPER_ITERATIONS * (uint64_t) (HYPER_PERIOD * NS_TO_US));
    printf("--Total execution time = %llu μs\n", endTime);
    for(unsigned t=0; t<NUM_OF_THREADS; t++)
    {
        printf("--Thread #%u no. of executed tasks = %ld\n", t, numExecTasks[t]);
        for(unsigned int i=0; i<TASKS_PER_THREAD; i++)
        {
            uint64_t avgDelta = threadSchedules[t].tasks[i].delta_sum/threadSchedules[t].tasks[i].exec_count;
            printf("----task[%d].period = %lld, executed with avg. dt = %llu (jitter = %d) from a total of %lu executions\n", i, 
            threadSchedules[t].tasks[i].period, avgDelta, (int) threadSchedules[t].tasks[i].period - (int) avgDelta, 
            threadSchedules[t].tasks[i].exec_count);
        }
    }

    return 0;
}