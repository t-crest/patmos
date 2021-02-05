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

#if defined(USE_HWFPU)
  int USE_PATMOS_FLOAT = USE_HWFPU;
#else
  int USE_PATMOS_FLOAT = 0;
#endif

#ifndef BOOTROM

int main()
{
  union { unsigned long long b; double f; } x;
  union { unsigned long long b; double f; } y;
  volatile unsigned long long startTime;
  volatile unsigned long long endTime;

  puts("Hello, testing floating point unit starts now!");
  printf("Please enter operand A = ");
  scanf("%lf", &x.f);
  printf("Please enter operand B = ");
  scanf("%lf", &y.f);

  for(unsigned i=0; i<1; i++)
  {
    printf("**************************Try = %d******************************\n", i);
    for(unsigned char operation=0; operation<4; operation++)
    {
      union { unsigned long long b; double f; } hard_res;
      union { unsigned long long b; double f; } soft_res;
      puts("---------------------------------------------------------");
      printf("Testing operation %x\n", operation);

      startTime = get_cpu_usecs();
      switch (operation)
      {
      case 0:
        soft_res.f = x.f+y.f;
        break;
      case 1:
        soft_res.f = x.f-y.f;
        break;
      case 2:
        soft_res.f = x.f*y.f;
        break;
      case 3:
        soft_res.f = x.f/y.f;
        break;
      }
      endTime = get_cpu_usecs();
      printf("Soft-Float Result = %f (computed in %llu μs as 0x%llx)\n", soft_res.f, endTime - startTime, soft_res.b);

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
      
      printf("Hard-Float Result = %f (computed in %llu μs as 0x%llx)\n", hard_res.f, endTime - startTime, hard_res.b);

      unsigned char underflow = (unsigned char) ((STATUS_REG & 0x20) >> 5);
      unsigned char overflow = (unsigned char) ((STATUS_REG & 0x16) >> 4);
      unsigned char inexact = (unsigned char) ((STATUS_REG & 0x8) >> 3);
      unsigned char exception = (unsigned char) ((STATUS_REG & 0x4) >> 2);
      unsigned char invalid = (unsigned char) ((STATUS_REG & 0x2) >> 1);
      puts(
        "FPU Flags: |EXC|UND|OVR|INX|INV|");
      printf(
        "           |  %u|  %u|  %u|  %u|  %u|\n", exception, underflow, overflow, inexact, invalid);
    }
    puts("*********************************************************");
  }

  return 0;
}

#else

#include "include/bootable.h"



int main()
{

  union { unsigned long long b; double f; } x;
  union { unsigned long long b; double f; } y;

  LEDS = 0x100;

  x.f = 10.75;
  y.f = 1.6;

  for(unsigned char operation=0; operation<4; operation++)
  {
    union { unsigned long long b; double f; } hard_res;
    union { unsigned long long b; double f; } soft_res;
    union { unsigned long long b; double f; } correct;

    LEDS = 0x100;

    switch (operation)
    {
    case 0:
      soft_res.f = x.f+y.f;
      correct.f = 12.35;
      break;
    case 1:
      soft_res.f = x.f-y.f;
      correct.f = 9.15;
      break;
    case 2:
      soft_res.f = x.f*y.f;
      correct.f = 17.2;
      break;
    case 3:
      soft_res.f = x.f/y.f;
      correct.f = 6.71875;
      break;
    }

    LEDS = 0x10F;

    OPERAND_A_LO_REG = (x.b & 0xFFFFFFFF);
    OPERAND_A_HI_REG = (x.b >> 32);
    OPERAND_B_LO_REG = (y.b & 0xFFFFFFFF);
    OPERAND_B_HI_REG = (y.b >> 32);
    SEL_OPERATION_REG = operation;
    ENABLE_REG = 0x1;
    while((STATUS_REG & 0x1) != 0x1) {continue;}
    hard_res.b = ((unsigned long long) RESULT_HI_REG << 32) + (unsigned long long) RESULT_LO_REG;

    LEDS = 0x1FF;

    if((soft_res.f - hard_res.f) == 0)
    {
      LEDS = 1;
    } else {
      LEDS = 0;
    }
  }
}

#endif