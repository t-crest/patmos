#include "libaudio/audio_singlecore.h"
#include "libaudio/audio_singlecore.c"

#include <stdio.h>
#include <stdlib.h>

const int BUFFER_SIZE = 128;

int main() {

    //setup(0); //line in
    int init_flag = 0;
    init_flag = ADAU1761_init(1); //line in

    if (init_flag != 0)
    {
        printf("ADAU1761_init failed.\n");
    }else{
        printf("ADAU1761_init succeded.\n");
    }

    //setInputBufferSize(BUFFER_SIZE);
    //setOutputBufferSize(BUFFER_SIZE);

    // enable input
    //*audioAdcEnReg = 1;
    //let input buffer fill in before starting to output
    //for(unsigned int i=0; i<(BUFFER_SIZE * 1536); i++) { //wait for BUFFER_SIZE samples
    //    *audioDacEnReg = 0;
    //}
    //finally, enable output
    //*audioDacEnReg = 1;


    volatile _SPM short *left;
    volatile _SPM short *right;
    volatile _SPM short *mute;

    const unsigned int LEFT_ADDR  = 0;
    const unsigned int RIGHT_ADDR = sizeof(int);
    const unsigned int MUTE_ADDR = 2*sizeof(int);


    left  = (volatile _SPM short *) LEFT_ADDR;
    right = (volatile _SPM short *) RIGHT_ADDR;
    mute = (volatile _SPM short *) MUTE_ADDR;
    
    *mute = 0;

    printf("Hello hello hello!\n");
    for (int i = 0; i < 1*480000; i++){
        getInputBufferSPM(left, right);
        setOutputBufferSPM(left, right);
    }
    //for (int i = 0; i < 1000; i++){printf("Interruption. ");}
    printf("Only left.\n");
    for (int i = 0; i < 1*480000; i++){
        getInputBufferSPM(left, right);
        setOutputBufferSPM(left, mute);
    }
    printf("Only right.\n");
    for (int i = 0; i < 1*480000; i++){
        getInputBufferSPM(left, right);
        setOutputBufferSPM(mute, right);
    }

    printf("Goodbye!\n");

    return 0;

}
