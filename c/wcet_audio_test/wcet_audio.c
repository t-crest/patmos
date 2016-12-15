#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <machine/spm.h>
#include <machine/rtc.h>

#define ONE_16b 0x7FFF

struct Distortion {
    int   accum[2]; //accummulator accum[2]
    int   k; //for distortion
    int   kOnePlus; //for distortion
    int   sftLft; //shift left amount
};


int audio_distortion(_SPM struct Distortion *distP, volatile _SPM short *xP, volatile _SPM short *yP) __attribute__((noinline,used));
int audio_distortion(_SPM struct Distortion *distP, volatile _SPM short *xP, volatile _SPM short *yP) {

    _Pragma("loopbound min 100 max 100")
    for(int i=0; i<100; i++) {

        _Pragma("loopbound min 2 max 2")
        for(int j=0; j<2; j++) {
            int index = 2 * i + j;
            distP->accum[0] = (distP->kOnePlus * xP[index]);// >> 15;
            distP->accum[1] = (distP->k * abs(xP[index])) >> 15;
            distP->accum[1] = distP->accum[1] + ((ONE_16b+distP->sftLft) >> distP->sftLft);
            distP->accum[0] = distP->accum[0] / distP->accum[1];
            //reduce if it is poisitive only
            if (xP[index] > 0) {
                yP[index] = distP->accum[0] - 1;
            }
            else {
                yP[index] = distP->accum[0];
            }
        }
    }

    return 0;
}

unsigned int alloc_distortion_vars(_SPM struct Distortion *distP) {

    //initialise k, kOnePlus, shiftLeft:
    float amount = 0.9;
    distP->k = 0x90000;
    distP->sftLft = 0;
    while(distP->k > ONE_16b) {
        distP->sftLft = distP->sftLft + 1;
        distP->k = distP->k >> 1;
    }
    distP->kOnePlus = 0x98000 >> distP->sftLft;

    return 0;
}


const int OFFS_ADDR = 0xe8000000;

int main() {

    _SPM struct Distortion *distortionP;
    distortionP = (_SPM struct Distortion *) OFFS_ADDR; //correct?
    alloc_distortion_vars(distortionP);

    volatile _SPM short * xP;
    volatile _SPM short * yP;

    xP = (volatile _SPM short *) (OFFS_ADDR + sizeof(struct Distortion));
    yP = (volatile _SPM short *) (OFFS_ADDR + sizeof(struct Distortion) + 2 * 100 * sizeof(short));

    audio_distortion(distortionP, xP, yP);

    return 0;
}
