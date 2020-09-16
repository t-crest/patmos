#ifndef _assemblage_H
#define _assemblage_H
#include "nonencoded_task_params.h"
#include "multirate_precedence.h"
#include "com_patterns.h"
#include "types.h"

void get_task_set(int* task_number, struct nonencoded_task_params** task_set);

void get_precedence_set(int* prec_number, struct multirate_precedence** presc);


#define H_C0		0
#define DELTA_E_C0	1
#define VZ_CONTROL	2
#define ENGINE		3
#define H_FILTER	4
#define AIRCRAFT_DYN	5 
#define Q_FILTER	6
#define VZ_FILTER	7
#define AZ_FILTER	8
#define DELTA_TH_C0	9
#define ALTI_HOLD	10
#define VA_C0		11
#define VA_CONTROL	12
#define ELEVATOR	13
#define VA_FILTER	14

#endif
