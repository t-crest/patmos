/*
	Test Floating-Point Unit Addressed Mapped

	Author: Lefteris Kyriakakis 
	Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>

#define FPU_BASE          0xf0040000
#define OPERAND_A_LO_REG  *((volatile _IODEV unsigned int *) (FPU_BASE + 0x0))
#define OPERAND_A_HI_REG  *((volatile _IODEV unsigned int *) (FPU_BASE + 0x4))
#define OPERAND_B_LO_REG  *((volatile _IODEV unsigned int *) (FPU_BASE + 0x8))
#define OPERAND_B_HI_REG  *((volatile _IODEV unsigned int *) (FPU_BASE + 0xC))
#define ENABLE_REG        *((volatile _IODEV unsigned int *) (FPU_BASE + 0x10))
#define SEL_OPERATION_REG *((volatile _IODEV unsigned int *) (FPU_BASE + 0x14))
#define ROUND_MODE_REG    *((volatile _IODEV unsigned int *) (FPU_BASE + 0x18))
#define STATUS_REG        *((volatile _IODEV unsigned int *) (FPU_BASE + 0x1C))
#define RESULT_LO_REG     *((volatile _IODEV unsigned int *) (FPU_BASE + 0x20))
#define RESULT_HI_REG     *((volatile _IODEV unsigned int *) (FPU_BASE + 0x24))

// This must be declared outisde of the main function to actually control the FPU
#if defined(ENABLE_HWFPU)
unsigned __USE_HWFPU__ = ENABLE_HWFPU;
#endif

int main()
{
  #if defined(ENABLE_HWFPU)
  unsigned __USE_HWFPU__ = ENABLE_HWFPU;
  #endif
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
    puts("***************************************************************");
  }

  return 0;
}
