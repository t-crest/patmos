/*
	Test GPIO devices by generating a simple PWM.

	Author: Lefteris Kyriakakis 
	Copyright: DTU, BSD License
*/
#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>

#define PERIOD 20000
#define DUTY_CYCLE 0.35
#define HIGH_TIME PERIOD*DUTY_CYCLE
#define LOW_TIME PERIOD-HIGH_TIME

int main(int argc, char **argv){
    volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0090000;
    volatile _SPM int *gpio_ptr = (volatile _SPM int *) 0xF00C0000;
    unsigned long long startTime, elapsedTime;
    int pwmState = 0;
    printf("PWM example started with: HIGH_TIME(us)=%.2f, LOW_TIME(us)=%.2f\n", HIGH_TIME, LOW_TIME);
    *gpio_ptr = 0x0;
    *led_ptr = 0xFF;
    startTime = get_cpu_usecs();
    while(1){
        switch(pwmState){
        case 0:
            if(get_cpu_usecs()-startTime >= LOW_TIME){
                pwmState = 1;
                *gpio_ptr = 0xFFFFFFFF;
                startTime = get_cpu_usecs();
            }
        break;
        case 1:
            if(get_cpu_usecs()-startTime >= HIGH_TIME){
                pwmState = 0;
                *gpio_ptr = 0x0;
                startTime = get_cpu_usecs();
            }
        break;
        }
    }
}