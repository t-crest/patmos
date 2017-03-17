#include "libaudio/audio_singlecore.h"
#include "libaudio/audio_singlecore.c"

const int BUFFER_SIZE = 128;

int main() {

    setup(0); //line in

    setInputBufferSize(128);
    setOutputBufferSize(128);

    // enable input
    *audioAdcEnReg = 1;
    //let input buffer fill in before starting to output
    for(unsigned int i=0; i<(BUFFER_SIZE * 1536); i++) { //wait for BUFFER_SIZE samples
        *audioDacEnReg = 0;
    }
    //finally, enable output
    *audioDacEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);


    volatile _SPM short *left;
    volatile _SPM short *right;

    const unsigned int LEFT_ADDR  = 0;
    const unsigned int RIGHT_ADDR = sizeof(int);

    left  = (volatile _SPM short *) LEFT_ADDR;
    right = (volatile _SPM short *) RIGHT_ADDR;


    while(*keyReg != 3) {

        getInputBufferSPM(left, right);
        setOutputBufferSPM(left, right);

    }

    return 0;

}
