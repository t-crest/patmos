/*
    * hello-noc.c
    * 
    * A simple program to test the S4NoC environment.
    * 
    * Author: Martin Schoeberl (martin@jopdesign.com)
*/

#include <stdio.h>
#include <pthread.h>

// Function to be executed by the thread
void* thread_function(void* arg) {
    printf("Hello from the thread!\n");
    return NULL;
}

int main() {
    pthread_t thread;

    // Create a new thread that will run 'thread_function'
    if (pthread_create(&thread, NULL, thread_function, NULL)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    // Wait for the thread to finish
    if (pthread_join(thread, NULL)) {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }

    printf("Hello from the main thread!\n");
    return 0;
}