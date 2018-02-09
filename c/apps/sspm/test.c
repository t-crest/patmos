#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include "../../libcorethread/corethread.h"
#include "../../libmp/mp.h"
#include "../../libmp/mp_internal.h"
#include "sspm_properties.h"
#include "atomic.h"
#include "led.h"

void slave(void* args){
	led_on();
}


int main(){
	printf("cores: %d\n", NOC_CORES);
	for(int i = 1; i<NOC_CORES; i++){
		printf("starting core %d\n", i);
		corethread_create(i, &slave, NULL);
		printf("core %d started\n", i);
	}
}
