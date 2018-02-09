#include "../led.h"

void led_on(){
	LED = 1;
}

void led_off(){
	LED = 0;
}


void led_on_for(int ms){
	int i,j;
	for(i = 0; i < ms; i++){
		for(j = 0; j < MS_CLOCK; j++){
			led_on();
		}
	}
}

void led_off_for(int ms){
	int i,j;
	for(i = 0; i < ms; i++){
		for(j = 0; j < MS_CLOCK; j++){
			led_off();
		}
	}
}
