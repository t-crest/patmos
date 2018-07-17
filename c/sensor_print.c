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

	int data0= *readdata_0_ptr;
	int data1= *readdata_1_ptr;
	int data2= *readdata_2_ptr;
	int data3= *readdata_3_ptr;
	int data4= *readdata_4_ptr;
	int data5= *readdata_5_ptr;

	for(;;) {
 
		printf("The Sensor 0 value : %d \n", data0);
		printf("The Sensor 1 value : %d \n", data1);
		printf("The Sensor 2 value : %d \n", data2);
		printf("The Sensor 3 value : %d \n", data3);
		printf("The Sensor 4 value : %d \n", data4);
		printf("The Sensor 5 value : %d \n", data5);
	}
	


}
