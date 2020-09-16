#include "rm_minimal_scheduler.h"
#include <math.h>

#define CPU_PERIOD 12.5
#define LED (*((volatile _IODEV unsigned *)PATMOS_IO_LED))
#define DEAD (*((volatile _IODEV int *) PATMOS_IO_DEADLINE))

__attribute__((noinline))
void task_1(const void *self)
{
    if(get_cpuid() == 0) LED = 0x1;
#ifdef DEBUG
    printf("{t_%u, #%lu, r = %llu}\n", ((MinimalRMTask*) self)->id, ((MinimalRMTask*) self)->exec_count, ((MinimalRMTask*) self)->release_time);
#else
    // Fake work
    DEAD = ((MinimalRMTask*) self)->wcet;   //clock cycles
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
    printf("{t_%u, #%lu, r = %llu}\n", ((MinimalRMTask*) self)->id, ((MinimalRMTask*) self)->exec_count, ((MinimalRMTask*) self)->release_time);
#else
    DEAD = ((MinimalRMTask*) self)->wcet;   //clock cycles
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
    printf("{t_%u, #%lu, r = %llu}\n", ((MinimalRMTask*) self)->id, ((MinimalRMTask*) self)->exec_count, ((MinimalRMTask*) self)->release_time);
#else
    DEAD = ((MinimalRMTask*) self)->wcet;   //clock cycles
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
    printf("{t_%u, #%lu, r = %llu}\n", ((MinimalRMTask*) self)->id, ((MinimalRMTask*) self)->exec_count, ((MinimalRMTask*) self)->release_time);
#else
    DEAD = ((MinimalRMTask*) self)->wcet;   //clock cycles
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
    printf("{t_%u, #%lu, r = %llu}\n", ((MinimalRMTask*) self)->id, ((MinimalRMTask*) self)->exec_count, ((MinimalRMTask*) self)->release_time);
#else
    DEAD = ((MinimalRMTask*) self)->wcet;   //clock cycles
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
    printf("{t_%u, #%lu, r = %llu}\n", ((MinimalRMTask*) self)->id, ((MinimalRMTask*) self)->exec_count, ((MinimalRMTask*) self)->release_time);
#else
    DEAD = ((MinimalRMTask*) self)->wcet;   //clock cycles
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
    printf("{t_%u, #%lu, r = %llu}\n", ((MinimalRMTask*) self)->id, ((MinimalRMTask*) self)->exec_count, ((MinimalRMTask*) self)->release_time);
#else
    DEAD = ((MinimalRMTask*) self)->wcet;   //clock cycles
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
    printf("{t_%u, #%lu, r = %llu}\n", ((MinimalRMTask*) self)->id, ((MinimalRMTask*) self)->exec_count, ((MinimalRMTask*) self)->release_time);
#else
    DEAD = ((MinimalRMTask*) self)->wcet;   //clock cycles
    int val = DEAD;
#endif
    if(get_cpuid() == 0) LED = 0x0;
}