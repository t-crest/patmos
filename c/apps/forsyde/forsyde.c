#include <stdio.h>
#include <math.h>
#include <string.h>

#define SIGNAL_SIZE 23
#define MAX_WINDOW_SIZE 4
#define MAX_OUTPUT_SIZE (SIGNAL_SIZE - MAX_WINDOW_SIZE + 1)

void print(double* array, int size) {
    printf("\n[");
    for (int i = 0; i < size; i++) {
        printf("%02.02f, ", array[i]);
    }
    printf("]\n");
}

void dspFilter(double* input, double* output, int size) {
    for (int i = 0; i < size; i++) {
        if (input[i] > 1.5) {
            output[i] = 1.5;
        } else if (input[i] < -1.5) {
            output[i] = -1.5;
        } else {
            output[i] = input[i];
        }
    }
}

void windowSY(int winSize, double* input, int inputSize, double output[][MAX_WINDOW_SIZE], int* outputSize) {
    *outputSize = inputSize; // Each input element creates a window.

    for (int i = 0; i < *outputSize; i++) {
        for (int j = 0; j < winSize; j++) {
            // Fill with zeros if the window index is out of range (before the current input index starts)
            if (i - winSize + 1 + j < 0) {
                output[i][j] = 0.0;
            } else {
                output[i][j] = input[i - winSize + 1 + j];
            }
        }
    }
}

void dspSMA(int n, double* input, double* output, int size) {
    double window[MAX_OUTPUT_SIZE][MAX_WINDOW_SIZE];
    int windowSize;

    windowSY(n, input, size, window, &windowSize);

    for (int i = 0; i < windowSize; i++) {
        double sum = 0;
        for (int j = 0; j < n; j++) {
            sum += window[i][j];
        }
        output[i] = sum / n;
    }
}

void dspRMS(int n, double* input, double* output, int size) {
    double window[MAX_OUTPUT_SIZE][MAX_WINDOW_SIZE];
    int windowSize;

    windowSY(n, input, size, window, &windowSize);

    for (int i = 0; i < windowSize; i++) {
        double sum = 0;
        for (int j = 0; j < n; j++) {
            sum += window[i][j] * window[i][j];
        }
        output[i] = sqrt(sum / n);
    }
}

void dspGain(double gain, double* input, double* output, int size) {
    for (int i = 0; i < size; i++) {
        output[i] = input[i] * gain;
    }
}

void dspMonitor(double* input, char output[][16], int size) {
    for (int i = 0; i < size; i++) {
        if (input[i] > 3.0) {
            strcpy(output[i], "Overload");
        } else if (input[i] < -3.0) {
            strcpy(output[i], "Underload");
        } else {
            strcpy(output[i], "Nominal");
        }
    }
}

void dspTag(double* input, char monitor[][16], double outputValues[], char outputLabels[][16], int size) {
    for (int i = 0; i < size; i++) {
        outputValues[i] = input[i];
        strcpy(outputLabels[i], monitor[i]);
    }
}

void dspAudio(double gain, int n, double* input, int size, double outputValues[], char outputLabels[][16], int* outputSize) {
    double filtered[SIGNAL_SIZE];
    double smoothed[MAX_OUTPUT_SIZE];
    double rms[MAX_OUTPUT_SIZE];
    double amplified[MAX_OUTPUT_SIZE];
    char monitored[MAX_OUTPUT_SIZE][16];

    dspFilter(input, filtered, size); print(filtered, size);
    dspSMA(n, filtered, smoothed, size); print(smoothed, size);
    dspRMS(n, smoothed, rms, size - n + 1); print(rms, size);
    dspGain(gain, rms, amplified, size - n + 1); print(amplified, size);
    dspMonitor(amplified, monitored, size - n + 1);
    dspTag(amplified, monitored, outputValues, outputLabels, size - n + 1);

    *outputSize = size - n + 1;
}

// Example usage
int main() {
   double input[SIGNAL_SIZE] = {1.4, -3.2, 0.1, 2.0, -1.5, 0.3, -0.7, 2.4, 0, -1.8, -0.3, 1.7, 1.9, 0.2, -3.1, -0.4, 0.6, 1.9, -2.1, -2.9, 0.5, 2.2, -0.2};

    double outputValues[MAX_OUTPUT_SIZE];
    char outputLabels[MAX_OUTPUT_SIZE][16];
    int outputSize;
    double gain = 2.5; 
    int winSize = 4; //max is 4

    dspAudio(gain, winSize, input, sizeof(input)/sizeof(double), outputValues, outputLabels, &outputSize);

    for (int i = 0; i < outputSize; i++) {
        printf("Value: %f, Label: %s\n", outputValues[i], outputLabels[i]);
    }

    return 0;
}
