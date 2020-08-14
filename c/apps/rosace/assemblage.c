#include <stdlib.h>
#include <stdbool.h>
#include "assemblage.h"
#include "assemblage_includes.h"

double aircraft_dynamics495_Va_Va_filter_100449_Va[2];
double Vz_control_50483_delta_e_c_elevator489_delta_e_c;
double Va_filter_100449_Va_f_Va_control_50474_Va_f[2];
double Vz_filter_100452_Vz_f_Va_control_50474_Vz_f[2];
double q_filter_100455_q_f_Va_control_50474_q_f[2];
double Va_c_Va_control_50474_Va_c;
double h_filter_100446_h_f_altitude_hold_50464_h_f[2];
double h_c_altitude_hold_50464_h_c;
double Va_control_50474_delta_th_c_delta_th_c;
double aircraft_dynamics495_az_az_filter_100458_az[2];
double aircraft_dynamics495_Vz_Vz_filter_100452_Vz[2];
double aircraft_dynamics495_q_q_filter_100455_q[2];
double elevator489_delta_e_aircraft_dynamics495_delta_e[3]={0.0120096156525, 0.0120096156525, 0.0120096156525};
double engine486_T_aircraft_dynamics495_T[3]={41813.9211946, 41813.9211946, 41813.9211946};
double aircraft_dynamics495_h_h_filter_100446_h[2];
double Va_control_50474_delta_th_c_engine486_delta_th_c;
double Vz_filter_100452_Vz_f_Vz_control_50483_Vz_f[2];
double altitude_hold_50464_Vz_c_Vz_control_50483_Vz_c;
double q_filter_100455_q_f_Vz_control_50483_q_f[2];
double az_filter_100458_az_f_Vz_control_50483_az_f[2];
double Vz_control_50483_delta_e_c_delta_e_c;

int Va_filter_100449_fun(void* args)
{
  double Va_f;
  static int Va_rcell=0;
  const struct write_proto_t Va_f_Va_control_50474_Va_f_write =
  { NULL, 0, (int []){ true , false }, 2 };
  static int Va_f_Va_control_50474_Va_f_wcell=0;
  static int instance=0;
  
  Va_f=Va_filter_100(aircraft_dynamics495_Va_Va_filter_100449_Va[Va_rcell]);
  Va_rcell=(Va_rcell+1)%2;
  if(must_write(Va_f_Va_control_50474_Va_f_write,instance)) {
    Va_filter_100449_Va_f_Va_control_50474_Va_f[Va_f_Va_control_50474_Va_f_wcell]=Va_f;
    Va_f_Va_control_50474_Va_f_wcell=(Va_f_Va_control_50474_Va_f_wcell+1)%2;
  }
  instance++;
  
  return 0;
}

int elevator489_fun(void* args)
{
  double delta_e;
  static int delta_e_aircraft_dynamics495_delta_e_wcell=1;
  static int instance=0;
  
  delta_e=elevator(Vz_control_50483_delta_e_c_elevator489_delta_e_c);
  elevator489_delta_e_aircraft_dynamics495_delta_e[delta_e_aircraft_dynamics495_delta_e_wcell]=delta_e;
  delta_e_aircraft_dynamics495_delta_e_wcell=(delta_e_aircraft_dynamics495_delta_e_wcell+1)%3;
  instance++;
  
  return 0;
}

int Va_control_50474_fun(void* args)
{
  double delta_th_c;
  static int Va_f_rcell=0;
  static int Vz_f_rcell=0;
  static int q_f_rcell=0;
  static int instance=0;
  
  delta_th_c=Va_control_50(Va_filter_100449_Va_f_Va_control_50474_Va_f[Va_f_rcell],
  Vz_filter_100452_Vz_f_Va_control_50474_Vz_f[Vz_f_rcell],q_filter_100455_q_f_Va_control_50474_q_f[q_f_rcell],
  Va_c_Va_control_50474_Va_c);
  Va_f_rcell=(Va_f_rcell+1)%2;
  Vz_f_rcell=(Vz_f_rcell+1)%2;
  q_f_rcell=(q_f_rcell+1)%2;
  Va_control_50474_delta_th_c_engine486_delta_th_c=delta_th_c;
  Va_control_50474_delta_th_c_delta_th_c=delta_th_c;
  instance++;
  
  return 0;
}

int Va_c0_fun(void* args)
{
  double Va_c;
  static int instance=0;
  
  Va_c=input_Va_c();
  Va_c_Va_control_50474_Va_c=Va_c;
  instance++;
  
  return 0;
}

