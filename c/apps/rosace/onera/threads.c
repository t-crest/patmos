#include "assemblage.h"
#include "assemblage_includes.h"
#include "threads.h"
#include "types.h"
#include <machine/patmos.h>
#include <machine/rtc.h>
#include <machine/spm.h>
#include <pthread.h>
#include <unistd.h> // For sleep()

// This should be set to 1 to run in "real-time" in the sense
// that the simulation time is close to the real world time
#define RUN_WITH_REAL_TIME 0
#define DEADLINE *((volatile _SPM unsigned int*)(PATMOS_IO_DEADLINE))

// Task set
struct nonencoded_task_params* tasks;

// I/O
output_t outs;
uint64_t step_simu;
uint64_t max_step_simu;

// Barriers for thread synchro
pthread_barrier_t cycle_start_b;
pthread_barrier_t engine_elevator_b;
pthread_barrier_t filter_b;
pthread_barrier_t control_b;
pthread_barrier_t output_b;

pthread_mutex_t common_l;

// Output variables
extern double aircraft_dynamics495_Va_Va_filter_100449_Va[2];
extern double aircraft_dynamics495_az_az_filter_100458_az[2];
extern double aircraft_dynamics495_Vz_Vz_filter_100452_Vz[2];
extern double aircraft_dynamics495_q_q_filter_100455_q[2];
extern double aircraft_dynamics495_h_h_filter_100446_h[2];
extern double Va_control_50474_delta_th_c_delta_th_c;
extern double Vz_control_50483_delta_e_c_delta_e_c;

void
copy_output_vars(output_t* v, uint64_t step)
{
  v->sig_outputs.Va = aircraft_dynamics495_Va_Va_filter_100449_Va[step % 2];
  v->sig_outputs.Vz = aircraft_dynamics495_Vz_Vz_filter_100452_Vz[step % 2];
  v->sig_outputs.q = aircraft_dynamics495_q_q_filter_100455_q[step % 2];
  v->sig_outputs.az = aircraft_dynamics495_az_az_filter_100458_az[step % 2];
  v->sig_outputs.h = aircraft_dynamics495_h_h_filter_100446_h[step % 2];
  v->sig_delta_th_c = Va_control_50474_delta_th_c_delta_th_c;
  v->sig_delta_e_c = Vz_control_50483_delta_e_c_delta_e_c;
}

void
rosace_init()
{
  // Init barriers
  pthread_barrier_init(&cycle_start_b, NULL, 5);
  pthread_barrier_init(&engine_elevator_b, NULL, 2);
  pthread_barrier_init(&filter_b, NULL, 5);
  pthread_barrier_init(&control_b, NULL, 2);
  pthread_barrier_init(&output_b, NULL, 2);

  // Initial values
  outs.sig_outputs.Va = 0;
  outs.sig_outputs.Vz = 0;
  outs.sig_outputs.q = 0;
  outs.sig_outputs.az = 0;
  outs.sig_outputs.h = 0;
  outs.t_simu = 0;
  step_simu = 0;

  // Get the task set (required for CALL() macro)
  int tmp;
  get_task_set(&tmp, &tasks);
}

#define CALL(val) tasks[(val)].ne_t_body(NULL)

void*
thread1(void* arg)
{
  uint64_t mystep_simu = step_simu;
  while (step_simu < max_step_simu) {
    pthread_barrier_wait(&cycle_start_b);

    // --- 200 Hz ---
    CALL(ENGINE);
    pthread_barrier_wait(&engine_elevator_b);
    // --- End 200 Hz ---

    // --- 100 Hz ---
    if (mystep_simu % 2 == 0) {
      pthread_barrier_wait(&filter_b);
      CALL(VZ_FILTER);
    }
    // --- End 100 Hz ---

    // --- 10 Hz ---
    if (mystep_simu % 20 == 0)
      CALL(VA_C0);
    // --- End 10 Hz ---

    // --- 50 Hz ---
    if (mystep_simu % 4 == 0) {
      pthread_barrier_wait(&control_b);
      CALL(VA_CONTROL);
    }
    if (mystep_simu % 4 == 3) {
      pthread_barrier_wait(&output_b);
      CALL(DELTA_TH_C0);
    }
    // --- End 50 Hz ---

    pthread_mutex_lock(&common_l);
    step_simu = step_simu + 1;
    pthread_mutex_unlock(&common_l);
    outs.t_simu += 5;

    // Print output
    copy_output_vars(&outs, step_simu);
    if (step_simu % 10) {
      // pthread_mutex_lock(&common_l);
      ROSACE_write_outputs(&outs);
      // pthread_mutex_unlock(&common_l);
    }

    if (RUN_WITH_REAL_TIME) {
      DEADLINE = 400000;
      unsigned int var = DEADLINE;
      // usleep(5000); // "Real-time" execution
    }

    mystep_simu++;
  }
  return NULL;
}

