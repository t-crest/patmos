#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <machine/rtc.h>
#include "tt_minimal_scheduler.h"
#include "demo_tasks.h"
#include "schedule.h"

#define SET_TO_MS 1000
#define SEC_TO_US 1000000
#define SEC_TO_NS 1000000000.0
#define MS_TO_US 1000
#define MS_TO_NS 1000000
#define US_TO_NS 1000

#define NS_TO_SEC 1.0/SEC_TO_NS
#define NT_TO_MS 1.0/MS_TO_NS
#define NS_TO_US 1.0/US_TO_NS

#define CLKS_TO_US CPU_PERIOD * NS_TO_US 
#define US_TO_CLKS (US_TO_NS/CPU_PERIOD)

#define CPU_PERIOD 12.5
#define WCET_DISPATCHER 2046

#define HYPER_ITERATIONS 100
#define RUN_INFINITE false

void convert_sched_to_timebase(uint64_t *sched_insts, uint32_t nr_insts, double timebase){
    for(int i=0; i<nr_insts; i++){
        sched_insts[i] = (uint64_t) (sched_insts[i] * timebase);
    }
}

int main()
{
    LED = 0x1FF;
    printf("\nPatmos Time-Triggered Executive Demo\n");
    
    uint16_t numExecTasks;
    uint64_t startTime, endTime;
    MinimalTTTask taskSet[NUM_OF_TASKS];
    MinimalTTSchedule schedule;

    static void (*tasks_func_ptrs[NUM_OF_TASKS])(const void*) = {task_1, task_2, task_3, task_4, task_5, task_6, task_7, task_8};

    // Tasks are defined in a set with the activation times according to the schedule generation
    for(unsigned int i=0; i<NUM_OF_TASKS; i++){
        convert_sched_to_timebase(tasks_schedules[i], tasks_insts_counts[i], NS_TO_US);
        init_minimal_tttask(&taskSet[i], i, (uint64_t)(tasks_periods[i] * NS_TO_US), tasks_schedules[i], tasks_insts_counts[i], tasks_func_ptrs[i]);
    }

    // Enqueue tasks to scheduler
    schedule.get_time = &get_cpu_usecs;
    schedule.head = NULL;
    schedule.tail = NULL;
    schedule.hyper_period = HYPER_PERIOD * NS_TO_US;
    for(unsigned int i=0; i<NUM_OF_TASKS; i++){
        ttschedule_sortedinsert_period(&schedule, create_tttasknode(taskSet[i]));
    }

    LED = 0xF0;

    //Execute
    printf("\nSchedule start_time(us)=%llu\n", get_cpu_usecs());
    startTime = get_cpu_usecs();
    numExecTasks = tt_minimal_schedule_loop(&schedule, HYPER_ITERATIONS, RUN_INFINITE);
    endTime = get_cpu_usecs();

    // Report
    LED = 0xFF;
    printf("\nGathered Statistics\n");
    printf("--No. of hyper period iterations = %u\n", HYPER_ITERATIONS);
    printf("--Theoritic duration = %llu μs\n", (uint64_t) HYPER_ITERATIONS * schedule.hyper_period);
    printf("--Total execution time = %llu μs\n", endTime - startTime);
    printf("--Total no. of executed tasks = %d\n", numExecTasks);
    MinimalTTTaskNode* itr_node = schedule.head;
    unsigned i =0 ;
    while(itr_node != NULL){
        double avgDelta = (double) (itr_node->task.delta_sum/ (double)itr_node->task.exec_count);
        printf("-- task[%d].period = %lld, executed with avg. dt = %.3f (jitter = %.3f) from a total of %lu executions\n", itr_node->task.id, itr_node->task.period, 
        avgDelta, (double) itr_node->task.period - avgDelta, itr_node->task.exec_count);
        itr_node = itr_node->next;
        i++;
    }
    LED = 0x0;
    return 0;
}