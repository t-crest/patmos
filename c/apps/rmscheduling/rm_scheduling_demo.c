#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <machine/rtc.h>
#include "rm_minimal_scheduler.h"
#include "demo_tasks.h"

#define HYPER_PERIOD 4200000
#define HYPER_ITERATIONS 2
#define RUN_INFINITE false
#define NUM_OF_TASKS 7

void convert_sched_to_timebase(uint64_t *sched_insts, uint32_t nr_insts, double timebase){
    for(int i=0; i<nr_insts; i++){
        sched_insts[i] = (uint64_t) (sched_insts[i] * timebase);
    }
}

int main()
{
    LED = 0;
    printf("\nPatmos Rate-Monotonic Scheduler Demo\n");
    
    uint16_t numExecTasks;
    uint64_t scheduleTime;
    uint64_t startTime;
    MinimalRMSchedule schedule;
    MinimalRMTask taskSet[NUM_OF_TASKS];

    // Tasks according to the automotive example in book Scheduling in real-time systems Section 9.3
    init_minimal_rmtask(&taskSet[0], 1, 10000, 10000, (2000 * US_TO_NS) / CPU_PERIOD, 0, task_1);
    init_minimal_rmtask(&taskSet[1], 2, 20000, 20000, (2000 * US_TO_NS) / CPU_PERIOD, 0, task_2);
    init_minimal_rmtask(&taskSet[2], 3, 100000, 100000, (2000 * US_TO_NS) / CPU_PERIOD, 0, task_3);
    init_minimal_rmtask(&taskSet[3], 4, 15000, 15000, (2000 * US_TO_NS) / CPU_PERIOD, 0, task_4);
    init_minimal_rmtask(&taskSet[4], 5, 14000, 14000, (2000 * US_TO_NS) / CPU_PERIOD, 0, task_5);
    init_minimal_rmtask(&taskSet[5], 6, 50000, 50000, (2000 * US_TO_NS) / CPU_PERIOD, 0, task_6);
    init_minimal_rmtask(&taskSet[6], 7, 40000, 40000, (2000 * US_TO_NS) / CPU_PERIOD, 0, task_7);

    // Enqueue tasks to scheduler
    schedule = init_minimal_rmschedule((uint64_t)(HYPER_PERIOD), NUM_OF_TASKS, &get_cpu_usecs);
    for(int i=0; i<NUM_OF_TASKS; i++){
        rmschedule_enqueue(&schedule, taskSet[i]);
    }

    // Execute
    printf("\nTask scheduler started @ %llu us\n\n", schedule.get_time());
    schedule.start_time = schedule.get_time();
    while (schedule.get_time() - schedule.start_time <= HYPER_ITERATIONS * schedule.hyper_period){
        numExecTasks += minimal_rm_scheduler(&schedule);
    }

    // Report
    printf("\nGathered Statistics...\n");
    printf("-- No. of hyper period iterations = %u\n", HYPER_ITERATIONS);
    printf("-- Theoritic duration = %llu μs\n", (uint64_t) HYPER_ITERATIONS * schedule.hyper_period);
    printf("-- Total execution time = %llu μs\n", schedule.get_time() - schedule.start_time);
    printf("-- Total no. of executed tasks = %d\n", numExecTasks);
    MinimalRMTaskNode* itr_task = schedule.head;
    for(int i=0; i<NUM_OF_TASKS; i++){
        uint64_t avgDelta = (uint64_t) (itr_task->task.delta_sum/ (uint64_t)itr_task->task.exec_count);
        printf("-- task[%d].period = %lld, executed with avg. dt = %llu (jitter = %d) from a total of %lu executions with %hu overruns\n", itr_task->task.id, itr_task->task.period, 
        avgDelta, (int) itr_task->task.period - (int) avgDelta, itr_task->task.exec_count, itr_task->task.overruns);
        itr_task = itr_task->next;
    }

    return 0;
}