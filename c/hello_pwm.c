/*
	Test GPIO device by generating a simple PWM of configurable DUTY_CYCLE and PERIOD.

	Author: Lefteris Kyriakakis 
	Copyright: DTU, BSD License
*/
#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>

#define PERIOD 20000
#define HIGH_TIME(DUTY_CYCLE) PERIOD*DUTY_CYCLE
#define LOW_TIME(DUTY_CYCLE) PERIOD-HIGH_TIME(DUTY_CYCLE)

int main(int argc, char **argv){
    volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0090000;
    volatile _SPM int *gpio_ptr = (volatile _SPM int *) 0xF00C0000;
    unsigned long long startTime, elapsedTime;
    int pwmState = 0;
    float dutyCycle = 0.1;
    printf("Enter the desired duty cycle as float: ");
    scanf("%f", &dutyCycle);
    printf("PWM example started with: HIGH_TIME(us)=%.2f, LOW_TIME(us)=%.2f\n", HIGH_TIME(dutyCycle), LOW_TIME(dutyCycle));
    *gpio_ptr = 0x0;
    *led_ptr = 0xFF;
    startTime = get_cpu_usecs();
    while(1){
        switch(pwmState){
        case 0:
            if(get_cpu_usecs()-startTime >= LOW_TIME(dutyCycle)){
                pwmState = 1;
                *gpio_ptr = 0xFFFFFFFF;
                startTime = get_cpu_usecs();
            }
        break;
        case 1:
            if(get_cpu_usecs()-startTime >= HIGH_TIME(dutyCycle)){
                pwmState = 0;
                *gpio_ptr = 0x0;
                startTime = get_cpu_usecs();
            }
        break;
        }
    }
}