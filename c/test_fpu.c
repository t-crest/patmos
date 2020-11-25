/*
	Test Floating-Point Unit Addressed Mapped

	Author: Lefteris Kyriakakis 
	Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/spm.h>
#include <machine/exceptions.h>
#include <machine/rtc.h>

#define FPU_BASE          0xf0040000
#define OPERAND_A_LO_REG  *((volatile _SPM unsigned int *) (FPU_BASE + 0x0))
#define OPERAND_A_HI_REG  *((volatile _SPM unsigned int *) (FPU_BASE + 0x4))
#define OPERAND_B_LO_REG  *((volatile _SPM unsigned int *) (FPU_BASE + 0x8))
#define OPERAND_B_HI_REG  *((volatile _SPM unsigned int *) (FPU_BASE + 0xC))
#define ENABLE_REG        *((volatile _SPM unsigned int *) (FPU_BASE + 0x10))
#define SEL_OPERATION_REG *((volatile _SPM unsigned int *) (FPU_BASE + 0x14))
#define ROUND_MODE_REG    *((volatile _SPM unsigned int *) (FPU_BASE + 0x18))
#define STATUS_REG        *((volatile _SPM unsigned int *) (FPU_BASE + 0x1C))
#define RESULT_LO_REG     *((volatile _SPM unsigned int *) (FPU_BASE + 0x20))
#define RESULT_HI_REG     *((volatile _SPM unsigned int *) (FPU_BASE + 0x24))

#ifndef BOOTROM

int main()
{
  
  union { unsigned long long b; double f; } x;
  union { unsigned long long b; double f; } y;
  union { unsigned long long b; double f; } hard_res;
  double soft_res = 0.0;
  unsigned operation = 0;
  unsigned long long startTime;
  unsigned long long endTime;

  puts("Hello, testing floating point unit starts now!");
  puts("Available modes are:");
  puts("0 = add\n1 = sub\n2 = mul\n3 = div");
  printf("Please enter operand A = ");
  scanf("%lf", &x.f);
  printf("Please enter operand B = ");
  scanf("%lf", &y.f);
  printf("Please enter operation = ");
  scanf("%d", &operation);

  startTime = get_cpu_usecs();
  switch (operation)
  {
  case 0:
    soft_res = x.f+y.f;
    break;
  case 1:
    soft_res = x.f-y.f;
    break;
  case 2:
    soft_res = x.f*y.f;
    break;
  case 3:
    soft_res = x.f/y.f;
    break;
  }
  endTime = get_cpu_usecs();

  printf("Soft-Float Result = %f (computed in %llu us)\n", soft_res, endTime - startTime);

  startTime = get_cpu_usecs();
  OPERAND_A_LO_REG = (x.b & 0xFFFFFFFF);
  OPERAND_A_HI_REG = (x.b >> 32);
  OPERAND_B_LO_REG = (y.b & 0xFFFFFFFF);
  OPERAND_B_HI_REG = (y.b >> 32);
  ROUND_MODE_REG = 0x0;
  SEL_OPERATION_REG = operation & 0xF;
  ENABLE_REG = 0x1;
  while((STATUS_REG & 0x1) != 0x1){continue;}
  hard_res.b = ((unsigned long long) RESULT_HI_REG << 32) + (unsigned long long) RESULT_LO_REG;
  endTime = get_cpu_usecs();
  
  printf("Hard-Float Result = %f (computed in %llu us)\n", hard_res.f, endTime - startTime);

  unsigned char underflow = (unsigned char) ((STATUS_REG & 0x20) >> 5);
  unsigned char overflow = (unsigned char) ((STATUS_REG & 0x16) >> 4);
  unsigned char inexact = (unsigned char) ((STATUS_REG & 0x8) >> 3);
  unsigned char exception = (unsigned char) ((STATUS_REG & 0x4) >> 2);
  unsigned char invalid = (unsigned char) ((STATUS_REG & 0x2) >> 1);

  printf("FPU exception is = %u\n", exception);
  printf("FPU underflow is = %u\n", underflow);
  printf("FPU overflow  is = %u\n", overflow);
  printf("FPU inexact   is = %u\n", inexact);
  printf("FPU invalid   is = %u\n", invalid);

  return 0;
}

#else

#include "include/bootable.h"

int main()
{
  union { unsigned long long b; double f; } x;
  union { unsigned long long b; double f; } y;
  union { unsigned long long b; double f; } hard_res;
  register union { unsigned long long b; double f; } soft_res;

  x.b = 0x0;
  y.b = 0x0;
  hard_res.b = 0x0;
  soft_res.b = 0x0;

  LEDS = 0x100;
  
  x.f = 2.0;
  y.f = 1.5;

  LEDS = 0x10F;

  soft_res.f = x.f * y.f;

  LEDS = 0x1FF;
  OPERAND_A_LO_REG = (x.b & 0xFFFFFFFF);
  OPERAND_A_HI_REG = (x.b >> 32);
  OPERAND_B_LO_REG = (y.b & 0xFFFFFFFF);
  OPERAND_B_HI_REG = (y.b >> 32);
  SEL_OPERATION_REG = 0x2;
  ENABLE_REG = 0x1;
  while((STATUS_REG & 0x1) != 0x1) {continue;}
  hard_res.b = ((unsigned long long) RESULT_HI_REG << 32) + (unsigned long long) RESULT_LO_REG;

  LEDS = 0x100;

  if((soft_res.b - hard_res.b) == 0)
  {
    LEDS = 1;
    return 1;
  } else {
    LEDS = 0;
    return 0;
  }
}

#endif