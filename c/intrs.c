#include <machine/patmos.h>
#include <machine/spm.h>

#include <stdio.h>
#include <stdlib.h>

#define LEDS (*((volatile _SPM unsigned *)0xf0000900))

#define EXC_BASE 0xf0000100
#define EXC_STATUS (*((volatile _SPM unsigned *)EXC_BASE+0))
#define EXC_MASK   (*((volatile _SPM unsigned *)EXC_BASE+1))
#define EXC_PEND   (*((volatile _SPM unsigned *)EXC_BASE+2))
#define EXC_SOURCE (*((volatile _SPM unsigned *)EXC_BASE+3))
#define EXC_VEC      ((volatile _SPM unsigned *)EXC_BASE+32)

void fault(void);
void trap(void) __attribute__((naked));
void intr(void) __attribute__((naked));

#define N 1000
#define EXIT_N 100000000

int main(void) {
  // store service routines to exception vector
  for (unsigned i = 0; i < 32; i++) {
	EXC_VEC[i] = (unsigned)&fault;
  }
  EXC_VEC[8] = (unsigned)&trap;
  EXC_VEC[16] = (unsigned)&intr;
  EXC_VEC[17] = (unsigned)&intr;
  EXC_VEC[18] = (unsigned)&intr;
  EXC_VEC[19] = (unsigned)&intr;

  EXC_MASK = 0xffffffff;
  EXC_PEND = 0x00000000;
  EXC_STATUS |= 1;

  volatile unsigned starts = 0;
  volatile unsigned ends = 0;
  volatile unsigned sent = 0;

  for (unsigned k = 0; k < N; k++) {
	starts++;
	for (unsigned i = 0; i < 32; i++) {
	  putchar('@'+i);
	  sent+=i;
	}
	putchar('\n');
	ends++;

	if (sent != 496*(k+1) || starts != ends) {
	  LEDS = 0x55;
	  abort();
	}
  }

  EXC_STATUS &= ~1;
  
  asm volatile("trap 8");

  // trigger illegal operation
  asm volatile(".word 0x04000000"); // illegal operation
  // trigger illegal memory access
  (*((volatile _SPM unsigned *)0xffffffff)) = 0;

  return 0;
}

void fault(void) {
  unsigned source = EXC_SOURCE;
  LEDS = source;

  const char *msg = "FAULT";
  switch(source) {
  case 0: msg = "Illegal operation"; break;
  case 1: msg = "Illegal memory access"; break;
  }
  puts(msg);

  for(unsigned i = 0; i < EXIT_N; i++) {
	asm volatile("");
  }
  abort();
}

