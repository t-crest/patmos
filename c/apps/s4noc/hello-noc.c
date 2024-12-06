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

// Function to be executed by the thread
void* thread_function(void* arg) {
    // Pointer to the S4NoC device
    volatile _IODEV int *s4noc_status = (volatile _IODEV int *) PATMOS_IO_S4NOC;
    volatile _IODEV int *s4noc_data = (volatile _IODEV int *) (PATMOS_IO_S4NOC + 4);
    volatile _IODEV int *s4noc_dest = (volatile _IODEV int *) (PATMOS_IO_S4NOC + 8);

    // TX should be ready
    printf("Status at core 1: %d\n", *s4noc_status);
    int status = (*s4noc_status) & 0x01;
    // We can wait till TX ready
    // However, a send to a not ready channel will block till ready
    // So polling is optional
    while (status == 0) {
        status = (*s4noc_status) & 0x01;
    }
    printf("Core 1 sending 42 to core 0\n");
    // Set destination to core 0
    // If you only send to the same core, this needs only be set once
    *s4noc_dest = 0;
    // Send a value
    *s4noc_data = 42;

    return NULL;
}

int main() {
    // Pointer to the S4NoC device
    volatile _IODEV int *s4noc_status = (volatile _IODEV int *) PATMOS_IO_S4NOC;
    volatile _IODEV int *s4noc_data = (volatile _IODEV int *) (PATMOS_IO_S4NOC + 4);
    volatile _IODEV int *s4noc_source = (volatile _IODEV int *) (PATMOS_IO_S4NOC + 8);

    pthread_t thread;


    // Create a new thread that will run 'thread_function'
    if (pthread_create(&thread, NULL, thread_function, NULL)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    // Note that printf does at the meoment NOT use a lock, therefore
    // the output from the main thread and the new thread might be mixed
    // We might use a lock here
    printf("Hello from the main thread!\n"); 
    printf("Status at core 1: %d\n", *s4noc_status);
    int status = (*s4noc_status) & 0x02;
    // We can wait till RX is ready
    // However, a read from a not ready channel will block till a value is there
    // Polling for ready is optional
    while (status == 0) {
        status = (*s4noc_status) & 0x02;
    }
    // Read the source
    int source = *s4noc_source;
    // Read the value
    int value = *s4noc_data;
    printf("Received: %d from core %d\n", value, source);

    // Wait for the thread to finish
    if (pthread_join(thread, NULL)) {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }
    return 0;
}