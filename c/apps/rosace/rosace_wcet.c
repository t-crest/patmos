#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include "threads.h"
#include "assemblage_includes.h"
#include "assemblage.h"

#define DEADLINE *((volatile _SPM unsigned int *) (PATMOS_IO_DEADLINE))

#define CALL(val)   tasks[(val)].ne_t_body(NULL)

// Task set
struct nonencoded_task_params* tasks;
int num_of_tasks;

// I/O
output_t outs;
uint64_t step_simu;
uint64_t max_step_simu;

// Output variables
extern double aircraft_dynamics495_Va_Va_filter_100449_Va[2];
extern double aircraft_dynamics495_az_az_filter_100458_az[2];
extern double aircraft_dynamics495_Vz_Vz_filter_100452_Vz[2];
extern double aircraft_dynamics495_q_q_filter_100455_q[2];
extern double aircraft_dynamics495_h_h_filter_100446_h[2];
extern double Va_control_50474_delta_th_c_delta_th_c;
extern double Vz_control_50483_delta_e_c_delta_e_c;

__attribute__((noinline))
void copy_output_vars(output_t* v, uint64_t step)
{
	v->sig_outputs.Va 	= aircraft_dynamics495_Va_Va_filter_100449_Va[step%2];
	v->sig_outputs.Vz 	= aircraft_dynamics495_Vz_Vz_filter_100452_Vz[step%2];
	v->sig_outputs.q  	= aircraft_dynamics495_q_q_filter_100455_q[step%2];
	v->sig_outputs.az 	= aircraft_dynamics495_az_az_filter_100458_az[step%2];
	v->sig_outputs.h  	= aircraft_dynamics495_h_h_filter_100446_h[step%2];
	v->sig_delta_th_c	= Va_control_50474_delta_th_c_delta_th_c;
	v->sig_delta_e_c	= Vz_control_50483_delta_e_c_delta_e_c;
}

__attribute__((noinline))
void rosace_init()
{
	// Initial values
	outs.sig_outputs.Va = 0;
	outs.sig_outputs.Vz = 0;
	outs.sig_outputs.q  = 0;
	outs.sig_outputs.az = 0;
	outs.sig_outputs.h  = 0;
	outs.t_simu         = 0;
	step_simu           = 0;
  get_task_set(&num_of_tasks, &tasks);
}

int main()
{
  rosace_init();
  while(1)
  {  
    // Run through the tasks
    for(int i=0; i<num_of_tasks; i++){
        CALL(i);
    }

    // Progress sim step
    step_simu    = step_simu + 1;
    outs.t_simu += 5;

    // Copy outputs
    copy_output_vars(&outs, step_simu);
    
    // Wait
    DEADLINE = 400000;
    unsigned int var = DEADLINE;
  }
}