void trap(void) {
  asm volatile("sres 48;"
			   // Save general-purpose registers
			   "sws  [1] = $r1;"
			   "sws  [2] = $r2;"
			   "sws  [3] = $r3;"
			   "sws  [4] = $r4;"
			   "sws  [5] = $r5;"
			   "sws  [6] = $r6;"
			   "sws  [7] = $r7;"
			   "sws  [8] = $r8;"
			   "sws  [9] = $r9;"
			   "sws  [10] = $r10;"
			   "sws  [11] = $r11;"
			   "sws  [12] = $r12;"
			   "sws  [13] = $r13;"
			   "sws  [14] = $r14;"
			   "sws  [15] = $r15;"
			   "sws  [16] = $r16;"
			   "sws  [17] = $r17;"
			   "sws  [18] = $r18;"
			   "sws  [19] = $r19;"
			   "sws  [20] = $r20;"
			   "sws  [21] = $r21;"
			   "sws  [22] = $r22;"
			   "sws  [23] = $r23;"
			   "sws  [24] = $r24;"
			   "sws  [25] = $r25;"
			   "sws  [26] = $r26;"
			   "sws  [27] = $r27;"
			   "sws  [28] = $r28;"
			   "sws  [29] = $r29;"
			   "sws  [30] = $r30;"
			   "sws  [31] = $r31;"
			   // Save special registers
			   "mfs  $r1 = $s0;"
			   "mfs  $r2 = $s1;"
			   "mfs  $r3 = $s2;"
			   "mfs  $r4 = $s3;"
			   "mfs  $r5 = $s4;"
			   "mfs  $r6 = $s5;"
			   "mfs  $r7 = $s6;"
			   "mfs  $r8 = $s7;"
			   "mfs  $r9 = $s8;"
			   "mfs  $r10 = $s9;"
			   "mfs  $r11 = $s10;"
			   "mfs  $r12 = $s11;"
			   "mfs  $r13 = $s12;"
			   "mfs  $r14 = $s13;"
			   "mfs  $r15 = $s14;"
			   "mfs  $r16 = $s15;"
			   "sws  [32] = $r1;"
			   "sws  [33] = $r2;"
			   "sws  [34] = $r3;"
			   "sws  [35] = $r4;"
			   "sws  [36] = $r5;"
			   "sws  [37] = $r6;"
			   "sws  [38] = $r7;"
			   "sws  [39] = $r8;"
			   "sws  [40] = $r9;"
			   "sws  [41] = $r10;"
			   "sws  [42] = $r11;"
			   "sws  [43] = $r12;"
			   "sws  [44] = $r13;"
			   "sws  [45] = $r14;"
			   "sws  [46] = $r15;"
			   "sws  [47] = $r16;");

  puts("TRAP");

  asm volatile(// Restore special registers
			   "lws  $r1 = [32];"
			   "lws  $r2 = [33];"
			   "lws  $r3 = [34];"
			   "lws  $r4 = [35];"
			   "lws  $r5 = [36];"
			   "lws  $r6 = [37];"
			   "lws  $r7 = [38];"
			   "lws  $r8 = [39];"
			   "lws  $r9 = [40];"
			   "lws  $r10 = [41];"
			   "lws  $r11 = [42];"
			   "lws  $r12 = [43];"
			   "lws  $r13 = [44];"
			   "lws  $r14 = [45];"
			   "lws  $r15 = [46];"
			   "lws  $r16 = [47];"
			   "nop;"
			   "mts  $s0 = $r1;"
			   "mts  $s1 = $r2;"
			   "mts  $s2 = $r3;"
			   "mts  $s3 = $r4;"
			   "mts  $s4 = $r5;"
			   "mts  $s5 = $r6;"
			   "mts  $s6 = $r7;"
			   "mts  $s7 = $r8;"
			   "mts  $s8 = $r9;"
			   "mts  $s9 = $r10;"
			   "mts  $s10 = $r11;"
			   "mts  $s11 = $r12;"
			   "mts  $s12 = $r13;"
			   "mts  $s13 = $r14;"
			   "mts  $s14 = $r15;"
			   "mts  $s15 = $r16;"
			   // Restore general-purpose registers
			   "lws  $r1 = [1];"
			   "lws  $r2 = [2];"
			   "lws  $r3 = [3];"
			   "lws  $r4 = [4];"
			   "lws  $r5 = [5];"
			   "lws  $r6 = [6];"
			   "lws  $r7 = [7];"
			   "lws  $r8 = [8];"
			   "lws  $r9 = [9];"
			   "lws  $r10 = [10];"
			   "lws  $r11 = [11];"
			   "lws  $r12 = [12];"
			   "lws  $r13 = [13];"
			   "lws  $r14 = [14];"
			   "lws  $r15 = [15];"
			   "lws  $r16 = [16];"
			   "lws  $r17 = [17];"
			   "lws  $r18 = [18];"
			   "lws  $r19 = [19];"
			   "lws  $r20 = [20];"
			   "lws  $r21 = [21];"
			   "lws  $r22 = [22];"
			   "lws  $r23 = [23];"
			   "lws  $r24 = [24];"
			   "lws  $r25 = [25];"
			   "lws  $r26 = [26];"
			   "lws  $r27 = [27];"
			   "lws  $r28 = [28];"
			   "lws  $r29 = [29];"			   
			   "lws  $r30 = [30];"
			   "lws  $r31 = [31];"
			   // Return to exception base/offset
			   "xret;"
			   "nop;"
			   "nop;"
			   "sfree 48;");
}

