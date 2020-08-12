#ifndef ROSACE_INCLUDES_H
#define ROSACE_INCLUDES_H
/* we need forward declaration only in order
 * to avoid redefinition in assemblage_vX generated headers
 * Real "#include "assemblage.h" is only done in assemblage_includes.c
 */
struct aircraft_dynamics_outs_t;

/* ***************************************************************************
 * Shared constants
 * ************************************************************************* */
#define delta_th_eq (1.5868660794926)
#define delta_e_eq (0.012009615652468)
extern const double h_eq;
extern const double Va_eq;
#ifndef NBMAX_SAMPLE
#define NBMAX_SAMPLE (6000000/4)
#endif
typedef enum SAMPLE_RANK {
  SPL_T, SPL_VA,SPL_AZ,SPL_Q,SPL_VZ,SPL_H,
  SPL_DELTA_TH_C, SPL_DELTA_E_C,
  SPL_SIZE
} SampleRank_t;
extern double sample[SPL_SIZE][NBMAX_SAMPLE];

void print_inmemory_sample();

/* ***************************************************************************
 * The prelude imported node prototypes
 * ************************************************************************* */
/**
 * Va filter (100/50/33/25 Hz --> 10/20/30/40 ms period)
 * @param[in] Va, airspeed (m/s)
 * @return Va_f, filtered airspeed (m/s)
 * 2nd order Butterworth filter with fc = 0.5 Hz (Matlab function butter)
 * Discretized with Zero-order Hold method with Ts = 0.01/0.02/0.03/0.04 (Matlab function c2d)
 */
double
Va_filter_100(double Va);

double
Va_filter_50(double Va);

double
Va_filter_33(double Va);

double
Va_filter_25(double Va);

/**
 * Vz filter (100/50/33/25 Hz --> 10/20/30/40 ms period)
 * @param[in] Vz, vertical speed (m/s)
 * @return Vz_f, filtered vertical airspeed (m/s)
 * 2nd order Butterworth filter with fc = 0.5 Hz (Matlab function butter)
 * Discretized with Zero-order Hold method with Ts = 0.01/0.02/0.03/0.04 (Matlab function c2d)
 */
double
Vz_filter_100(double Vz);

double
Vz_filter_50 (double Vz);

double
Vz_filter_33 (double Vz);

double
Vz_filter_25 (double Vz);

/**
 * q filter (100/50/33/25 Hz --> 10/20/30/40 ms period)
 * @param[in] q, pitch rate (rad/s)
 * @return q_f, filtered pitch rate (rad/s)
 * 2nd order Butterworth filter with fc = 3.0 Hz (Matlab function butter)
 * Discretized with Zero-order Hold method with Ts = 0.01/0.02/0.03/0.04 (Matlab function c2d)
 */
double
q_filter_100(double q);

double
q_filter_50 (double q);

double
q_filter_33 (double q);

double
q_filter_25 (double q);

/**
 * az filter (100/50/33/25 Hz --> 10/20/30/40 ms period)
 * @param[in] az, normal acceleration (m/s^2)
 * @return az_f, filtered normal acceleration (m/s^2)
 * 2nd order Butterworth filter with fc = 10.0 Hz (Matlab function butter)
 * Discretized with Zero-order Hold method with Ts = 0.01/0.02/0.03/0.04 (Matlab function c2d)
 */
double
az_filter_100(double az);

double
az_filter_50 (double az);

double
az_filter_33 (double az);

double
az_filter_25 (double az);

/**
 * h filter (100/50/33/25 Hz --> 10/20/30/40 ms period)
 * @param[in] h, altitude (m)
 * @return h_f, filtered altitude (m)
 * 2nd order Butterworth filter with fc = 3.0 Hz (Matlab function butter)
 * Discretized with Zero-order Hold method with Ts = 0.01/0.02/0.03/0.04 (Matlab function c2d)
 */
double
h_filter_100(double h);

double
h_filter_50 (double h);

double
h_filter_33 (double h);

double
h_filter_25 (double h);

