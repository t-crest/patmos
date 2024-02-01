/*
    Demo program for showing the effect of caching on execution time.
	
	Usage:
		Intended for use with single-core patmos on a DE2-115.
		
		LED8 is initially on, showing program is ready. All other LED start disabled.
		
		Pressing Button0 (key0) will start the test loop, which enable LEDs 0-3 sequentially.
		When LED3 is enabled the loop is done. This should take ~10 seconds.
		The number of cycles spent on the run will also be printed at the end.
		
		Pressing Button1 will start the same loop, but now using the cache instead directly accessing main memory.
		Otherwise dos the same.
		Should take ~5.8 seconds for LED3 to be enabled.
		
		After each run, Button3 can be pressed to reset the LEDs before the next run.
		
		The "run" is intended to be WCET analyzed:

		~/t-crest/patmos/c$ patmos-clang wcet_demo.c -O2 -mserialize=wcet_demo.pml -I.
		~/t-crest/patmos/c$ platin wcet -b a.out -i wcet_demo.pml -e run --report

		Platin should say it will take ~816 million cycles.
*/

#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>

// this is roughly 2-3 seconds on single-core
#define LOOP_ITER 4000000
#define TEST_LEDS 4

// Acces to LEDs
volatile _SPM int *led_ptr  = (volatile _SPM int *) PATMOS_IO_LED;

// Access to Keys (buttons)
volatile _SPM int *key_ptr  = (volatile _SPM int *) PATMOS_IO_KEYS;

// Tracks what LEDS are on/off
int LED_STATUS=0;

// Used by the test loop.
volatile int DUMMY=123;

// Enables the the LED with the given index
void enable_led(int id){
	LED_STATUS |= 1 << id;
	*led_ptr = LED_STATUS;
}

// disables the the LED with the given index
void disable_led(int id){
	LED_STATUS &= ~(1 << id);
	*led_ptr = LED_STATUS;
}

// Enables the LEDs 0-8
void enable_all_leds(){
	for(int i=0; i<9; i++){
		enable_led(i);
	}
}

// Disables the LEDs 0-8
void disable_all_leds(){
	for(int i=0; i<9; i++){
		disable_led(i);
	}
}

// Returns whether the key (button) with the given index is currently pressed.
int key_pressed(int id){
	return !(*key_ptr & (1 << id));
}

// Runs the test loop and finishes by enabling the LED with the given index
//
// If use_cache, the loop will run using the cache
void run_x(int id, int use_cache){
	if(use_cache){
		#pragma loopbound min LOOP_ITER max LOOP_ITER
		for (int i = 0; i<LOOP_ITER; i++) {
				DUMMY++;
		}
	} else {
		#pragma loopbound min LOOP_ITER max LOOP_ITER
		for (int i = 0; i<LOOP_ITER; i++) {
			volatile _UNCACHED  int* t = (volatile _UNCACHED  int*) &DUMMY;
			(*t)++;
		}
	}
	
	enable_led(id);
}

// Runs the test loop for LEDs 0-3
//
// If use_cache, the loop will run using the cache
void run(int use_cache) __attribute__((noinline));
void run(int use_cache){
	#pragma loopbound min TEST_LEDS max TEST_LEDS
	for (int i = 0; i<TEST_LEDS; i++) {
		run_x(i, use_cache);
	}
}

// Runs the full test and times the number of cycles used, printing them.
void run_timed(int use_cache){
	unsigned long long cycles_start = get_cpu_cycles();
	run(use_cache);
	unsigned long long cycles_end = get_cpu_cycles();
	printf("Done: %llu\n", cycles_end-cycles_start);
}

// Disables all LEDs except 8
void reset(){	
	disable_all_leds();
	enable_led(8);
}

int main() {
	
	int status=0;
	
	reset();
	
	for (;;) {
		
		if(status==0) {
			// waiting
			if(key_pressed(0)){
				printf("Run Uncached\n");
				run_timed(0);
				status=1;
			} else if(key_pressed(1)){
				printf("Run Cached\n");
				run_timed(1);
				status=1;
			}
		} else if(status==1){
			// Done, waiting
			if(key_pressed(3)){
				printf("Reset\n");
				reset();
				status=0;
			}
		}

	}
}