#include <stdio.h> 
#include <machine/patmos.h>
#include <machine/spm.h>

// Pointers to the IO device registers
volatile _SPM int *readdata_0_ptr          = (volatile _SPM int *) 0xF00b0000;
volatile _SPM int *readdata_1_ptr     	   = (volatile _SPM int *) 0xF00b0004;
volatile _SPM int *readdata_2_ptr    	   = (volatile _SPM int *) 0xF00b0008;
volatile _SPM int *readdata_3_ptr 	   = (volatile _SPM int *) 0xF00b000c;
volatile _SPM int *readdata_4_ptr          = (volatile _SPM int *) 0xF00b0010;
volatile _SPM int *readdata_5_ptr     	   = (volatile _SPM int *) 0xF00b0014;
//volatile _SPM int *readdata_6_ptr    	   = (volatile _SPM int *) 0xF00b0018;
//volatile _SPM int *readdata_7_ptr 	   = (volatile _SPM int *) 0xF00b001c;
//volatile _SPM int *readdata_8_ptr    	   = (volatile _SPM int *) 0xF00b0018;
//volatile _SPM int *readdata_9_ptr 	   = (volatile _SPM int *) 0xF00b001c;


int main(){

	int data=*readdata_0_ptr;

	for(;;) {
 
		printf("The Sensor value : %d \n", data);
	}
	


}
