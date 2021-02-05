#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <machine/rtc.h>
#include "rm_minimal_scheduler.h"

#define SEC_TO_MS 1000
#define SEC_TO_US 1000000
#define SEC_TO_NS 1000000000.0
#define MS_TO_US 1000
#define MS_TO_NS 1000000
#define US_TO_NS 1000
#define CPU_PERIOD 12.5

#define NS_TO_SEC 1.0/SEC_TO_NS
#define NT_TO_MS 1.0/MS_TO_NS
#define NS_TO_US 1.0/US_TO_NS

#define HYPER_ITERATIONS 5
#define RUN_INFINITE false

#define LED (*((volatile _IODEV unsigned *)PATMOS_IO_LED))
#define DEAD (*((volatile _IODEV int *) PATMOS_IO_DEADLINE))

typedef struct 
{
    MinimalRMTask task;
    schedtime_t* activations;
    uint16_t hyper_acts_num;
} MinimalRMTaskReleaseLog;

typedef struct
{
    MinimalRMTaskReleaseLog *tasks_act_log;
    uint32_t num_of_tasks;
} MinimalRMScheduleActLog;

static MinimalRMScheduleActLog schedule_log;

void demo_task(const void *self)
{
    if(get_cpuid() == 0) LED = ((MinimalRMTask*) self)->id;
#ifdef DEBUG
    printf("@ {t_%u, #%lu, rt_nxt = %llu}\n", ((MinimalRMTask*) self)->id, ((MinimalRMTask*) self)->exec_count, ((MinimalRMTask*) self)->release_time);
#else
    // Fake work
    DEAD = ((MinimalRMTask*) self)->wcet - 1010;   //clock cycles
    int val = DEAD;
#endif
    // Logging
    #pragma loopbound min 3 max 3
    for(int t=0; t<schedule_log.num_of_tasks; t++){
        if(schedule_log.tasks_act_log[t].task.id == ((MinimalRMTask*) self)->id){
            if (((MinimalRMTask*) self)->exec_count <= schedule_log.tasks_act_log[t].hyper_acts_num){
                schedule_log.tasks_act_log[t].activations[((MinimalRMTask*) self)->exec_count] = ((MinimalRMTask*) self)->release_time - ((MinimalRMTask*) self)->period;
            }
            break;
        }
    }
    if(get_cpuid() == 0) LED = 0x0;
}

