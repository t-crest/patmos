#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Constants
#define SIGNAL_SIZE 10

// Signal struct
typedef struct {
    int values[SIGNAL_SIZE];
    int size;
} Signal;

typedef enum {
    danger, alarm, normal
} level_t;

// Function to create a signal
#if WCET
__attribute__((noinline))
#endif
Signal createSignal(int start, int end, int step) {
    Signal signal;
    signal.size = 0;
    for (int i = start; i <= end; i += step) {
        signal.values[signal.size++] = i;
    }
    return signal;
}

// Function for zipWithSY equivalent
#if WCET
__attribute__((noinline))
#endif
Signal zipWithProcessor(const Signal* signal1, const Signal* signal2) {
    Signal result;
    result.size = 0;

    int minSize = (signal1->size < signal2->size) ? signal1->size : signal2->size;
    #pragma loopbound min SIGNAL_SIZE max SIGNAL_SIZE
    for (int i = 0; i < minSize; i++) {
        int proc1 = signal1->values[i] + signal2->values[i];
        int proc2 = proc1 * 2; // Equivalent to proc2(proc1(x, y))
        result.values[result.size++] = proc2;
    }

    return result;
}

// Function for mapSY equivalent to compute brake signal
#if WCET
__attribute__((noinline))
#endif
Signal brake(int threshold, const Signal* input) {
    Signal result;
    result.size = input->size;
    #pragma loopbound min SIGNAL_SIZE max SIGNAL_SIZE
    for (int i = 0; i < input->size; i++) {
        result.values[i] = input->values[i] > threshold; // Boolean stored as integer
    }

    return result;
}

// Function for mapSY equivalent to display signals
#if WCET
__attribute__((noinline))
#endif
void display(const Signal* input, char output[SIGNAL_SIZE][20]) {
    #pragma loopbound min SIGNAL_SIZE max SIGNAL_SIZE
    for (int i = 0; i < input->size; i++) {
        memset(output[i], 0, 16);
        if (input->values[i] > 5) {
            output[i][0]='D';
            //strcpy(output[i], "DANGER!");
        } else if (input->values[i] > 3) {
            output[i][0]='A';
            //strcpy(output[i], "ALARM");
        } else {
            output[i][0]='O';
            //sprintf(output[i], "%d", input->values[i]);
        }
    }
}

// Main function to demonstrate functionality
#if WCET
__attribute__((noinline))
#endif
int main() {
    // Create lidar and camera signals
    Signal lidar = createSignal(1, 10, 1);     // [1, 2, 3, ..., 10]
    Signal camera = createSignal(2, 20, 2);    // [2, 4, 6, ..., 20]

    // Process signals using adasproc
    Signal processed = zipWithProcessor(&lidar, &camera);

    // Generate brake signals with a threshold of 15
    Signal brakeSignal = brake(15, &processed);

    // Display the processed signal
    char displayOutput[SIGNAL_SIZE][20];
    display(&processed, displayOutput);
#if !WCET
    // Print results
    printf("Processed Signal: ");
    for (int i = 0; i < processed.size; i++) {
        printf("%d ", processed.values[i]);
    }
    printf("\n");

    printf("Brake Signal: ");
    for (int i = 0; i < brakeSignal.size; i++) {
        printf("%s ", brakeSignal.values[i] ? "true" : "false");
    }
    printf("\n");

    printf("Display Output: ");
    for (int i = 0; i < processed.size; i++) {
        printf("%s ", displayOutput[i]);
    }
    printf("\n");
#endif
    return 0;
}
