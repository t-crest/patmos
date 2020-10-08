#ifndef ASSEMBLAGE_INCLUDES_H
#define ASSEMBLAGE_INCLUDES_H
#include "types.h"
#include "io.h"


/* ***************************************************************************
 * Shared constants
 * ************************************************************************* */
#define delta_th_eq (1.5868660794926)
#define delta_e_eq (0.012009615652468)
extern const REAL_TYPE h_eq;
extern const REAL_TYPE Va_eq;
#ifndef NBMAX_SAMPLE
#define NBMAX_SAMPLE (6000000/4)
#endif
extern REAL_TYPE sample[SPL_SIZE][NBMAX_SAMPLE];

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
REAL_TYPE
Va_filter_100(REAL_TYPE Va);

REAL_TYPE
Va_filter_50(REAL_TYPE Va);

REAL_TYPE
Va_filter_33(REAL_TYPE Va);

REAL_TYPE
Va_filter_25(REAL_TYPE Va);

/**
 * Vz filter (100/50/33/25 Hz --> 10/20/30/40 ms period)
 * @param[in] Vz, vertical speed (m/s)
 * @return Vz_f, filtered vertical airspeed (m/s)
 * 2nd order Butterworth filter with fc = 0.5 Hz (Matlab function butter)
 * Discretized with Zero-order Hold method with Ts = 0.01/0.02/0.03/0.04 (Matlab function c2d)
 */
REAL_TYPE
Vz_filter_100(REAL_TYPE Vz);

REAL_TYPE
Vz_filter_50 (REAL_TYPE Vz);

REAL_TYPE
Vz_filter_33 (REAL_TYPE Vz);

REAL_TYPE
Vz_filter_25 (REAL_TYPE Vz);

/**
 * q filter (100/50/33/25 Hz --> 10/20/30/40 ms period)
 * @param[in] q, pitch rate (rad/s)
 * @return q_f, filtered pitch rate (rad/s)
 * 2nd order Butterworth filter with fc = 3.0 Hz (Matlab function butter)
 * Discretized with Zero-order Hold method with Ts = 0.01/0.02/0.03/0.04 (Matlab function c2d)
 */
REAL_TYPE
q_filter_100(REAL_TYPE q);

REAL_TYPE
q_filter_50 (REAL_TYPE q);

REAL_TYPE
q_filter_33 (REAL_TYPE q);

REAL_TYPE
q_filter_25 (REAL_TYPE q);

/**
 * az filter (100/50/33/25 Hz --> 10/20/30/40 ms period)
 * @param[in] az, normal acceleration (m/s^2)
 * @return az_f, filtered normal acceleration (m/s^2)
 * 2nd order Butterworth filter with fc = 10.0 Hz (Matlab function butter)
 * Discretized with Zero-order Hold method with Ts = 0.01/0.02/0.03/0.04 (Matlab function c2d)
 */
REAL_TYPE
az_filter_100(REAL_TYPE az);

REAL_TYPE
az_filter_50 (REAL_TYPE az);

REAL_TYPE
az_filter_33 (REAL_TYPE az);

REAL_TYPE
az_filter_25 (REAL_TYPE az);

/**
 * h filter (100/50/33/25 Hz --> 10/20/30/40 ms period)
 * @param[in] h, altitude (m)
 * @return h_f, filtered altitude (m)
 * 2nd order Butterworth filter with fc = 3.0 Hz (Matlab function butter)
 * Discretized with Zero-order Hold method with Ts = 0.01/0.02/0.03/0.04 (Matlab function c2d)
 */
REAL_TYPE
h_filter_100(REAL_TYPE h);

REAL_TYPE
h_filter_50 (REAL_TYPE h);

REAL_TYPE
h_filter_33 (REAL_TYPE h);

REAL_TYPE
h_filter_25 (REAL_TYPE h);

/**
 * Altitude hold controller (rate 50/33/25/10 Hz sampling period 0.02/0.03/0.04/0.1)
 * @param[in] h_f, filtered altitude (m)
 * @param[in] h_c, commanded altitude (m)
 * @return Vz_c, commanded vertical speed (m/s)
 * Generates the vertical speed command Vz_c to track altitude change h_c
 */
REAL_TYPE
altitude_hold_50 (REAL_TYPE h_f, REAL_TYPE h_c);

REAL_TYPE
altitude_hold_33 (REAL_TYPE h_f, REAL_TYPE h_c);

REAL_TYPE
altitude_hold_25 (REAL_TYPE h_f, REAL_TYPE h_c);