void*
thread2(void* arg)
{
  uint64_t mystep_simu = step_simu;
  while (step_simu < max_step_simu) {
    pthread_barrier_wait(&cycle_start_b);

    // --- 200 Hz ---
    CALL(ELEVATOR);
    pthread_barrier_wait(&engine_elevator_b);

    CALL(AIRCRAFT_DYN);
    // --- End 200 Hz ---

    // --- 100 Hz ---
    if (mystep_simu % 2 == 0) {
      pthread_barrier_wait(&filter_b);
      CALL(H_FILTER);
    }
    // --- End 100 Hz ---

    // --- 10 Hz ---
    if (mystep_simu % 20 == 0)
      CALL(H_C0);
    // --- End 10 Hz ---

    // --- 50 Hz ---
    if (mystep_simu % 4 == 0) {
      CALL(ALTI_HOLD);
      pthread_barrier_wait(&control_b);
      CALL(VZ_CONTROL);
    }
    if (mystep_simu % 4 == 3) {
      pthread_barrier_wait(&output_b);
      CALL(DELTA_E_C0);
    }
    // --- End 50 Hz ---

    mystep_simu++;
  }
  return NULL;
}

void*
thread3(void* arg)
{
  uint64_t mystep_simu = step_simu;
  while (step_simu < max_step_simu) {
    pthread_barrier_wait(&cycle_start_b);
    // --- 100 Hz ---
    if (mystep_simu % 2 == 0) {
      pthread_barrier_wait(&filter_b);
      CALL(Q_FILTER);
    }
    // --- End 100 Hz ---
    mystep_simu++;
  }
  return NULL;
}

void*
thread4(void* arg)
{
  uint64_t mystep_simu = step_simu;
  while (step_simu < max_step_simu) {
    pthread_barrier_wait(&cycle_start_b);
    // --- 100 Hz ---
    if (mystep_simu % 2 == 0) {
      pthread_barrier_wait(&filter_b);
      CALL(VA_FILTER);
    }
    // --- End 100 Hz ---
    mystep_simu++;
  }
  return NULL;
}

void*
thread5(void* arg)
{
  uint64_t mystep_simu = step_simu;
  while (step_simu < max_step_simu) {
    pthread_barrier_wait(&cycle_start_b);
    // --- 100 Hz ---
    if (mystep_simu % 2 == 0) {
      pthread_barrier_wait(&filter_b);
      CALL(AZ_FILTER);
    }
    // --- End 100 Hz ---
    mystep_simu++;
  }
  return NULL;
}

int
run_rosace(uint64_t nbstep)
{
  LED = 0x1FF;
  int i;

  // Variables for thread management
  void* fcts[] = { &thread1, &thread2, &thread3, &thread4, &thread5 };
  pthread_t threads[5];

  pthread_mutex_init(&common_l, NULL);

  printf("Initializing ROSACE...\n");
  rosace_init();
  max_step_simu = nbstep;

  // Set first command
  ROSACE_update_altitude_command(10000.0);

  // Create the 5 threads
  printf("Spawning threads...\n");
  for (i = 0; i < 5; i++) {
    if (pthread_create(&(threads[i]), NULL, fcts[i], NULL) != 0) {
      printf("Error creating thread %d\n", i);
      return -1;
    }
    LED = i;
  }

  // Scenario: sleep 50 seconds before climbing
  while (1) {
    pthread_mutex_lock(&common_l);
    if (step_simu >= 10000) {
      pthread_mutex_unlock(&common_l);
      break;
    }
    pthread_mutex_unlock(&common_l);
  }
  ROSACE_update_altitude_command(11000.0);

  // Exit
  printf("Joining threads...");
  for (i = 0; i < 5; i++) {
    pthread_join(threads[i], NULL);
    LED--;
  }
  return 0;
}
