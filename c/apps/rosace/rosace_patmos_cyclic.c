#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h> //just for fabs
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include "rosace_dist_common.h"
#include "rosace_tasks_schedule.h"

__attribute__((noinline))
void rosace_init(MinimalTTTask *schedule)
{
	// Initial values
	outs.sig_outputs.Va = 0;
	outs.sig_outputs.Vz = 0;
	outs.sig_outputs.q  = 0;
	outs.sig_outputs.az = 0;
	outs.sig_outputs.h  = 0;
	outs.t_simu         = 0;
	step_simu           = 0;
  max_step_simu       = MAX_STEP_SIM;
  get_task_set(&num_of_tasks, &tasks);
  hyper_period = HYPER_PERIOD;
  num_of_tasks = NUM_OF_TASKS;
  #pragma loopbound min 6 max 16
  for(int i=0; i<NUM_OF_TASKS; i++)
  {
    schedule[i].period = (schedtime_t) tasks_periods[i];
    schedule[i].nr_releases = tasks_insts_counts[i];
    schedule[i].release_times = tasks_schedules[i];
    schedule[i].last_time = 0;
    schedule[i].delta_sum = 0;
    schedule[i].exec_count = 0;
    schedule[i].release_inst = 0;
    switch (i)
    {
    case LOGGING_ID:
      schedule[i].task_fun = &logging_fun;
      break;
    case ENGINE_ID:
      schedule[i].task_fun = tasks[ENGINE].ne_t_body;
      break;
    case ELEVATOR_ID:
      schedule[i].task_fun = tasks[ELEVATOR].ne_t_body;
      break;
    case AIRCRAFT_DYN_ID:
      schedule[i].task_fun = tasks[AIRCRAFT_DYN].ne_t_body;
      break;
    case H_C0_ID:
      schedule[i].task_fun = tasks[H_C0].ne_t_body;
      break;
    case VA_C0_ID:
      schedule[i].task_fun = tasks[H_C0].ne_t_body;
      break;
    case H_FILTER_ID:
      schedule[i].task_fun = tasks[H_FILTER].ne_t_body;
      break;
    case Q_FILTER_ID:
      schedule[i].task_fun = tasks[Q_FILTER].ne_t_body;
      break;
    case VZ_FILTER_ID:
      schedule[i].task_fun = tasks[VZ_FILTER].ne_t_body;
      break;
    case AZ_FILTER_ID:
      schedule[i].task_fun = tasks[AZ_FILTER].ne_t_body;
      break;
    case VA_FILTER_ID:
      schedule[i].task_fun = tasks[VA_FILTER].ne_t_body;
      break;
    case VZ_CONTROL_ID:
      schedule[i].task_fun = tasks[VZ_CONTROL].ne_t_body;
      break;
    case VA_CONTROL_ID:
      schedule[i].task_fun = tasks[VA_CONTROL].ne_t_body;
      break;
    case DELTA_TH_C0_ID:
      schedule[i].task_fun = tasks[DELTA_TH_C0].ne_t_body;
      break;
    case DELTA_E_C0_ID:
      schedule[i].task_fun = tasks[DELTA_E_C0].ne_t_body;
      break;
    case ALTI_HOLD_ID:
      schedule[i].task_fun = tasks[ALTI_HOLD].ne_t_body;
      break;
    }
    LEDS = i;
  }
}

__attribute__((noinline))
schedtime_t execute_cyclic_loop(MinimalTTTask *schedule)
{
  ROSACE_update_altitude_command(10000);
  puts("\nT,Va,az,q,Vz,h,delta_th_c,delta_e_c");
  start_time = get_cpu_usecs();
  #pragma loopbound min 1 max 1
	while(step_simu < max_step_simu)
  {
    register schedtime_t current_time = get_cpu_usecs() - start_time;
    #pragma loopbound min 16 max 16
    for(int task=0; task < num_of_tasks; task++)
    {
      LEDS = task;
      if(current_time >= schedule[task].release_times[schedule[task].release_inst])
      {
        schedule[task].task_fun(NULL);
        schedule[task].release_times[schedule[task].release_inst] += hyper_period;
        schedule[task].release_inst = (schedule[task].release_inst + 1) % schedule[task].nr_releases;
        schedule[task].delta_sum += schedule[task].last_time == 0 ? 0 : (current_time - schedule[task].last_time);
        schedule[task].last_time = current_time;
        schedule[task].exec_count++;
      }
    }
    if(step_simu >= 2500)
    {
      ROSACE_update_altitude_command(11000);
    }
	}
  return get_cpu_usecs();
}

int main()
{
  // _SPM MinimalTTTask *schedule = (_SPM MinimalTTTask*) (LOCAL_SPM_BASE);
  MinimalTTTask *schedule = (MinimalTTTask*) malloc(NUM_OF_TASKS * sizeof(MinimalTTTask));
  LEDS = 0x1FF;
  printf("\nWelcome to ROSACE cyclic execution\n");
  printSegmentInt(0xABCD0123);
  rosace_init((MinimalTTTask*) schedule);
  LEDS = 0x000;
  printf("\nRosace started @ %llu us (max_sim_time = %llu us)\n", get_cpu_usecs(), max_step_simu * STEP_TIME_SCALE * 1000);
  schedtime_t end_time = execute_cyclic_loop((MinimalTTTask*) schedule);
  printf("Rosace ended @ %llu us (elapsed = %llu us)\n", end_time, end_time-start_time);
	puts("---------------------------------------------------------------------------");
	puts("Task log:");
	for(int task=0; task<NUM_OF_TASKS; task++){
		float avgDelta = (float) schedule[task].delta_sum/ (float) schedule[task].exec_count;
		printf("--task[%12s] avg. dt = %5.3f us (avg. jitter = %5.3f us) from a total of %lu executions\n", 
          tasks_names[task], avgDelta, fabs(schedule[task].period - avgDelta), schedule[task].exec_count);
	}
	puts("---------------------------------------------------------------------------");
  LEDS = 0x000;
  return 0;
}