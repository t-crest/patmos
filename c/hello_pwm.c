/*
	Test GPIO device by generating a software PWM of variable DUTY_CYCLE 
    with no jitter, using the DEADLINE device 

	Author: Lefteris Kyriakakis 
	Copyright: DTU, BSD License
*/
#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>

#define NS_TO_SEC 0.000000001
#define NS_TO_USEC 0.001
#define USEC_TO_NS 1000
#define USEC_TO_SEC 0.000001
#define SEC_TO_NS 1000000000
#define SEC_TO_USEC 1000000
#define SEC_TO_HOUR 0.000277777778

#define PWM_PERIOD 20000
#define MIN_CYCLE 0.015
#define MAX_CYCLE 0.1
#define MOTOR_STEP 0.001

#define HIGH_TIME(DUTY_CYCLE) PWM_PERIOD*DUTY_CYCLE
#define LOW_TIME(DUTY_CYCLE) PWM_PERIOD-HIGH_TIME(DUTY_CYCLE)
#define DEAD_COMP 0
#define DEAD_CALC(DUTY_CYCLE, CPU_PERIOD) (HIGH_TIME(dutyCycle)*USEC_TO_NS/CPU_PERIOD)-DEAD_COMP

#define DISP_SYM_MASK 0x80

volatile _IODEV int *led_ptr = (volatile _SPM int *) PATMOS_IO_LED;
volatile _IODEV int *gpio_ptr = (volatile _SPM int *) PATMOS_IO_GPIO;
volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;
volatile _SPM unsigned *disp_ptr = (volatile _SPM unsigned *) PATMOS_IO_SEGDISP;

void printSegmentInt(unsigned number) {
    *(disp_ptr+0) = number & 0xF;
    *(disp_ptr+1) = (number >> 4) & 0xF;
    *(disp_ptr+2) = (number >> 8) & 0xF;
    *(disp_ptr+3) = (number >> 12) & 0xF;
    *(disp_ptr+4) = (number >> 16) & 0xF;
    *(disp_ptr+5) = (number >> 20) & 0xF;
    *(disp_ptr+6) = (number >> 24) & 0xF;
    *(disp_ptr+7) = (number >> 28) & 0xF;
}
int main(int argc, char **argv){
    puts("Test GPIO device by generating a software PWM of variable DUTY_CYCLE");
    int pwmState = 0;
    float dutyCycle = 0.1;
    float cpuPeriod = (1.0f/get_cpu_freq()) * SEC_TO_NS;
    *gpio_ptr = 0x0;
    *led_ptr = 0xFF;
    volatile unsigned long long elapsedTime = 0;
    volatile unsigned long long startTime = get_cpu_usecs();
    while(1){
        elapsedTime = get_cpu_usecs() - startTime;                                      //calculate elapsed time
        if(elapsedTime >= PWM_PERIOD - 10){
            startTime = get_cpu_usecs();
            int val = 0;
            *gpio_ptr = 0x1;                                                            //signal high time
            *dead_ptr = DEAD_CALC(dutyCycle, cpuPeriod);                                //wait
            val = *dead_ptr;                                            
            *gpio_ptr = 0x0;                                                            //signal low time
            dutyCycle = (dutyCycle >= MAX_CYCLE) ? MIN_CYCLE : dutyCycle + MOTOR_STEP;  //increase dutyCycle for sweep
        } else {
            printSegmentInt(DEAD_CALC(dutyCycle, cpuPeriod));
        }
    }
}