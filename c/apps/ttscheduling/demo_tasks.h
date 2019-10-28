#include "tt_minimal_scheduler.h"
#include <math.h>

#define CPU_PERIOD 12.5
#define LED (*((volatile _IODEV unsigned *)PATMOS_IO_LED))
#define DEAD (*((volatile _IODEV int *) PATMOS_IO_DEADLINE))
#define DEAD_CALC(timeout) (timeout*US_TO_NS/CPU_PERIOD)

long long maxJitter = 0;
unsigned long long sumJitter = 0;

long long task_act(void *self)
{
    unsigned long long now = get_cpu_usecs();
    //Fake work
    DEAD = DEAD_CALC(2304);
    int val = DEAD;
    //Report
    long delta = ((MinimalTTTask *) self)->last_start_time > 0 ? now - ((MinimalTTTask *) self)->last_start_time : 0;
    long jitter = delta > 0 ? delta - ((MinimalTTTask *) self)->period : 0;
#ifdef DEBUG
    printf("ACT(!),\tdelta=%8lld μs,\tjitter=%+4lld μs\n", delta, jitter);
#endif
    if(jitter > maxJitter){
        maxJitter = jitter;
    }
    sumJitter += jitter;
    LED = 0x1;
    return jitter;
}

long long task_snd(void *self)
{
    unsigned long long now = get_cpu_usecs();
    //Fake work
    DEAD = DEAD_CALC(238);
    int val = DEAD;
    //Report
    long delta = ((MinimalTTTask *) self)->last_start_time > 0 ? now - ((MinimalTTTask *) self)->last_start_time : 0;
    long jitter = delta > 0 ? delta - ((MinimalTTTask *) self)->period : 0;
#ifdef DEBUG
    printf("SND(>),\tdelta=%8lld μs,\tjitter=%+4lld μs\n", delta, jitter);
#endif
    if(jitter > maxJitter){
        maxJitter = jitter;
    }
    sumJitter += jitter;
    LED = 0x2;
    return jitter;
}

long long task_rcv(void *self)
{
    unsigned long long now = get_cpu_usecs();
    //Fake work
    DEAD = DEAD_CALC(300);
    int val = DEAD;
    //Report
    long delta = ((MinimalTTTask *) self)->last_start_time > 0 ? now - ((MinimalTTTask *) self)->last_start_time : 0;
    long jitter = delta > 0 ? delta - ((MinimalTTTask *) self)->period : 0;
#ifdef DEBUG
    printf("RCV(<),\tdelta=%8lld μs,\tjitter=%+4lld μs\n", delta, jitter);
#endif
    if(jitter > maxJitter){
        maxJitter = jitter;
    }
    sumJitter += jitter;
    LED = 0x4;
    return jitter;
}

long long task_mon(void *self)
{
    unsigned long long now = get_cpu_usecs();
    //Fake work
    DEAD = DEAD_CALC(4);
    int val = DEAD;
    //Report
    long delta = ((MinimalTTTask *) self)->last_start_time > 0 ? now - ((MinimalTTTask *) self)->last_start_time : 0;
    long jitter = delta > 0 ? delta - ((MinimalTTTask *) self)->period : 0;
#ifdef DEBUG
    printf("MON(?),\tdelta=%8lld μs,\tjitter=%+4lld μs\n", delta, jitter);
#endif
    if(jitter > maxJitter){
        maxJitter = jitter;
    }
    sumJitter += jitter;
    LED = 0x8;
    return jitter;
}

long long task_syn(void *self)
{
    unsigned long long now = get_cpu_usecs();
    //Fake work
    DEAD = DEAD_CALC(475);
    int val = DEAD;
    //Report
    long delta = ((MinimalTTTask *) self)->last_start_time > 0 ? now - ((MinimalTTTask *) self)->last_start_time : 0;
    long jitter = delta > 0 ? delta - ((MinimalTTTask *) self)->period : 0;
#ifdef DEBUG
    printf("SYN(=),\tdelta=%8lld μs,\tjitter=%+4lld μs\n", delta, jitter);
#endif
    if(jitter > maxJitter){
        maxJitter = jitter;
    }
    sumJitter += jitter;
    LED = 0x10;
    return jitter;
}