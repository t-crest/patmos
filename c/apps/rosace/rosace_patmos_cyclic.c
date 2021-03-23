#include <stdlib.h>
#include <inttypes.h>
#include <math.h> //just for fabs
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include "helpers/printf.h"
#include "onera/io.h"
#include "onera/assemblage_includes.h"
#include "onera/assemblage.h"
#include "schedules/rosace_tasks_schedule.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define LEDS *((volatile _SPM unsigned int *) (PATMOS_IO_LED))   
#define KEYS *((volatile _SPM unsigned int *) (PATMOS_IO_KEYS))
#define GPIO *((volatile _SPM unsigned int *) (PATMOS_IO_GPIO))
#define SEGDISP *((volatile _SPM unsigned int *) (PATMOS_IO_SEGDISP))
#define DEADLINE *((volatile _SPM unsigned int *) (PATMOS_IO_DEADLINE))
#define CALL(val)   tasks[(val)].ne_t_body(NULL)

#define MAX_STEP_SIM 30000
#define STEP_TIME_SCALE 20  //ms

#define schedtime_t uint64_t

// Task set
struct nonencoded_task_params* tasks;
typedef int (*generic_task_fp)(void *);
typedef struct
{
	unsigned short id;
	schedtime_t period;
	schedtime_t *release_times;
	unsigned long release_inst;
	unsigned long nr_releases;
	schedtime_t last_time;
	schedtime_t delta_sum;
	unsigned long exec_count;
	generic_task_fp task_fun;
} MinimalTTTask;
typedef void (*task_sync_fp)(unsigned long long start_time, unsigned long long current_time, MinimalTTTask* tasks);

struct nonencoded_task_params* tasks;
int num_of_tasks;
schedtime_t hyper_period;
MinimalTTTask *schedule;

// Output variables
extern double aircraft_dynamics495_Va_Va_filter_100449_Va[2];
extern double aircraft_dynamics495_az_az_filter_100458_az[2];
extern double aircraft_dynamics495_Vz_Vz_filter_100452_Vz[2];
extern double aircraft_dynamics495_q_q_filter_100455_q[2];
extern double aircraft_dynamics495_h_h_filter_100446_h[2];
extern double Va_control_50474_delta_th_c_delta_th_c;
extern double Vz_control_50483_delta_e_c_delta_e_c;

// I/O
output_t outs;
uint64_t step_simu;
uint64_t max_step_simu;

// Enable FPU
unsigned __USE_HWFPU__ = 0;

void printSegmentInt(unsigned number) 
{
    *(&SEGDISP+0) = number & 0xF;
    *(&SEGDISP+1) = (number >> 4) & 0xF;
    *(&SEGDISP+2) = (number >> 8) & 0xF;
    *(&SEGDISP+3) = (number >> 12) & 0xF;
    *(&SEGDISP+4) = (number >> 16) & 0xF;
    *(&SEGDISP+5) = (number >> 20) & 0xF;
    *(&SEGDISP+6) = (number >> 24) & 0xF;
    *(&SEGDISP+7) = (number >> 28) & 0xF;
}

__attribute__((noinline))
int logging_fun(void *args)
{
  outs.t_simu = step_simu * STEP_TIME_SCALE;
  outs.sig_outputs.Va = aircraft_dynamics495_Va_Va_filter_100449_Va[step_simu % 2];
  outs.sig_outputs.Vz = aircraft_dynamics495_Vz_Vz_filter_100452_Vz[step_simu % 2];
  outs.sig_outputs.q = aircraft_dynamics495_q_q_filter_100455_q[step_simu % 2];
  outs.sig_outputs.az = aircraft_dynamics495_az_az_filter_100458_az[step_simu % 2];
  outs.sig_outputs.h = aircraft_dynamics495_h_h_filter_100446_h[step_simu % 2];
  outs.sig_delta_th_c = Va_control_50474_delta_th_c_delta_th_c;
  outs.sig_delta_e_c = Vz_control_50483_delta_e_c_delta_e_c;

  printf("%3.3f,%5.3f,%5.3f,%5.4f,%5.3f,%5.3f,%5.4f,%5.4f\n", 
  outs.t_simu/1000.0f, outs.sig_outputs.Va, outs.sig_outputs.az, 
  outs.sig_outputs.q, outs.sig_outputs.Vz, outs.sig_outputs.h,
  outs.sig_delta_th_c,outs.sig_delta_e_c);
  step_simu += 1;

  return 1;
}

__attribute__((noinline))
void initialize_schedule(MinimalTTTask *schedule)
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
  printf("\nT,Va,az,q,Vz,h,delta_th_c,delta_e_c\n");
  schedtime_t start_time = get_cpu_usecs();
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
    else
    {
      ROSACE_update_altitude_command(10000);
    }
	}
  return get_cpu_usecs();
}

int main()
{
  MinimalTTTask schedule[NUM_OF_TASKS];
  LEDS = 0x1FF;
  printf("\nWelcome to ROSACE cyclic execution\n");
  initialize_schedule((MinimalTTTask*) schedule);
  LEDS = 0x000;
  printf("\nRosace started @ %llu us (max_sim_time = %lu us)\n", get_cpu_usecs(), max_step_simu * STEP_TIME_SCALE * 1000);
  schedtime_t end_time = execute_cyclic_loop((MinimalTTTask*) schedule);
  printf("Rosace ended @ %llu us\n", end_time);
	printf("---------------------------------------------------------------------------\n");
	printf("Task log:\n");
	for(int task=0; task<NUM_OF_TASKS; task++){
		float avgDelta = (float) schedule[task].delta_sum/ (float) schedule[task].exec_count;
		printf("--task[%12s] avg. dt = %5.3f us (avg. jitter = %5.3f us) from a total of %lu executions\n", 
          tasks_names[task], avgDelta, fabs(schedule[task].period - avgDelta), schedule[task].exec_count);
	}
	printf("---------------------------------------------------------------------------\n");
  LEDS = 0x000;
  return 0;
}