/**
 * Altitude hold controller (rate 50/33/25/10 Hz sampling period 0.02/0.03/0.04/0.1)
 * @param[in] h_f, filtered altitude (m)
 * @param[in] h_c, commanded altitude (m)
 * @return Vz_c, commanded vertical speed (m/s)
 * Generates the vertical speed command Vz_c to track altitude change h_c
 */
double
altitude_hold_50 (double h_f, double h_c);

double
altitude_hold_33 (double h_f, double h_c);

double
altitude_hold_25 (double h_f, double h_c);

double
altitude_hold_10 (double h_f, double h_c);

/**
 * Vz Speed controller (rate 50/33/25/10 Hz sampling period 0.02/0.03/0.04/0.1)
 * @param[in] Vz_f, filtered vertical speed (m/s)
 * @param[in] Vz_c, commanded vertical speed (m/s)
 * @param[in] q_f, filtered pitch rate (rad/s)
 * @param[in] az_f, filtered normal acceleration (m/s^2)
 * @return delta_e_c, commanded elevator deflection (rad)
 * Generates the elevator deflection command to track vertical speed command Vz_c
 */
double
Vz_control_50 (double Vz_f, double Vz_c, double q_f, double az_f);

double
Vz_control_33 (double Vz_f, double Vz_c, double q_f, double az_f);

double
Vz_control_25 (double Vz_f, double Vz_c, double q_f, double az_f);

double
Vz_control_10 (double Vz_f, double Vz_c, double q_f, double az_f);

/**
 * Va Speed controller (rate 50/33/25/10 Hz sampling period 0.02/0.03/0.04/0.1)
 * @param[in] Va_f, filtered airspeed (m/s)
 * @param[in] Vz_f, filtered vertical speed (m/s)
 * @param[in] q_f, filtered pitch rate (rad/s)
 * @return delta_th_c, commanded throttle (-)
 * Generates the throttle command to track airspeed change Va_c
 */
double
Va_control_50 (double Va_f, double Vz_f, double q_f, double Va_c);

double
Va_control_33 (double Va_f, double Vz_f, double q_f, double Va_c);

double
Va_control_25 (double Va_f, double Vz_f, double q_f, double Va_c);

double
Va_control_10 (double Va_f, double Vz_f, double q_f, double Va_c);

/**
 * Engine (200 Hz --> 5ms period)
 * @param[in] delta_th_c, commanded throttle (-)
 * @return T, Thrust (N)
 * 1st order system with time constant 0.5 s
 * ODE Solver: Euler method with fixed-step = 0.005 (200 Hz)
 */
double
engine(double delta_th_c);

/**
 * Elevator (200 Hz --> 5ms period)
 * @param[in] delta_e_c, commanded elevator deflection (rad)
 * @return delta_e, elevator deflection (rad)
 * 2nd order system (natural frequency omega = 25.0 rad/s and damping xi = 0.85)
 * ODE Solver: Euler method with fixed-step = 0.005 s (200 Hz)
 */
double
elevator(double delta_e_c);

/**
 * Flight dynamics (200 Hz --> 5ms period)
 * @param[in] i, the simulation step
 * @param[in] delta_e, elevator deflection (rad)
 * @param[in] T, Thrust (N)
 * @param[out] outputs, the outputs Va, Vz, q, az, h
 * Aircraft flight dynamics
 * ODE Solver: Euler method with fixed-step = 0.005 s (200 Hz)
 */
void
aircraft_dynamics (double delta_e, double T,  struct aircraft_dynamics_outs_t *outputs);

/* ***************************************************************************
 * The prelude sensor node prototypes
 * ************************************************************************* */

/**
 * (200 Hz --> 5ms period)
 */
double
input_h_c();

double
input_Va_c();


/* ***************************************************************************
 * The prelude actuator node prototypes
 * ************************************************************************* */

/**
 * (200 Hz --> 5ms period)
 */
void
output_delta_th_c(double delta_th_c);

/**
 * (200 Hz --> 5ms period)
 */
void
output_delta_e_c(double delta_e_c);

#endif