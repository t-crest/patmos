#include "hello_i2c.h"

//Blinks the LEDs once
void flash_leds_my()
{
  int i, j;
  for (i = 2000; i != 0; --i)
    for (j = 2000; j != 0; --j)
      LED = 0x0001;
  for (i = 2000; i != 0; --i)
    for (j = 2000; j != 0; --j)
      LED = 0x0000;
  return;
}

void print_accel()
{
  read_accel = report_accel();
  printf("Measured at: %d us\n", read_accel.timestamp);
  printf("Accel X: %.2f | Accel Y: %.2f | Accel Z: %.2f\n", read_accel.accel_x * SCALE, read_accel.accel_y * SCALE, read_accel.accel_z * SCALE);
  printf("Temprature: %.2f\n", read_accel.temp);
  printf("\n");
}

void print_gyro()
{
  read_gyro = report_gyro();
  printf("Measured at: %d us\n", read_gyro.timestamp);
  printf("Gyro X: %.2f | Gyro Y: %.2f | Gyro Z: %.2f\n", read_gyro.accel_x * SCALE, read_gyro.accel_y * SCALE, read_gyro.accel_z * SCALE);
  printf("\n");
}

float _abs(float x)
{
  return x < 0 ? x : -x;
}

#ifdef WCET
__attribute__((noinline))
#endif
int16_t
atan2_fp(int16_t y_fp, int16_t x_fp)
{
  int32_t coeff_1 = 45;
  int32_t coeff_1b = -56; // 56.24;
  int32_t coeff_1c = 11;  // 11.25
  int16_t coeff_2 = 135;

  int16_t angle = 0;

  int32_t r;
  int32_t r3;

  int16_t y_abs_fp = y_fp;
  if (y_abs_fp < 0)
    y_abs_fp = -y_abs_fp;

  if (y_fp == 0)
  {
    if (x_fp >= 0)
    {
      angle = 0;
    }
    else
    {
      angle = 180;
    }
  }
  else if (x_fp >= 0)
  {
    r = (((int32_t)(x_fp - y_abs_fp)) << MULTIPLY_FP_RESOLUTION_BITS) /
        ((int32_t)(x_fp + y_abs_fp));

    r3 = r * r;
    r3 = r3 >> MULTIPLY_FP_RESOLUTION_BITS;
    r3 *= r;
    r3 = r3 >> MULTIPLY_FP_RESOLUTION_BITS;
    r3 *= coeff_1c;
    angle = (int16_t)(coeff_1 + ((coeff_1b * r + r3) >>
                                 MULTIPLY_FP_RESOLUTION_BITS));
  }
  else
  {
    r = (((int32_t)(x_fp + y_abs_fp)) << MULTIPLY_FP_RESOLUTION_BITS) /
        ((int32_t)(y_abs_fp - x_fp));
    r3 = r * r;
    r3 = r3 >> MULTIPLY_FP_RESOLUTION_BITS;
    r3 *= r;
    r3 = r3 >> MULTIPLY_FP_RESOLUTION_BITS;
    r3 *= coeff_1c;
    angle = coeff_2 + ((int16_t)(((coeff_1b * r + r3) >>
                                  MULTIPLY_FP_RESOLUTION_BITS)));
  }

  if (y_fp < 0)
    return (-angle); // negate if in quad III or IV
  else
    return (angle);
}

#ifdef WCET
__attribute__((noinline))
#endif
float
calc_pith()
{
  int dt = read_accel.timestamp - lastTime;
  int16_t accelDat = atan2_fp(read_accel.accel_y, read_accel.accel_z);
  // p_angleY = 0.98 * (p_angleY + read_gyro.accel_y * dt) + 0.02 * read_accel.accel_y;
  // p_angleZ = 0.98 * (p_angleZ + read_gyro.accel_z * dt) + 0.02 * read_accel.accel_z;
  p_angleY = ((980 * (p_angleY + read_gyro.accel_y * dt)) >> 10) + ((20 * accelDat) >> 10);
  //p_angleZ = ((980 * (p_angleZ + read_gyro.accel_z * dt)) >> 10) + ((20 * read_accel.accel_z) >> 10);
  return p_angleY;
}

#ifdef WCET
__attribute__((noinline))
#endif
float
calc_roll()
{
  int dt = read_accel.timestamp - lastTime;
  int16_t accelDat = atan2_fp(read_accel.accel_x, read_accel.accel_z);
  // r_angleX = 0.98 * (r_angleX + read_gyro.accel_x * dt) + 0.02 * read_accel.accel_x;
  // r_angleZ = 0.98 * (r_angleZ + read_gyro.accel_z * dt) + 0.02 * read_accel.accel_z;
  r_angleX = ((980 * (r_angleX + read_gyro.accel_x * dt)) >> 10) + ((20 * accelDat) >> 10);
  //r_angleZ = ((980 * (r_angleZ + read_gyro.accel_z * dt)) >> 10) + ((20 * read_accel.accel_z) >> 10);

  return r_angleX;
}

void complementary_filter_fusion()
{
  int dt = read_accel.timestamp - lastTime;
  int pitchAcc, rollAcc;

  pitch += (read_gyro.accel_x / GYRO_SCALE) * 0.01;
  roll -= (read_gyro.accel_y / GYRO_SCALE) * 0.01;

  pitchAcc = atan2_fp(read_accel.accel_y * SCALE, read_accel.accel_z * SCALE) * 180 / 3.14159265359;
  //pitch = ((980 * (pitch)) >> 10) + ((20 * pitchAcc) >> 10);
  pitch = pitch * 0.98 + pitchAcc * 0.02;

  rollAcc = atan2_fp(read_accel.accel_x * SCALE, read_accel.accel_z * SCALE) * 180 / 3.14159265359;
  //roll = ((980 * (roll)) >> 10) + ((20 * rollAcc) >> 10);
  roll = roll * 0.98 + pitchAcc * 0.02;
}

int main(int argc, char **argv)
{
  char linefeed = 0x0A;

  printf("Starting!\n");
  flash_leds_my();
  set_accel_range(2);
  set_gyro_range(125);
  //print_registers();

  for (;;)
  {
    //printf("Reading imu values...\n");
    //print_accel();
    read_accel = report_accel();
    read_gyro = report_gyro();
    complementary_filter_fusion();
    //print_gyro();
    //float pit = calc_pith();
    // float roll = calc_roll();
    //printf("%.2f,%.2f\n%c", (float)pitch, (float)roll, linefeed);
    printf("%.2f \n", read_accel.temp);
    lastTime = read_accel.timestamp;
    //flash_leds_my();
  }
}