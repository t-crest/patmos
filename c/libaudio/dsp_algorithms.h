#ifndef _DSP_ALGORITHMS_H_
#define _DSP_ALGORITHMS_H_

int filterIIR_1st(int pnt_i, _SPM short (*x)[2], _SPM short (*y)[2], _SPM int *accum, _SPM short *B, _SPM short *A, int shiftLeft);

int filterIIR_2nd(_SPM int *pnt_i, _SPM short (*x)[2], _SPM short (*y)[2], _SPM int *accum, _SPM short *B, _SPM short *A, _SPM int *shiftLeft);

int storeSinInterpol(int *sinArray, short *fracArray, int SIZE, int OFFSET, int AMP);

int storeSin(int *sinArray, int SIZE, int OFFSET, int AMP);

int filter_coeff_bp_br(int FILT_ORD_1PL, _SPM short *B, _SPM short *A, int Fc, int Fb,_SPM int *shiftLeft, int fixedShift);

int filter_coeff_hp_lp(int FILT_ORD_1PL, _SPM short *B, _SPM short *A, int Fc, float Q, _SPM int *shiftLeft, int fixedShift, int type);

__attribute__((always_inline))
int allpass_comb(int AP_BUF_LEN, _SPM int *pnt, short (*ap_buffer)[AP_BUF_LEN], volatile _SPM short *x, volatile _SPM short *y, _SPM short *g);

__attribute__((always_inline))
int combFilter_1st(int AUDIO_BUF_LEN, _SPM int *pnt, short (*audio_buffer)[AUDIO_BUF_LEN], volatile _SPM short *y, _SPM int *accum, _SPM short *g, _SPM int *del);

__attribute__((always_inline))
int combFilter_2nd(int AUDIO_BUF_LEN, _SPM int *pnt, short (*audio_buffer)[AUDIO_BUF_LEN], volatile _SPM short *y, _SPM int *accum, _SPM short *g, _SPM int *del);

#endif /* _DSP_ALGORITHMS_H_ */