void intr(void) {
  asm volatile("sres 48;"
			   // Save general-purpose registers
			   "sws  [1] = $r1;"
			   "sws  [2] = $r2;"
			   "sws  [3] = $r3;"
			   "sws  [4] = $r4;"
			   "sws  [5] = $r5;"
			   "sws  [6] = $r6;"
			   "sws  [7] = $r7;"
			   "sws  [8] = $r8;"
			   "sws  [9] = $r9;"
			   "sws  [10] = $r10;"
			   "sws  [11] = $r11;"
			   "sws  [12] = $r12;"
			   "sws  [13] = $r13;"
			   "sws  [14] = $r14;"
			   "sws  [15] = $r15;"
			   "sws  [16] = $r16;"
			   "sws  [17] = $r17;"
			   "sws  [18] = $r18;"
			   "sws  [19] = $r19;"
			   "sws  [20] = $r20;"
			   "sws  [21] = $r21;"
			   "sws  [22] = $r22;"
			   "sws  [23] = $r23;"
			   "sws  [24] = $r24;"
			   "sws  [25] = $r25;"
			   "sws  [26] = $r26;"
			   "sws  [27] = $r27;"
			   "sws  [28] = $r28;"
			   "sws  [29] = $r29;"
			   "sws  [30] = $r30;"
			   "sws  [31] = $r31;"
			   // Save special registers
			   "mfs  $r1 = $s0;"
			   "mfs  $r2 = $s1;"
			   "mfs  $r3 = $s2;"
			   "mfs  $r4 = $s3;"
			   "mfs  $r5 = $s4;"
			   "mfs  $r6 = $s5;"
			   "mfs  $r7 = $s6;"
			   "mfs  $r8 = $s7;"
			   "mfs  $r9 = $s8;"
			   "mfs  $r10 = $s9;"
			   "mfs  $r11 = $s10;"
			   "mfs  $r12 = $s11;"
			   "mfs  $r13 = $s12;"
			   "mfs  $r14 = $s13;"
			   "mfs  $r15 = $s14;"
			   "mfs  $r16 = $s15;"
			   "sws  [32] = $r1;"
			   "sws  [33] = $r2;"
			   "sws  [34] = $r3;"
			   "sws  [35] = $r4;"
			   "sws  [36] = $r5;"
			   "sws  [37] = $r6;"
			   "sws  [38] = $r7;"
			   "sws  [39] = $r8;"
			   "sws  [40] = $r9;"
			   "sws  [41] = $r10;"
			   "sws  [42] = $r11;"
			   "sws  [43] = $r12;"
			   "sws  [44] = $r13;"
			   "sws  [45] = $r14;"
			   "sws  [46] = $r15;"
			   "sws  [47] = $r16;");

  // keep the scheduler from doing something stupid
  asm volatile("");

  // Increment leds
  LEDS += EXC_SOURCE & 0xf;

  asm volatile(// Restore special registers
			   "lws  $r1 = [32];"
			   "lws  $r2 = [33];"
			   "lws  $r3 = [34];"
			   "lws  $r4 = [35];"
			   "lws  $r5 = [36];"
			   "lws  $r6 = [37];"
			   "lws  $r7 = [38];"
			   "lws  $r8 = [39];"
			   "lws  $r9 = [40];"
			   "lws  $r10 = [41];"
			   "lws  $r11 = [42];"
			   "lws  $r12 = [43];"
			   "lws  $r13 = [44];"
			   "lws  $r14 = [45];"
			   "lws  $r15 = [46];"
			   "lws  $r16 = [47];"
			   "nop;"
			   "mts  $s0 = $r1;"
			   "mts  $s1 = $r2;"
			   "mts  $s2 = $r3;"
			   "mts  $s3 = $r4;"
			   "mts  $s4 = $r5;"
			   "mts  $s5 = $r6;"
			   "mts  $s6 = $r7;"
			   "mts  $s7 = $r8;"
			   "mts  $s8 = $r9;"
			   "mts  $s9 = $r10;"
			   "mts  $s10 = $r11;"
			   "mts  $s11 = $r12;"
			   "mts  $s12 = $r13;"
			   "mts  $s13 = $r14;"
			   "mts  $s14 = $r15;"
			   "mts  $s15 = $r16;"
			   // Restore general-purpose registers
			   "lws  $r1 = [1];"
			   "lws  $r2 = [2];"
			   "lws  $r3 = [3];"
			   "lws  $r4 = [4];"
			   "lws  $r5 = [5];"
			   "lws  $r6 = [6];"
			   "lws  $r7 = [7];"
			   "lws  $r8 = [8];"
			   "lws  $r9 = [9];"
			   "lws  $r10 = [10];"
			   "lws  $r11 = [11];"
			   "lws  $r12 = [12];"
			   "lws  $r13 = [13];"
			   "lws  $r14 = [14];"
			   "lws  $r15 = [15];"
			   "lws  $r16 = [16];"
			   "lws  $r17 = [17];"
			   "lws  $r18 = [18];"
			   "lws  $r19 = [19];"
			   "lws  $r20 = [20];"
			   "lws  $r21 = [21];"
			   "lws  $r22 = [22];"
			   "lws  $r23 = [23];"
			   "lws  $r24 = [24];"
			   "lws  $r25 = [25];"
			   "lws  $r26 = [26];"
			   "lws  $r27 = [27];"
			   "lws  $r28 = [28];"
			   "lws  $r29 = [29];"			   
			   "lws  $r30 = [30];"
			   "lws  $r31 = [31];"
			   // Return to exception base/offset
			   "xret;"
			   "nop;"
			   "nop;"
			   "sfree 48;");
}
