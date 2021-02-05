#include "tt_minimal_scheduler.h"
#include <math.h>

#define CPU_PERIOD 12.5
#define LED (*((volatile _IODEV unsigned *)PATMOS_IO_LED))
#define DEAD (*((volatile _IODEV int *) PATMOS_IO_DEADLINE))

__attribute__((noinline))
void task_1(const void *self)
{
    //Fake work
    if(get_cpuid() == 0) LED = 0x1;
#ifdef DEBUG
    printf("EXEC {T1,%lu}", ((MinimalTTTask*) self)->release_inst);
#else
    DEAD = (unsigned)(250000/CPU_PERIOD) - 10;   //clock cycles
    int val = DEAD;
#endif
    if(get_cpuid() == 0) LED = 0x0;
}


__attribute__((noinline))
void task_2(const void *self)
{
    //Fake work
    if(get_cpuid() == 0) LED = 0x2;
#ifdef DEBUG
    printf("EXEC {T2,%lu}", ((MinimalTTTask*) self)->release_inst);
#else
    DEAD = (unsigned)(500000/CPU_PERIOD) - 10;  //clock cycles
    int val = DEAD;
#endif
    if(get_cpuid() == 0) LED = 0x0;
}

__attribute__((noinline))
void task_3(const void *self)
{
    //Fake work
    if(get_cpuid() == 0) LED = 0x4;
#ifdef DEBUG
    printf("EXEC {T3,%lu}", ((MinimalTTTask*) self)->release_inst);
#else
    DEAD = (unsigned)(200000/CPU_PERIOD) - 10;  //clock cycles
    int val = DEAD;
#endif
    if(get_cpuid() == 0) LED = 0x0;
}

__attribute__((noinline))
void task_4(const void *self)
{
    //Fake work
    if(get_cpuid() == 0) LED = 0x8;
#ifdef DEBUG
    printf("EXEC {T4,%lu}\n", ((MinimalTTTask*) self)->release_inst);
#else
    DEAD = (unsigned)(500000/CPU_PERIOD) - 10;  //clock cycles
    int val = DEAD;
#endif
    if(get_cpuid() == 0) LED = 0x0;
}

__attribute__((noinline))
void task_5(const void *self)
{
    //Fake work
    if(get_cpuid() == 0) LED = 0x10;
#ifdef DEBUG
    printf("EXEC {T5,%lu}", ((MinimalTTTask*) self)->release_inst);
#else
    DEAD = (unsigned)(825000/CPU_PERIOD) - 10;   //clock cycles
    int val = DEAD;
#endif
    if(get_cpuid() == 0) LED = 0x0;
}


__attribute__((noinline))
void task_6(const void *self)
{
    //Fake work
    if(get_cpuid() == 0) LED = 0x20;
#ifdef DEBUG
    printf("EXEC {T6,%lu}", ((MinimalTTTask*) self)->release_inst);
#else
    DEAD = (unsigned)(1000000/CPU_PERIOD) - 10;  //clock cycles
    int val = DEAD;
#endif
    if(get_cpuid() == 0) LED = 0x0;
}

__attribute__((noinline))
void task_7(const void *self)
{
    //Fake work
    if(get_cpuid() == 0) LED = 0x40;
#ifdef DEBUG
    printf("EXEC {T7,%lu}", ((MinimalTTTask*) self)->release_inst);
#else
    DEAD = (unsigned)(640000/CPU_PERIOD) - 10;  //clock cycles
    int val = DEAD;
#endif
    if(get_cpuid() == 0) LED = 0x0;
}

__attribute__((noinline))
void task_8(const void *self)
{
    //Fake work
    if(get_cpuid() == 0) LED = 0x80;
#ifdef DEBUG
    printf("EXEC {T8,%lu}\n", ((MinimalTTTask*) self)->release_inst);
#else
    DEAD = (unsigned)(1500000/CPU_PERIOD) - 10;  //clock cycles
    int val = DEAD;
#endif
    if(get_cpuid() == 0) LED = 0x0;
}