int altitude_hold_50464_fun(void* args)
{
  double Vz_c;
  static int h_f_rcell=0;
  static int instance=0;
  
  Vz_c=altitude_hold_50(h_filter_100446_h_f_altitude_hold_50464_h_f[h_f_rcell],
  h_c_altitude_hold_50464_h_c);
  h_f_rcell=(h_f_rcell+1)%2;
  altitude_hold_50464_Vz_c_Vz_control_50483_Vz_c=Vz_c;
  instance++;
  
  return 0;
}

int delta_th_c0_fun(void* args)
{
  static int instance=0;
  
  output_delta_th_c(Va_control_50474_delta_th_c_delta_th_c);
  instance++;
  
  return 0;
}

int az_filter_100458_fun(void* args)
{
  double az_f;
  static int az_rcell=0;
  const struct write_proto_t az_f_Vz_control_50483_az_f_write =
  { NULL, 0, (int []){ true , false }, 2 };
  static int az_f_Vz_control_50483_az_f_wcell=0;
  static int instance=0;
  
  az_f=az_filter_100(aircraft_dynamics495_az_az_filter_100458_az[az_rcell]);
  az_rcell=(az_rcell+1)%2;
  if(must_write(az_f_Vz_control_50483_az_f_write,instance)) {
    az_filter_100458_az_f_Vz_control_50483_az_f[az_f_Vz_control_50483_az_f_wcell]=az_f;
    az_f_Vz_control_50483_az_f_wcell=(az_f_Vz_control_50483_az_f_wcell+1)%2;
  }
  instance++;
  
  return 0;
}

int Vz_filter_100452_fun(void* args)
{
  double Vz_f;
  static int Vz_rcell=0;
  const struct write_proto_t Vz_f_Va_control_50474_Vz_f_write =
  { NULL, 0, (int []){ true , false }, 2 };
  static int Vz_f_Va_control_50474_Vz_f_wcell=0;
  const struct write_proto_t Vz_f_Vz_control_50483_Vz_f_write =
  { NULL, 0, (int []){ true , false }, 2 };
  static int Vz_f_Vz_control_50483_Vz_f_wcell=0;
  static int instance=0;
  
  Vz_f=Vz_filter_100(aircraft_dynamics495_Vz_Vz_filter_100452_Vz[Vz_rcell]);
  Vz_rcell=(Vz_rcell+1)%2;
  if(must_write(Vz_f_Va_control_50474_Vz_f_write,instance)) {
    Vz_filter_100452_Vz_f_Va_control_50474_Vz_f[Vz_f_Va_control_50474_Vz_f_wcell]=Vz_f;
    Vz_f_Va_control_50474_Vz_f_wcell=(Vz_f_Va_control_50474_Vz_f_wcell+1)%2;
  }
  if(must_write(Vz_f_Vz_control_50483_Vz_f_write,instance)) {
    Vz_filter_100452_Vz_f_Vz_control_50483_Vz_f[Vz_f_Vz_control_50483_Vz_f_wcell]=Vz_f;
    Vz_f_Vz_control_50483_Vz_f_wcell=(Vz_f_Vz_control_50483_Vz_f_wcell+1)%2;
  }
  instance++;
  
  return 0;
}

int q_filter_100455_fun(void* args)
{
  double q_f;
  static int q_rcell=0;
  const struct write_proto_t q_f_Va_control_50474_q_f_write =
  { NULL, 0, (int []){ true , false }, 2 };
  static int q_f_Va_control_50474_q_f_wcell=0;
  const struct write_proto_t q_f_Vz_control_50483_q_f_write =
  { NULL, 0, (int []){ true , false }, 2 };
  static int q_f_Vz_control_50483_q_f_wcell=0;
  static int instance=0;
  
  q_f=q_filter_100(aircraft_dynamics495_q_q_filter_100455_q[q_rcell]);
  q_rcell=(q_rcell+1)%2;
  if(must_write(q_f_Va_control_50474_q_f_write,instance)) {
    q_filter_100455_q_f_Va_control_50474_q_f[q_f_Va_control_50474_q_f_wcell]=q_f;
    q_f_Va_control_50474_q_f_wcell=(q_f_Va_control_50474_q_f_wcell+1)%2;
  }
  if(must_write(q_f_Vz_control_50483_q_f_write,instance)) {
    q_filter_100455_q_f_Vz_control_50483_q_f[q_f_Vz_control_50483_q_f_wcell]=q_f;
    q_f_Vz_control_50483_q_f_wcell=(q_f_Vz_control_50483_q_f_wcell+1)%2;
  }
  instance++;
  
  return 0;
}

