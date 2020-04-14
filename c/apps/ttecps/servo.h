#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define PWM_PERIOD 20000000
#define MIN_CYCLE 0.015
#define MAX_CYCLE 0.1
#define MOTOR_STEP 0.0005

#define HIGH_TIME(DUTY_CYCLE) PWM_PERIOD*DUTY_CYCLE
#define LOW_TIME(DUTY_CYCLE) PWM_PERIOD-HIGH_TIME(DUTY_CYCLE)
#define DEAD_CALC(DUTY_CYCLE) (HIGH_TIME(DUTY_CYCLE)/CPU_PERIOD)