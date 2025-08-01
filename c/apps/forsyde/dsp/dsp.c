#include <stdio.h>
#include <math.h>
#include <string.h>

#define SIGNAL_SIZE 23
#define MAX_WINDOW_SIZE 4
#define MAX_OUTPUT_SIZE (SIGNAL_SIZE - MAX_WINDOW_SIZE + 1)

typedef int DataType; //To make code analyzable, we need to use integer instead of double. The least significant digit is considered as the decimal of the integer.
typedef enum {
    overload, underload, nominal
} level_t;

typedef struct {
    DataType value;
    level_t label; 
} tagged_item;

#if WCET
__attribute__((noinline))
#endif
void Clipper(DataType* input, DataType* output, int size) {
    #pragma loopbound min SIGNAL_SIZE max SIGNAL_SIZE
    for (int i = 0; i < size; i++) {
        if (input[i] > 15) {
            output[i] = 15;
        } 
        else if (input[i] < -15) {
            output[i] = -15;
        } 
        else {
            output[i] = input[i];
        }
    }
}

#if WCET
__attribute__((noinline))
#endif
void windowSY(int winSize, DataType* input, int inputSize, DataType output[][MAX_WINDOW_SIZE], int* outputSize) {
    *outputSize = inputSize; // Each input element creates a window.
    #pragma loopbound min SIGNAL_SIZE max SIGNAL_SIZE
    for (int i = 0; i < *outputSize; i++) {
        #pragma loopbound min MAX_WINDOW_SIZE max MAX_WINDOW_SIZE
        for (int j = 0; j < winSize; j++) {
            // Fill with zeros if the window index is out of range (before the current input index starts)
            if (i - winSize + 1 + j < 0) {
                output[i][j] = 0;
            } else {
                output[i][j] = input[i - winSize + 1 + j];
            }
        }
    }
}

#if WCET
__attribute__((noinline))
#endif
void SMA(int n, DataType* input, DataType* output, int size) {
    DataType window[MAX_OUTPUT_SIZE][MAX_WINDOW_SIZE];
    int windowSize;

    windowSY(n, input, size, window, &windowSize);
    #pragma loopbound min MAX_WINDOW_SIZE max MAX_WINDOW_SIZE
    for (int i = 0; i < windowSize; i++) {
        DataType sum = 0;
        for (int j = 0; j < n; j++) {
            sum += window[i][j];
        }
        output[i] = sum / n;
    }
}


int isqrt(int n) {
    
    // Start iteration from 1 until the 
    // square of a number exceeds n
    int res = 1;
    #pragma loopbound min MAX_WINDOW_SIZE max MAX_WINDOW_SIZE
    while (res * res <= n) {
        res++;
    }
    
    // return the largest integer whose 
    // square is less than or equal to n
    return res - 1;
}

#if WCET
__attribute__((noinline))
#endif
void RMS(int n, DataType* input, DataType* output, int size) {
    DataType window[MAX_OUTPUT_SIZE][MAX_WINDOW_SIZE];
    int windowSize;

    windowSY(n, input, size, window, &windowSize);
    #pragma loopbound min MAX_WINDOW_SIZE max MAX_WINDOW_SIZE
    for (int i = 0; i < windowSize; i++) {
        DataType sum = 0;
        for (int j = 0; j < n; j++) {
            sum += window[i][j] * window[i][j];
        }
        output[i] = isqrt(sum / n);
    }
}

#if WCET
__attribute__((noinline))
#endif
void Gain(DataType gain, DataType* input, DataType* output, int size) {
    #pragma loopbound min SIGNAL_SIZE max SIGNAL_SIZE
    for (int i = 0; i < size; i++) {
        output[i] = (input[i] * gain) / 10;
    }
}

#if WCET
__attribute__((noinline))
#endif
void Monitor(DataType* input, level_t output[], int size) {
    #pragma loopbound min SIGNAL_SIZE max SIGNAL_SIZE
    for (int i = 0; i < size; i++) {
        if (input[i] > 30) {
            output[i] = overload;
        } else if (input[i] < -30) {
            output[i] = underload;
        } else {
            output[i] = nominal;
        }
    }
}

#if WCET
__attribute__((noinline))
#endif
void Tag(DataType* input, level_t monitor[], tagged_item output[], int size) {
    #pragma loopbound min SIGNAL_SIZE max SIGNAL_SIZE
    for (int i = 0; i < size; i++) {
        output[i].value = input[i];
        output[i].label = monitor[i];
    }
}

void printArray(char *title, DataType* array, int size) {
    #if !WCET
    printf("\n%s: [", title);
    for (int i = 0; i < size; i++) {
        printf("%i.%i, ", array[i]/10, abs(array[i])%10);
        //printf("%02.02f, ", array[i]);
    }
    printf("]\n");
    #endif
}

#if !WCET 
static char string[20];
char *printLevel(level_t level) {
    switch (level)
    {
        case underload: sprintf(string, "%s","Underload"); break;
        case overload:  sprintf(string, "%s","Overload"); break;
        case nominal:   sprintf(string, "%s","Nominal"); break;
        default:        sprintf(string, "%s %i","Error", (int)level); break;
    }
    return string;
}
#endif

#if WCET
__attribute__((noinline))
#endif
void Audio(DataType gain, int n, DataType* input, int size, tagged_item output[], int* outputSize) {
    DataType clipped[SIGNAL_SIZE];
    DataType smoothed[MAX_OUTPUT_SIZE];
    DataType rms[MAX_OUTPUT_SIZE];
    DataType amplified[MAX_OUTPUT_SIZE];
    level_t monitored[MAX_OUTPUT_SIZE];
    printArray("input", input, size);
    Clipper(input, clipped, size); printArray("clipped", clipped, size);
    SMA(n, clipped, smoothed, size); printArray("smoothed", smoothed, size);
    RMS(n, smoothed, rms, size - n + 1); printArray("rms", rms, size);
    Gain(gain, rms, amplified, size - n + 1); printArray("gained", amplified, size);
    Monitor(amplified, monitored, size - n + 1);
    Tag(amplified, monitored, output, size - n + 1);

    *outputSize = size - n + 1;
}

int main() {
   DataType input[SIGNAL_SIZE] = {14, -32, 1, 20, -15, 03, -7, 24, 0, -18, -3, 17, 19, 2, -31, -4, 06, 19, -21, -29, 5, 22, -2};

    tagged_item output[MAX_OUTPUT_SIZE];
    int outputSize;
    DataType gain = 25; 
    int winSize = 4; //max is 4

    Audio(gain, winSize, input, SIGNAL_SIZE, output, &outputSize);
#if !WCET
    for (int i = 0; i < outputSize; i++) {
        printf("Value: %i.%i, Label: %s\n", output[i].value/10, output[i].value%10, printLevel(output[i].label));
    }
#endif

    return 0;
}