REAL_TYPE
altitude_hold_12 (REAL_TYPE h_f, REAL_TYPE h_c);

REAL_TYPE
altitude_hold_10 (REAL_TYPE h_f, REAL_TYPE h_c);

/**
 * Vz Speed controller (rate 50/33/25/10 Hz sampling period 0.02/0.03/0.04/0.1)
 * @param[in] Vz_f, filtered vertical speed (m/s)
 * @param[in] Vz_c, commanded vertical speed (m/s)
 * @param[in] q_f, filtered pitch rate (rad/s)
 * @param[in] az_f, filtered normal acceleration (m/s^2)
 * @return delta_e_c, commanded elevator deflection (rad)
 * Generates the elevator deflection command to track vertical speed command Vz_c
 */
REAL_TYPE
Vz_control_50 (REAL_TYPE Vz_f, REAL_TYPE Vz_c, REAL_TYPE q_f, REAL_TYPE az_f);

REAL_TYPE
Vz_control_33 (REAL_TYPE Vz_f, REAL_TYPE Vz_c, REAL_TYPE q_f, REAL_TYPE az_f);

REAL_TYPE
Vz_control_25 (REAL_TYPE Vz_f, REAL_TYPE Vz_c, REAL_TYPE q_f, REAL_TYPE az_f);

REAL_TYPE
Vz_control_12 (REAL_TYPE Vz_f, REAL_TYPE Vz_c, REAL_TYPE q_f, REAL_TYPE az_f);

REAL_TYPE
Vz_control_10 (REAL_TYPE Vz_f, REAL_TYPE Vz_c, REAL_TYPE q_f, REAL_TYPE az_f);

/**
 * Va Speed controller (rate 50/33/25/10 Hz sampling period 0.02/0.03/0.04/0.1)
 * @param[in] Va_f, filtered airspeed (m/s)
 * @param[in] Vz_f, filtered vertical speed (m/s)
 * @param[in] q_f, filtered pitch rate (rad/s)
 * @return delta_th_c, commanded throttle (-)
 * Generates the throttle command to track airspeed change Va_c
 */
REAL_TYPE
Va_control_50 (REAL_TYPE Va_f, REAL_TYPE Vz_f, REAL_TYPE q_f, REAL_TYPE Va_c);

REAL_TYPE
Va_control_33 (REAL_TYPE Va_f, REAL_TYPE Vz_f, REAL_TYPE q_f, REAL_TYPE Va_c);

REAL_TYPE
Va_control_25 (REAL_TYPE Va_f, REAL_TYPE Vz_f, REAL_TYPE q_f, REAL_TYPE Va_c);

REAL_TYPE
Va_control_12 (REAL_TYPE Va_f, REAL_TYPE Vz_f, REAL_TYPE q_f, REAL_TYPE Va_c);

REAL_TYPE
Va_control_10 (REAL_TYPE Va_f, REAL_TYPE Vz_f, REAL_TYPE q_f, REAL_TYPE Va_c);

/**
 * Engine (200 Hz --> 5ms period)
 * @param[in] delta_th_c, commanded throttle (-)
 * @return T, Thrust (N)
 * 1st order system with time constant 0.5 s
 * ODE Solver: Euler method with fixed-step = 0.005 (200 Hz)
 */
REAL_TYPE
engine(REAL_TYPE delta_th_c);

/**
 * Elevator (200 Hz --> 5ms period)
 * @param[in] delta_e_c, commanded elevator deflection (rad)
 * @return delta_e, elevator deflection (rad)
 * 2nd order system (natural frequency omega = 25.0 rad/s and damping xi = 0.85)
 * ODE Solver: Euler method with fixed-step = 0.005 s (200 Hz)
 */
REAL_TYPE
elevator(REAL_TYPE delta_e_c);

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
aircraft_dynamics (REAL_TYPE delta_e, REAL_TYPE T,  struct aircraft_dynamics_outs_t *outputs);

/* ***************************************************************************
 * The prelude sensor node prototypes
 * ************************************************************************* */

/**
 * (200 Hz --> 5ms period)
 */
REAL_TYPE
input_h_c();

REAL_TYPE
input_Va_c();


/* ***************************************************************************
 * The prelude actuator node prototypes
 * ************************************************************************* */

/**
 * (200 Hz --> 5ms period)
 */
void
output_delta_th_c(REAL_TYPE delta_th_c);

/**
 * (200 Hz --> 5ms period)
 */
void
output_delta_e_c(REAL_TYPE delta_e_c);

#endif
