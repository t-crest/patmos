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

struct Overdrive {
    int   accum[2]; //accummulator accum[2]
};


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


int audio_overdrive(_SPM struct Overdrive *odP, volatile _SPM short *xP, volatile _SPM short *yP) {
    //THRESHOLD IS 1/3 = 0x2AAB
    //input abs:
    unsigned int x_abs[2];

    _Pragma("loopbound min 100 max 100")
    for(int i=0; i<100; i++) {
        _Pragma("loopbound min 2 max 2")
        for(int j=0; j<2; j++) {
            int index = 2 * i + j;
            x_abs[j] = abs(xP[index]);
            if(x_abs[j] > (2 * 0x2AAB)) { // saturation : y = 1
                if (xP[index] > 0) {
                    yP[index] = 0x7FFF;
                }
                else {
                    yP[index] = 0x8000;
                }
            }
            else {
                if(x_abs[j] > 0x2AAB) { // smooth overdrive: y = ( 3 - (2-3*x)^2 ) / 3;
                    odP->accum[j] = (0x17FFF * x_abs[j]) >> 15 ; // result is 1 sign + 17 bits
                    odP->accum[j] = 0xFFFF - odP->accum[j];
                    odP->accum[j] = (odP->accum[j] * odP->accum[j]) >> 15;
                    odP->accum[j] = 0x17FFF - odP->accum[j];
                    odP->accum[j] = (odP->accum[j] * 0x2AAB) >> 15;
                    if(xP[index] > 0) { //positive
                        if(odP->accum[j] > 32767) {
                            yP[index] = 32767;
                        }
                        else {
                            yP[index] = odP->accum[j];
                        }
                    }
                    else { // negative
                        yP[index] = -odP->accum[j];
                    }
                }
                else { // linear zone: y = 2*x
                    yP[index] = xP[index] << 1;
                }
            }
        }
    }

    return 0;
}



const unsigned int OFFS_ADDR = 0xe8000000;

int main() {

    unsigned int LAST_ADDR = OFFS_ADDR; //correct?

    _SPM struct Distortion *distortionP;
    distortionP = (_SPM struct Distortion *) LAST_ADDR;
    alloc_distortion_vars(distortionP);
    LAST_ADDR += sizeof(struct Distortion);

    _SPM struct Overdrive *overdriveP;
    overdriveP = (_SPM struct Overdrive *) LAST_ADDR;
    //nothing to allocate
    LAST_ADDR += sizeof(struct Overdrive);

    volatile _SPM short * xP;
    volatile _SPM short * yP;

    xP = (volatile _SPM short *) LAST_ADDR;
    yP = (volatile _SPM short *) (LAST_ADDR + 2 * 100 * sizeof(short));

    //initialise some data
    for(int i=0; i<100; i++) {
        xP[i*2]   = ONE_16b * (float)(i+1)/(float)100; //from 0 to 1
        yP[i*2+1] = ONE_16b - (ONE_16b * (float)(i+1)/(float)100); //from 1 to 0
    }

    audio_distortion(distortionP, xP, yP);

    audio_overdrive(overdriveP, xP, yP);

    return 0;
}
