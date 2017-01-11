#ifndef _AUDIOINIT_H_
#define _AUDIOINIT_H_

/*
//how many cores take part in the audio system
const int AUDIO_CORES = 4; //3;
//how many effects are on the system in total
const int FX_AMOUNT = 6;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
const int FX_SCHED[FX_AMOUNT][10] = {
    {0, 0,  0, 8, 8, 1, 0, 0, -1,  1},
    {1, 0, 11, 8, 8, 1, 0, 1,  0,  0},
    {2, 3,  0, 8, 8, 1, 1, 0,  0,  3},
    {3, 3,  3, 8, 8, 1, 0, 1,  2,  1},
    {4, 1,  4, 8, 8, 1, 1, 1,  1,  2},
    {5, 0,  0, 8, 8, 1, 1, 0,  2, -1}
};
//amount of NoC channels
const int CHAN_AMOUNT = 3;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 8, 8, 8, };
//latency from input to output in samples (without considering NoC)
const int LATENCY = 4;
*/
//max amount of cores
const int ALL_CORES = 3;
//configuration modes
const int MODES = 2;
//how many cores take part in the audio system
const int AUDIO_CORES[MODES] = {1, 3, };
//how many effects are on the system in total
const int FX_AMOUNT[MODES] = {1, 5, };
//maximum FX_AMOUNT
const int MAX_FX = 5;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
const int FX_SCHED_0[1][10] = {
    {0, 0, 2, 1, 1, 1, 0, 0, -1, -1}
};
const int FX_SCHED_1[5][10] = {
    {0, 0, 0, 8, 8, 1, 0, 0, -1,  1},
    {1, 0, 1, 8, 8, 8, 0, 1,  0,  0},
    {2, 1, 4, 8, 1, 1, 1, 1,  0,  1},
    {3, 2, 7, 1, 8, 1, 1, 1,  1,  2},
    {4, 0, 0, 8, 8, 1, 1, 0,  2, -1}
};
const int *FX_SCHED_PNT[MODES] = {
    (const int *)FX_SCHED_0,
    (const int *)FX_SCHED_1,
};
//amount of NoC channels
const int CHAN_AMOUNT = 3;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = {8, 8, 8, };
//latency from input to output in samples (without considering NoC)
const int LATENCY[MODES] = {0, 4, };

/*
//how many cores take part in the audio system
const int AUDIO_CORES = 4;
//how many effects are on the system in total
const int FX_AMOUNT = 5;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
const int FX_SCHED[FX_AMOUNT][10] = {
    {0, 0,  2, 1, 1, 1, 0, 1, -1,  0},
    {1, 1,  4, 1, 8, 1, 1, 1,  0,  1},
    {2, 3,  1, 8, 8, 8, 1, 1,  1,  2},
    {3, 2,  5, 8, 1, 1, 1, 1,  2,  3},
    {4, 0,  0, 1, 1, 1, 1, 0,  3, -1}
};
//amount of NoC channels
const int CHAN_AMOUNT = 4;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 8, 8, 8, 8, };
//latency from input to output in samples (without considering NoC)
const int LATENCY = 19;
*/
/*
//how many cores take part in the audio system
const int AUDIO_CORES = 4;
//how many effects are on the system in total
const int FX_AMOUNT = 5;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
const int FX_SCHED[FX_AMOUNT][10] = {
    {0, 0, 11,  32,  32, 1, 0, 1, -1,  0},
    {1, 1,  1,  32, 128, 8, 1, 1,  0,  1},
    {2, 3,  9, 128, 128, 1, 1, 1,  1,  2},
    {3, 2,  5, 128,  32, 1, 1, 1,  2,  3},
    {4, 0,  0,  32,  32, 1, 1, 0,  3, -1}
};
//amount of NoC channels
const int CHAN_AMOUNT = 4;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 4, 4, 4, 4, };
//latency from input to output in samples (without considering NoC)
const int LATENCY = 11;
*/

/*
//max amount of cores
const int ALL_CORES = 1;
//configuration modes
const int MODES = 2;
//how many cores take part in the audio system
const int AUDIO_CORES[MODES] = {1, 1, };
//how many effects are on the system in total
const int FX_AMOUNT[MODES] = {1, 2, };
//maximum FX_AMOUNT
const int MAX_FX = 2;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
const int FX_SCHED_0[1][10] = {
    {0, 0, 2, 1, 1, 1, 0, 0, -1, -1}
};
const int FX_SCHED_1[2][10] = {
    {0, 0, 7, 1, 1, 1, 0, 0, -1,  1},
    {1, 0, 2, 1, 1, 1, 0, 0,  0, -1}
};
const int *FX_SCHED_PNT[MODES] = {
    (const int *)FX_SCHED_0,
    (const int *)FX_SCHED_1,
};
//amount of NoC channels
const int CHAN_AMOUNT = 0;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = {};
//latency from input to output in samples (without considering NoC)
const int LATENCY[MODES] = {0, 0, };
*/


#endif /* _AUDIOINIT_H_ */
