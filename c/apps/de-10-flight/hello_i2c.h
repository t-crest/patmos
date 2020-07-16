#ifndef FLIGHT_HELLO_I2C_H
#define FLIGHT_HELLO_I2C_H
#include "common.h"
#include "bmi055_accel.h"
#include "bmi055_gyro.h"
//LEDs
#define LED (*((volatile _IODEV unsigned *)PATMOS_IO_LED))

#define MULTIPLY_FP_RESOLUTION_BITS 15

static struct Report read_accel;
static struct Report read_gyro;

static int p_angleY = 0;
static int p_angleZ = 0;
static int r_angleX = 0;
static int r_angleZ = 0;
static int lastTime = 0;

static int pitch;
static int roll;

void flash_leds_my();
void print_accel();
void print_gyro();
float _abs(float x);
int16_t atan2_fp(int16_t y_fp, int16_t x_fp);
float calc_pith();
float calc_roll();

#endif