int aircraft_dynamics495_fun(void* args)
{
  struct aircraft_dynamics_outs_t aircraft_dynamics495_fun_outs;
  static int delta_e_rcell=0;
  static int T_rcell=0;
  const struct write_proto_t Va_Va_filter_100449_Va_write =
  { NULL, 0, (int []){ true , false }, 2 };
  static int Va_Va_filter_100449_Va_wcell=0;
  const struct write_proto_t Vz_Vz_filter_100452_Vz_write =
  { NULL, 0, (int []){ true , false }, 2 };
  static int Vz_Vz_filter_100452_Vz_wcell=0;
  const struct write_proto_t q_q_filter_100455_q_write =
  { NULL, 0, (int []){ true , false }, 2 };
  static int q_q_filter_100455_q_wcell=0;
  const struct write_proto_t az_az_filter_100458_az_write =
  { NULL, 0, (int []){ true , false }, 2 };
  static int az_az_filter_100458_az_wcell=0;
  const struct write_proto_t h_h_filter_100446_h_write =
  { NULL, 0, (int []){ true , false }, 2 };
  static int h_h_filter_100446_h_wcell=0;
  static int instance=0;
  
  aircraft_dynamics(elevator489_delta_e_aircraft_dynamics495_delta_e[delta_e_rcell],
  engine486_T_aircraft_dynamics495_T[T_rcell],&aircraft_dynamics495_fun_outs);
  delta_e_rcell=(delta_e_rcell+1)%3;
  T_rcell=(T_rcell+1)%3;
  if(must_write(Va_Va_filter_100449_Va_write,instance)) {
    aircraft_dynamics495_Va_Va_filter_100449_Va[Va_Va_filter_100449_Va_wcell]=aircraft_dynamics495_fun_outs.Va;
    Va_Va_filter_100449_Va_wcell=(Va_Va_filter_100449_Va_wcell+1)%2;
  }
  if(must_write(Vz_Vz_filter_100452_Vz_write,instance)) {
    aircraft_dynamics495_Vz_Vz_filter_100452_Vz[Vz_Vz_filter_100452_Vz_wcell]=aircraft_dynamics495_fun_outs.Vz;
    Vz_Vz_filter_100452_Vz_wcell=(Vz_Vz_filter_100452_Vz_wcell+1)%2;
  }
  if(must_write(q_q_filter_100455_q_write,instance)) {
    aircraft_dynamics495_q_q_filter_100455_q[q_q_filter_100455_q_wcell]=aircraft_dynamics495_fun_outs.q;
    q_q_filter_100455_q_wcell=(q_q_filter_100455_q_wcell+1)%2;
  }
  if(must_write(az_az_filter_100458_az_write,instance)) {
    aircraft_dynamics495_az_az_filter_100458_az[az_az_filter_100458_az_wcell]=aircraft_dynamics495_fun_outs.az;
    az_az_filter_100458_az_wcell=(az_az_filter_100458_az_wcell+1)%2;
  }
  if(must_write(h_h_filter_100446_h_write,instance)) {
    aircraft_dynamics495_h_h_filter_100446_h[h_h_filter_100446_h_wcell]=aircraft_dynamics495_fun_outs.h;
    h_h_filter_100446_h_wcell=(h_h_filter_100446_h_wcell+1)%2;
  }
  instance++;
  
  return 0;
}

int h_filter_100446_fun(void* args)
{
  double h_f;
  static int h_rcell=0;
  const struct write_proto_t h_f_altitude_hold_50464_h_f_write =
  { NULL, 0, (int []){ true , false }, 2 };
  static int h_f_altitude_hold_50464_h_f_wcell=0;
  static int instance=0;
  
  h_f=h_filter_100(aircraft_dynamics495_h_h_filter_100446_h[h_rcell]);
  h_rcell=(h_rcell+1)%2;
  if(must_write(h_f_altitude_hold_50464_h_f_write,instance)) {
    h_filter_100446_h_f_altitude_hold_50464_h_f[h_f_altitude_hold_50464_h_f_wcell]=h_f;
    h_f_altitude_hold_50464_h_f_wcell=(h_f_altitude_hold_50464_h_f_wcell+1)%2;
  }
  instance++;
  
  return 0;
}