void create_taskset_basic(MinimalRMSchedule *schedule)
{
    MinimalRMTask taskSet[3];

    printf("Initializing tasks...\n");
    init_minimal_rmtask(&taskSet[0], 3, 4000, 4000, (1000 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[1], 1, 8000, 8000, (4000 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[2], 2, 16000, 16000, (3000 * US_TO_NS) / CPU_PERIOD, 0, demo_task);

    // Enqueue tasks to scheduler
    for(int i=0; i<3; i++){
        rmschedule_enqueue(schedule, taskSet[i]);
    }
    printf("Calculating hyper-period...\n");
    schedule->hyper_period = calc_hyperperiod(schedule, MS_TO_US);
    
    // Logging init
    schedule_log.num_of_tasks = schedule->task_count;
    schedule_log.tasks_act_log = (MinimalRMTaskReleaseLog *) malloc(schedule->task_count * sizeof(MinimalRMTaskReleaseLog)); 
    for(int i=0; i<schedule->task_count; i++){
        MinimalRMTask task_i =  taskSet[i];
        schedule_log.tasks_act_log[i].task = task_i;
        schedule_log.tasks_act_log[i].hyper_acts_num = schedule->hyper_period / task_i.period;
        schedule_log.tasks_act_log[i].activations = (schedtime_t*) malloc(schedule_log.tasks_act_log[i].hyper_acts_num * sizeof(schedtime_t));
        for(int r=0; r<schedule_log.tasks_act_log[i].hyper_acts_num; r++){
            schedule_log.tasks_act_log[i].activations[r] = 0;
        }
    }
}

void create_taskset_table_9_2(MinimalRMSchedule *schedule){
    MinimalRMTask taskSet[10];

    // Tasks according to book use-case Table 9.2
    printf("Initializing tasks...\n");
    init_minimal_rmtask(&taskSet[0], 1, 4000, 1000, (992 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[1], 2, 4000, 4000, (221 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[2], 3, 4000, 4000, (496 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[3], 4, 4000, 4000, (249 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[4], 5, 4000, 4000, (218 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[5], 6, 4000, 4000, (348 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[6], 7, 20000, 10000, (1430 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[7], 8, 100000, 50000, (2220 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[8], 9, 200000, 200000, (1950 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[9], 10, 200000, 200000, (2060 * US_TO_NS) / CPU_PERIOD, 0, demo_task);

    // Enqueue tasks to scheduler
    for(int i=0; i<10; i++){
        rmschedule_sortedinsert_period(schedule, create_rmtasknode(taskSet[i]));
    }
    printf("Calculating hyper-period...\n");
    schedule->hyper_period = calc_hyperperiod(schedule, MS_TO_US);

    // Logging init
    schedule_log.num_of_tasks = schedule->task_count;
    schedule_log.tasks_act_log = (MinimalRMTaskReleaseLog *) malloc(schedule->task_count * sizeof(MinimalRMTaskReleaseLog)); 
    for(int i=0; i<schedule->task_count; i++){
        MinimalRMTask task_i =  taskSet[i];
        schedule_log.tasks_act_log[i].task = task_i;
        schedule_log.tasks_act_log[i].hyper_acts_num = schedule->hyper_period / task_i.period;
        schedule_log.tasks_act_log[i].activations = (schedtime_t*) malloc(schedule_log.tasks_act_log[i].hyper_acts_num * sizeof(schedtime_t));
        for(int r=0; r<schedule_log.tasks_act_log[i].hyper_acts_num; r++){
            schedule_log.tasks_act_log[i].activations[r] = 0;
        }
    }
}

void create_taskset_table_9_6(MinimalRMSchedule *schedule){
    MinimalRMTask taskSet[7];

    // Tasks according book Scheduling in real-time systems use-case Table 9.6
    printf("Initializing tasks...\n");
    init_minimal_rmtask(&taskSet[0], 1, 10000, 10000, (2000 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[1], 2, 20000, 20000, (2000 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[2], 3, 100000, 100000, (2000 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[3], 4, 15000, 15000, (2000 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[4], 5, 14000, 14000, (2000 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[5], 6, 50000, 50000, (2000 * US_TO_NS) / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[6], 7, 40000, 40000, (2000 * US_TO_NS) / CPU_PERIOD, 0, demo_task);

    // Enqueue tasks to scheduler
    for(int i=0; i<7; i++){
        rmschedule_sortedinsert_period(schedule, create_rmtasknode(taskSet[i]));
    }
    printf("Calculating hyper-period...\n");
    schedule->hyper_period = calc_hyperperiod(schedule, MS_TO_US);

    // Logging init
    schedule_log.num_of_tasks = schedule->task_count;
    schedule_log.tasks_act_log = (MinimalRMTaskReleaseLog *) malloc(schedule->task_count * sizeof(MinimalRMTaskReleaseLog)); 
    for(int i=0; i<schedule->task_count; i++){
        MinimalRMTask task_i =  taskSet[i];
        schedule_log.tasks_act_log[i].task = task_i;
        schedule_log.tasks_act_log[i].hyper_acts_num = schedule->hyper_period / task_i.period;
        schedule_log.tasks_act_log[i].activations = (schedtime_t*) malloc(schedule_log.tasks_act_log[i].hyper_acts_num * sizeof(schedtime_t));
        for(int r=0; r<schedule_log.tasks_act_log[i].hyper_acts_num; r++){
            schedule_log.tasks_act_log[i].activations[r] = 0;
        }
    }
}

void create_taskset_table_tttasks(MinimalRMSchedule *schedule){
    MinimalRMTask taskSet[8];

    // Tasks according to book use-case Table 9.2
    printf("Initializing tasks...\n");
    init_minimal_rmtask(&taskSet[0], 0, 5000, 1000, 5000 / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[1], 1, 10000, 4000, 10000 / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[2], 2, 2500, 4000, 2500 / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[3], 3, 50000, 4000, 50000 / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[4], 4, 5000, 4000, 5000 / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[5], 5, 10000, 4000, 10000 / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[6], 6, 2500, 10000, 2500 / CPU_PERIOD, 0, demo_task);
    init_minimal_rmtask(&taskSet[7], 7, 50000, 50000, 50000 / CPU_PERIOD, 0, demo_task);

    // Enqueue tasks to scheduler
    schedule->get_time = &get_cpu_usecs;
    schedule->head = NULL;
    schedule->tail = NULL;
    for(int i=0; i<8; i++){
        rmschedule_sortedinsert_period(schedule, create_rmtasknode(taskSet[i]));
    }
    printf("Calculating hyper-period...\n");
    schedule->hyper_period = 50000;
}

int main()
{
    LED = 0x1FF;
    printf("\nPatmos Rate-Monotonic Scheduler Demo\n");
    
    uint16_t numExecTasks;
    uint64_t scheduleTime;
    uint64_t startTime;
    MinimalRMSchedule schedule = {
        .get_time = &get_cpu_usecs,
        .head = NULL,
        .tail = NULL,
        .task_count = 0
    };

    // create_taskset_basic(&schedule);
    // create_taskset_table_9_2(&schedule);
    // create_taskset_table_9_6(&schedule);
    create_taskset_table_tttasks(&schedule);

    LED = 0xF0;

    // Execute
    printf("Task scheduler started @ %llu μs, task count = %lu, hyper-period = %llu μs\n", schedule.get_time(), schedule.task_count, schedule.hyper_period);
    schedule.start_time = schedule.get_time();
    do {
        numExecTasks += minimal_rm_scheduler(&schedule);
    } while (schedule.get_time() - schedule.start_time <= HYPER_ITERATIONS * schedule.hyper_period);

    // Report
    LED = 0xFF;
    printf("\nGathered Statistics...\n");
    printf("-- No. of hyper period iterations = %u\n", HYPER_ITERATIONS);
    printf("-- Theoritic duration = %llu μs\n", (uint64_t) HYPER_ITERATIONS * schedule.hyper_period);
    printf("-- Total execution time = %llu μs\n", endTime - schedule.start_time);
    printf("-- Total no. of executed tasks = %d\n", numExecTasks);
    MinimalRMTaskNode* itr_task = schedule.head;
    while(itr_task != NULL){
        double avgDelta = (double) (itr_task->task.delta_sum/ (double)itr_task->task.exec_count);
        printf("-- Task %d  with period = %lld, executed with avg. jitter = %.3f from a total of %lu executions (%hu overruns)\n", itr_task->task.id, itr_task->task.period, 
        (double) itr_task->task.period - avgDelta, itr_task->task.exec_count, itr_task->task.overruns);
        itr_task = itr_task->next;
    }
    printf("\nHyper-period activations log:\n");
    for(int i=0; i<schedule.task_count; i++)
    {
        printf("Task %d [%hu] \n{", schedule_log.tasks_act_log[i].task.id, schedule_log.tasks_act_log[i].hyper_acts_num);
        for(int r=0; r<schedule_log.tasks_act_log[i].hyper_acts_num; r++)
        {
            printf("\t%llu),", schedule_log.tasks_act_log[i].activations[r]);
        }
        printf("\n}\n");
    }

    LED = 0x0;
    return 0;
}