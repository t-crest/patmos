#include "libaudio/audio_singlecore.h"
#include "libaudio/audio_singlecore.c"

const int BUFFER_SIZE = 128;

short sineArray[220];

int main() {

    setup(0); //line in

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    // enable input
    *audioAdcEnReg = 1;
    //let input buffer fill in before starting to output
    for(unsigned int i=0; i<(BUFFER_SIZE * 1536); i++) { //wait for BUFFER_SIZE samples
        *audioDacEnReg = 0;
    }
    //finally, enable output
    *audioDacEnReg = 1;


    volatile _SPM short *left;
    volatile _SPM short *right;

    const unsigned int LEFT_ADDR  = 0;
    const unsigned int RIGHT_ADDR = sizeof(int);

    left  = (volatile _SPM short *) LEFT_ADDR;
    right = (volatile _SPM short *) RIGHT_ADDR;


    //Fill sine array:
    for (int i = 0; i < 220; i++) {
        sineArray[i] = 16384*sin(2.0*M_PI* i /220);
    }

    while(*keyReg != 3) {

        for (int i = 0; i < 220; i++) {
            *left = sineArray[i];
            *right = sineArray[i];

            setOutputBufferSPM(left, right);
        }

    }

    return 0;

}