int engine486_fun(void* args)
{
  double T;
  static int T_aircraft_dynamics495_T_wcell=1;
  static int instance=0;
  
  T=engine(Va_control_50474_delta_th_c_engine486_delta_th_c);
  engine486_T_aircraft_dynamics495_T[T_aircraft_dynamics495_T_wcell]=T;
  T_aircraft_dynamics495_T_wcell=(T_aircraft_dynamics495_T_wcell+1)%3;
  instance++;
  
  return 0;
}

int Vz_control_50483_fun(void* args)
{
  double delta_e_c;
  static int Vz_f_rcell=0;
  static int q_f_rcell=0;
  static int az_f_rcell=0;
  static int instance=0;
  
  delta_e_c=Vz_control_50(Vz_filter_100452_Vz_f_Vz_control_50483_Vz_f[Vz_f_rcell],
  altitude_hold_50464_Vz_c_Vz_control_50483_Vz_c,q_filter_100455_q_f_Vz_control_50483_q_f[q_f_rcell],
  az_filter_100458_az_f_Vz_control_50483_az_f[az_f_rcell]);
  Vz_f_rcell=(Vz_f_rcell+1)%2;
  q_f_rcell=(q_f_rcell+1)%2;
  az_f_rcell=(az_f_rcell+1)%2;
  Vz_control_50483_delta_e_c_delta_e_c=delta_e_c;
  Vz_control_50483_delta_e_c_elevator489_delta_e_c=delta_e_c;
  instance++;
  
  return 0;
}

int delta_e_c0_fun(void* args)
{
  static int instance=0;
  
  output_delta_e_c(Vz_control_50483_delta_e_c_delta_e_c);
  instance++;
  
  return 0;
}

int h_c0_fun(void* args)
{
  double h_c;
  static int instance=0;
  
  h_c=input_h_c();
  h_c_altitude_hold_50464_h_c=h_c;
  instance++;
  
  return 0;
}

#define PLUD_TASK_NUMBER 15
static struct nonencoded_task_params static_task_set[PLUD_TASK_NUMBER] = {
  { "h_c0", 1000, 0, 1, 1000, h_c0_fun },
  { "delta_e_c0", 200, 0, 1, 200, delta_e_c0_fun },
  { "Vz_control_50483", 200, 0, 1, 200, Vz_control_50483_fun },
  { "engine486", 50, 0, 1, 50, engine486_fun },
  { "h_filter_100446", 100, 0, 1, 100, h_filter_100446_fun },
  { "aircraft_dynamics495", 50, 0, 1, 50, aircraft_dynamics495_fun },
  { "q_filter_100455", 100, 0, 1, 100, q_filter_100455_fun },
  { "Vz_filter_100452", 100, 0, 1, 100, Vz_filter_100452_fun },
  { "az_filter_100458", 100, 0, 1, 100, az_filter_100458_fun },
  { "delta_th_c0", 200, 0, 1, 200, delta_th_c0_fun },
  { "altitude_hold_50464", 200, 0, 1, 200, altitude_hold_50464_fun },
  { "Va_c0", 1000, 0, 1, 1000, Va_c0_fun },
  { "Va_control_50474", 200, 0, 1, 200, Va_control_50474_fun },
  { "elevator489", 50, 0, 1, 50, elevator489_fun },
  { "Va_filter_100449", 100, 0, 1, 100, Va_filter_100449_fun }
};



void get_task_set (int* task_number, struct nonencoded_task_params** task_set)
{
  *task_number = PLUD_TASK_NUMBER;
  *task_set=static_task_set;
}

