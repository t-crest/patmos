#include "tt_minimal_scheduler.h"
#include <math.h>

#define CPU_PERIOD 12.5
#define LED (*((volatile _IODEV unsigned *)PATMOS_IO_LED))
#define DEAD (*((volatile _IODEV int *) PATMOS_IO_DEADLINE))
#define US_TO_CLKS(timeout) (timeout*US_TO_NS/CPU_PERIOD)

uint64_t T1_sched_insts[4] = {1500, 6500, 11500, 16500};
uint64_t T2_sched_insts[2] = {3500, 13500};
uint64_t T3_sched_insts[8] = {0, 2500, 5000, 7500, 10000, 12500, 15000, 17500};
uint64_t T4_sched_insts[1] = {18700};

__attribute__((noinline))
void task_1(const void *self)
{
    //Fake work
    LED = 0x0;
#ifdef DEBUG
    printf("{T1,%lu}", ((MinimalTTTask*) self)->release_inst);
#else
    DEAD = (500000/CPU_PERIOD) - 10;   //clock cycles
    int val = DEAD;
#endif
    LED = 0x1;
}


__attribute__((noinline))
void task_2(const void *self)
{
    //Fake work
    LED = 0x0;
#ifdef DEBUG
    printf("{T2,%lu}", ((MinimalTTTask*) self)->release_inst);
#else
    DEAD = (1000000/CPU_PERIOD) - 10;  //clock cycles
    int val = DEAD;
#endif
    LED = 0x2;
}

__attribute__((noinline))
void task_3(const void *self)
{
    //Fake work
    LED = 0x0;
#ifdef DEBUG
    printf("{T3,%lu}", ((MinimalTTTask*) self)->release_inst);
#else
    DEAD = (150000/CPU_PERIOD) - 10;  //clock cycles
    int val = DEAD;
#endif
    LED = 0x4;
}

__attribute__((noinline))
void task_4(const void *self)
{
    //Fake work
    LED = 0x0;
#ifdef DEBUG
    printf("{T4,%lu}\n", ((MinimalTTTask*) self)->release_inst);
#else
    DEAD = (800000/CPU_PERIOD) - 10;  //clock cycles
    int val = DEAD;
#endif
    LED = 0x4;
}