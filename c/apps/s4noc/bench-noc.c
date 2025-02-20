/*
    * hello-noc.c
    * 
    * A simple program to test the S4NoC device.
    * 
    * Author: Martin Schoeberl (martin@jopdesign.com)
    * 
    * CPU interface to two ready/valid channels.
  * IO mapping as in classic PC serial port
  * 0: status (control): bit 0 transmit ready, bit 1 rx data available
  * 4: txd and rxd
 * S4NOC:
  * 8: write receiver, read sender
*/

#include <stdio.h>
#include <pthread.h>
#include <machine/patmos.h>
#include <machine/rtc.h>

#define MAX_ITER 10

int time = 0;

// Function to be executed by the thread
void* thread_function(void* arg) {
    // Pointer to the S4NoC device
    volatile _IODEV int *s4noc_status = (volatile _IODEV int *) PATMOS_IO_S4NOC;
    volatile _IODEV int *s4noc_data = (volatile _IODEV int *) (PATMOS_IO_S4NOC + 4);
    volatile _IODEV int *s4noc_dest = (volatile _IODEV int *) (PATMOS_IO_S4NOC + 8);


    printf("Status at core 1: %d\n", *s4noc_status);
    *s4noc_dest = 0;
    time = get_cpu_usecs(); 
    for (int i = 0; i < MAX_ITER; i++) {
       int status = (*s4noc_status) & 0x01;
        while (status == 0) {
           status = (*s4noc_status) & 0x01;
       }
        *s4noc_data = i;
        printf("s\n");
    }

    return NULL;
}

int main() {
    // Pointer to the S4NoC device
    volatile _IODEV int *s4noc_status = (volatile _IODEV int *) PATMOS_IO_S4NOC;
    volatile _IODEV int *s4noc_data = (volatile _IODEV int *) (PATMOS_IO_S4NOC + 4);
    volatile _IODEV int *s4noc_source = (volatile _IODEV int *) (PATMOS_IO_S4NOC + 8);

    pthread_t thread;
    volatile int sum = 0;
    int x = 0;


    // Create a new thread that will run 'thread_function'
    if (pthread_create(&thread, NULL, thread_function, NULL)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    printf("Benchmarking the S4NoC\n"); 

    for (int i = 0; i < MAX_ITER; i++) {
        int status = (*s4noc_status) & 0x02;
        while (status == 0) {
            status = (*s4noc_status) & 0x02;
        }
        sum += *s4noc_data;
        printf("r\n");
        x += i;
    }
    time = get_cpu_usecs() - time;
    printf("\nSum: %d %d in %d us\n", sum, x, time);

    // Wait for the thread to finish
    if (pthread_join(thread, NULL)) {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }
    return 0;
}
