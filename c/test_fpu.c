/*
	Test Floating-Point Unit Addressed Mapped

	Author: Lefteris Kyriakakis 
	Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>

#define FPU_BASE          0xf00e0000
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

volatile _SPM int *led_ptr = (volatile _SPM int *) PATMOS_IO_LED;

int main(int argc, char **argv)
{
  
  union { unsigned long long b; double f; } x;
  union { unsigned long long b; double f; } y;
  union { unsigned long long b; double f; } hard_res;
  double soft_res = 0.0;
  unsigned operation = 0;
  unsigned long long startTime;
  unsigned long long endTime;

  puts("Hello, testing hello floating point unit starts now!");
  puts("Available modes are:");
  puts("0 = add\n1 = sub\n 2 = mul\n 3 = div\n");
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
  OPERAND_A_LO_REG = ((unsigned long long) x.b & 0xFFFFFFFF);
  OPERAND_A_HI_REG = ((unsigned long long) x.b >> 32);
  OPERAND_B_LO_REG = ((unsigned long long) y.b & 0xFFFFFFFF);
  OPERAND_B_HI_REG = ((unsigned long long) y.b >> 32);
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