static struct job_prec engine486_aircraft_dynamics495_pcpat[1] = { {0,1} };
static struct job_prec Va_filter_100449_Va_control_50474_pcpat[1] = { 
{0,0} };
static struct job_prec q_filter_100455_Va_control_50474_pcpat[1] = { {0,0} };
static struct job_prec q_filter_100455_Vz_control_50483_pcpat[1] = { {0,0} };
static struct job_prec aircraft_dynamics495_q_filter_100455_pcpat[1] = {
{0,0} };
static struct job_prec Va_c0_Va_control_50474_pcpat[1] = { {0,0} };
static struct job_prec Vz_control_50483_delta_e_c0_pcpat[1] = { {0,0} };
static struct job_prec Vz_control_50483_elevator489_pcpat[1] = { {0,0} };
static struct job_prec Vz_filter_100452_Va_control_50474_pcpat[1] = { 
{0,0} };
static struct job_prec Vz_filter_100452_Vz_control_50483_pcpat[1] = { 
{0,0} };
static struct job_prec h_filter_100446_altitude_hold_50464_pcpat[1] = { 
{0,0} };
static struct job_prec aircraft_dynamics495_az_filter_100458_pcpat[1] = {
{0,0} };
static struct job_prec elevator489_aircraft_dynamics495_pcpat[1] = { {0,1} };
static struct job_prec aircraft_dynamics495_Vz_filter_100452_pcpat[1] = {
{0,0} };
static struct job_prec az_filter_100458_Vz_control_50483_pcpat[1] = { 
{0,0} };
static struct job_prec h_c0_altitude_hold_50464_pcpat[1] = { {0,0} };
static struct job_prec altitude_hold_50464_Vz_control_50483_pcpat[1] = {
{0,0} };
static struct job_prec aircraft_dynamics495_Va_filter_100449_pcpat[1] = {
{0,0} };
static struct job_prec aircraft_dynamics495_h_filter_100446_pcpat[1] = {
{0,0} };
static struct job_prec Va_control_50474_engine486_pcpat[1] = { {0,0} };
static struct job_prec Va_control_50474_delta_th_c0_pcpat[1] = { {0,0} };

#define PLUD_PREC_NUMBER 21
static struct multirate_precedence static_prec_set[PLUD_PREC_NUMBER] = {
  { "engine486", "aircraft_dynamics495", 0, 1, NULL,
  engine486_aircraft_dynamics495_pcpat },
  { "Va_filter_100449", "Va_control_50474", 0, 1, NULL,
  Va_filter_100449_Va_control_50474_pcpat },
  { "q_filter_100455", "Va_control_50474", 0, 1, NULL,
  q_filter_100455_Va_control_50474_pcpat },
  { "q_filter_100455", "Vz_control_50483", 0, 1, NULL,
  q_filter_100455_Vz_control_50483_pcpat },
  { "aircraft_dynamics495", "q_filter_100455", 0, 1, NULL,
  aircraft_dynamics495_q_filter_100455_pcpat },
  { "Va_c0", "Va_control_50474", 0, 1, NULL, Va_c0_Va_control_50474_pcpat },
  { "Vz_control_50483", "delta_e_c0", 0, 1, NULL,
  Vz_control_50483_delta_e_c0_pcpat },
  { "Vz_control_50483", "elevator489", 0, 1, NULL,
  Vz_control_50483_elevator489_pcpat },
  { "Vz_filter_100452", "Va_control_50474", 0, 1, NULL,
  Vz_filter_100452_Va_control_50474_pcpat },
  { "Vz_filter_100452", "Vz_control_50483", 0, 1, NULL,
  Vz_filter_100452_Vz_control_50483_pcpat },
  { "h_filter_100446", "altitude_hold_50464", 0, 1, NULL,
  h_filter_100446_altitude_hold_50464_pcpat },
  { "aircraft_dynamics495", "az_filter_100458", 0, 1, NULL,
  aircraft_dynamics495_az_filter_100458_pcpat },
  { "elevator489", "aircraft_dynamics495", 0, 1, NULL,
  elevator489_aircraft_dynamics495_pcpat },
  { "aircraft_dynamics495", "Vz_filter_100452", 0, 1, NULL,
  aircraft_dynamics495_Vz_filter_100452_pcpat },
  { "az_filter_100458", "Vz_control_50483", 0, 1, NULL,
  az_filter_100458_Vz_control_50483_pcpat },
  { "h_c0", "altitude_hold_50464", 0, 1, NULL, h_c0_altitude_hold_50464_pcpat
  },
  { "altitude_hold_50464", "Vz_control_50483", 0, 1, NULL,
  altitude_hold_50464_Vz_control_50483_pcpat },
  { "aircraft_dynamics495", "Va_filter_100449", 0, 1, NULL,
  aircraft_dynamics495_Va_filter_100449_pcpat },
  { "aircraft_dynamics495", "h_filter_100446", 0, 1, NULL,
  aircraft_dynamics495_h_filter_100446_pcpat },
  { "Va_control_50474", "engine486", 0, 1, NULL,
  Va_control_50474_engine486_pcpat },
  { "Va_control_50474", "delta_th_c0", 0, 1, NULL,
  Va_control_50474_delta_th_c0_pcpat }
};

void get_precedence_set (int* prec_number, struct multirate_precedence** prec_set)
{
  *prec_number = PLUD_PREC_NUMBER;
  *prec_set=static_prec_set;
}
