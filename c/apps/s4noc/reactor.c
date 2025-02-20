#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include <pthread.h>

typedef int64_t instant_t;
typedef int64_t interval_t;

int _lf_clock_gettime(instant_t* t) {
    if (t != NULL) {
        *t = get_cpu_usecs() * 1000;
    }

    return 0;
}
  
int lf_sleep(interval_t sleep_duration) {
    instant_t now;
    _lf_clock_gettime(&now);
    instant_t wakeup = now + sleep_duration;
  
    // Do busy sleep
    do {
      _lf_clock_gettime(&now);
    } while ((now < wakeup));
    return 0;
}

// Data structure for Source reactor
typedef struct {
    int (*generate_value)();       // Function pointer to generate values
    void (*sender_function)();
    int value;                     // The value to send
    pthread_t thread;
} Source;

// Data structure for Sink reactor
typedef struct {
    void (*receiver_function)();    // Function pointer to handle received values
    pthread_t thread;
} Sink;

// Function for the Source reactor to generate a value
int generate_value() {
    static int counter = 42;
    return counter++; // Incremental values
}

#define MAX_ITER 10
void sender_function() {
    volatile _IODEV int *s4noc_status = (volatile _IODEV int *) PATMOS_IO_S4NOC;
    volatile _IODEV int *s4noc_data   = (volatile _IODEV int *) (PATMOS_IO_S4NOC + 4);
    volatile _IODEV int *s4noc_dest   = (volatile _IODEV int *) (PATMOS_IO_S4NOC + 8);
    
    *s4noc_dest = 2;
    for (int i = 0; i < MAX_ITER; i++) { 
        int status = (*s4noc_status) & 0x01; 
        while (status == 0) { 
           status = (*s4noc_status) & 0x01;
        }
        *s4noc_data = generate_value();  
        lf_sleep(1000000);
    } 
 
}


// Function for the Sink reactor to handle received values
void receiver_function() { 
    volatile _IODEV int *s4noc_status = (volatile _IODEV int *) PATMOS_IO_S4NOC;
    volatile _IODEV int *s4noc_data   = (volatile _IODEV int *) (PATMOS_IO_S4NOC + 4);
    volatile _IODEV int *s4noc_source = (volatile _IODEV int *) (PATMOS_IO_S4NOC + 8);

    for (int i = 0; i < MAX_ITER; i++) { 
    int status = (*s4noc_status) & 0x02;
        while (status == 0) { 
            status = (*s4noc_status) & 0x02;
        } 
        int value = *s4noc_data; 
        int source = *s4noc_source;
        printf("Received: %d from core %d\n", value, source);
    }
}

// Main function
int main() {

    // Initialize the Source reactor
    Source source;
    source.generate_value = generate_value;
    source.sender_function = sender_function;
    source.value = 0;
    
    // Initialize the Sink reactor
    Sink sink;
    sink.receiver_function = receiver_function;

    if (pthread_create(&source.thread, NULL, source.sender_function, NULL)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    if (pthread_create(&sink.thread, NULL, sink.receiver_function, NULL)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    if (pthread_join(source.thread, NULL)) {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }
    if (pthread_join(sink.thread, NULL)) {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }
    return 0;
}
