#ifndef _AUDIOINIT_H_
#define _AUDIOINIT_H_

/*
//how many cores take part in the audio system
const int AUDIO_CORES = 4; //3;
//how many effects are on the system in total
const int FX_AMOUNT = 6;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | S | IN_TYPE | OUT_TYPE //
const int FX_SCHED[FX_AMOUNT][8] = {
    {0, 0,  0, 8, 8, 1, 0, 3},
    {1, 0, 11, 8, 8, 1, 3, 2},
    {2, 3,  0, 8, 8, 1, 2, 3},
    {3, 3,  3, 8, 8, 1, 3, 2},
    {4, 1,  4, 8, 8, 1, 2, 2},
    {5, 0,  0, 8, 8, 1, 2, 1},
};
//amount of NoC channels (NoC or same core!!)
const int CHAN_AMOUNT = 5;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 8, 8, 8, 8, 8, };
// column: FX_ID source   ,   row: CHAN_ID dest
const int SEND_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {1, 0, 0, 0, 0}, // fx 0 sends to chan 0
    {0, 1, 0, 0, 0}, // fx 1 sends to chan 1
    {0, 0, 1, 0, 0}, // fx 2 sends to chan 2
    {0, 0, 0, 1, 0}, // fx 3 sends to chan 3
    {0, 0, 0, 0, 1}, // fx 4 sends to chan 4
    {0, 0, 0, 0, 0},
};
// column: FX_ID dest   ,   row: CHAN_ID source
const int RECV_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0}, // fx 1 receives from chan 0
    {0, 1, 0, 0, 0}, // fx 2 receives from chan 1
    {0, 0, 1, 0, 0}, // fx 3 receives from chan 2
    {0, 0, 0, 1, 0}, // fx 4 receives from chan 3
    {0, 0, 0, 0, 1}, // fx 5 receives from chan 4
};
//latency from input to output in samples (without considering NoC)
const int LATENCY = 4;
*/
/*
//how many cores take part in the audio system
const int AUDIO_CORES = 3;
//how many effects are on the system in total
const int FX_AMOUNT = 5;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
const int FX_SCHED[FX_AMOUNT][10] = {
    {0, 0, 0, 8, 8, 1, 0, 0, -1,  1},
    {1, 0, 1, 8, 8, 8, 0, 1,  0,  0},
    {2, 1, 4, 8, 1, 1, 1, 1,  0,  1},
    {3, 2, 6, 1, 8, 1, 1, 1,  1,  2},
    {4, 0, 0, 8, 8, 1, 1, 0,  2, -1}
};
//amount of NoC channels
const int CHAN_AMOUNT = 3;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 8, 8, 8, };
//latency from input to output in samples (without considering NoC)
const int LATENCY = 4;
*/
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
//how many cores take part in the audio system
const int AUDIO_CORES = 4;
//how many effects are on the system in total
const int FX_AMOUNT = 7;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | S | IN_TYPE | OUT_TYPE //
const int FX_SCHED[FX_AMOUNT][8] = {
    {0, 0, 1,  8, 8, 8, 0, 3},
    {1, 0, 0,  8, 8, 1, 3, 2},
    {2, 1, 0,  8, 1, 1, 2, 2},
    {3, 3, 0,  1, 1, 1, 2, 2},
    {4, 2, 0,  1, 8, 1, 2, 2},
    {5, 0, 0,  8, 8, 1, 2, 3},
    {6, 0, 1,  8, 8, 8, 3, 1},
};
//amount of NoC channels (NoC or same core!!)
const int CHAN_AMOUNT = 6;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 8, 8, 8, 8, 8, 8, };
// column: FX_ID source   ,   row: CHAN_ID dest
const int SEND_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {1, 0, 0, 0, 0, 0}, // fx 0 sends to chan 0
    {0, 1, 0, 0, 0, 0}, // fx 1 sends to chan 1
    {0, 0, 1, 0, 0, 0}, // fx 2 sends to chan 2
    {0, 0, 0, 1, 0, 0}, // fx 3 sends to chan 3
    {0, 0, 0, 0, 1, 0}, // fx 4 sends to chan 4
    {0, 0, 0, 0, 0, 1}, // fx 5 sends to chan 5
    {0, 0, 0, 0, 0, 0},
};
// column: FX_ID dest   ,   row: CHAN_ID source
const int RECV_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {0, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 0}, // fx 1 receives from chan 0
    {0, 1, 0, 0, 0, 0}, // fx 2 receives from chan 1
    {0, 0, 1, 0, 0, 0}, // fx 3 receives from chan 2
    {0, 0, 0, 1, 0, 0}, // fx 4 receives from chan 3
    {0, 0, 0, 0, 1, 0}, // fx 5 receives from chan 4
    {0, 0, 0, 0, 0, 1}, // fx 6 receives from chan 5
};
//latency from input to output in samples (without considering NoC)
const int LATENCY = 4;
*/
/*
//how many cores take part in the audio system
const int AUDIO_CORES = 3;
//how many effects are on the system in total
const int FX_AMOUNT = 4;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | S | IN_TYPE | OUT_TYPE //
const int FX_SCHED[FX_AMOUNT][8] = {
    {0, 0, 0,  8, 8, 1, 0, 2},
    {1, 1, 2,  8, 8, 1, 2, 2},
    {2, 2, 6,  8, 8, 1, 2, 2},
    {3, 0, 0,  8, 8, 1, 2, 1},
};
//amount of NoC channels (NoC or same core!!)
const int CHAN_AMOUNT = 4;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 8, 8, 8, 8, };
// column: FX_ID source   ,   row: CHAN_ID dest
const int SEND_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {1, 1, 0, 0}, // fx 0 sends to chan 0 and 1
    {0, 0, 1, 0}, // fx 1 sends to chan 2
    {0, 0, 0, 1}, // fx 2 sends to chan 3
    {0, 0, 0, 0},
};
// column: FX_ID dest   ,   row: CHAN_ID source
const int RECV_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {0, 0, 0, 0},
    {1, 0, 0, 0}, // fx 1 receives from chan 0
    {0, 1, 0, 0}, // fx 2 receives from chan 1
    {0, 0, 1, 1}, // fx 3 receives from chan 2 and 3
};
//latency from input to output in samples (without considering NoC)
const int LATENCY = 3;
*/

//total amount of modes
const int MODES = 3;
//how many cores take part in the audio system
const int AUDIO_CORES = 4;
//how many effects are on each mode
const int FX_AMOUNT[MODES] = {6, 2, 6, };
//maximum amount of effects per core
const int MAX_FX_PER_CORE[AUDIO_CORES] = {3, 1, 1, 2, };
//maximum amount of FX
const int MAX_FX = 6;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | S | IN_TYPE | OUT_TYPE //
const int FX_SCHED_0[6][8] = {
    {0, 0,  0,  32, 32, 1, 0, 2},
    {1, 1,  1,  32, 32, 8, 2, 2},
    {2, 3,  2,  32, 32, 1, 2, 3},
    {3, 3,  7,  32, 32, 1, 3, 2},
    {4, 2, 11,  32, 32, 1, 2, 2},
    {5, 0,  0,  32, 32, 1, 2, 1},
};
const int FX_SCHED_1[2][8] = {
    {0, 0,  0, 16, 16, 1, 0, 3},
    {1, 0,  6, 16, 16, 1, 3, 1},
};
const int FX_SCHED_2[6][8] = {
    {0, 0,  0, 64, 64, 1, 0, 3},
    {1, 0, 11, 64, 64, 1, 3, 2},
    {2, 3,  0, 64, 64, 1, 2, 3},
    {3, 3,  3, 64, 64, 1, 3, 2},
    {4, 1,  4, 64, 64, 1, 2, 2},
    {5, 0,  0, 64, 64, 1, 2, 1},
};
//pointer to schedules
const int *FX_SCHED_P[MODES] = {
    (const int *)FX_SCHED_0,
    (const int *)FX_SCHED_1,
    (const int *)FX_SCHED_2,
};
//amount of NoC channels (NoC or same core!!) ON ALL MODES
const int CHAN_AMOUNT = 12;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, };
// column: FX_ID source   ,   row: CHAN_ID dest
const int SEND_ARRAY_0[6][CHAN_AMOUNT] = {
    {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, },
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, },
    {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
};
const int SEND_ARRAY_1[2][CHAN_AMOUNT] = {
    {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
};
const int SEND_ARRAY_2[6][CHAN_AMOUNT] = {
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, },
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
};
//pointer to send arrays
const int *SEND_ARRAY_P[MODES] = {
    (const int *)SEND_ARRAY_0,
    (const int *)SEND_ARRAY_1,
    (const int *)SEND_ARRAY_2,
};
// column: FX_ID dest   ,   row: CHAN_ID source
const int RECV_ARRAY_0[6][CHAN_AMOUNT] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, },
    {0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, },
    {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, },
};
const int RECV_ARRAY_1[2][CHAN_AMOUNT] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, },
};
const int RECV_ARRAY_2[6][CHAN_AMOUNT] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, },
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
};
//pointer to receive arrays
const int *RECV_ARRAY_P[MODES] = {
    (const int *)RECV_ARRAY_0,
    (const int *)RECV_ARRAY_1,
    (const int *)RECV_ARRAY_2,
};
//latency from input to output in samples (without considering NoC)
const unsigned int LATENCY[MODES] = {4, 0, 4, };

/*
//how many cores take part in the audio system
const int AUDIO_CORES = 4;
//how many effects are on the system in total
const int FX_AMOUNT = 5;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | S | IN_TYPE | OUT_TYPE //
const int FX_SCHED[FX_AMOUNT][8] = {
    {0, 0, 1,  8, 8, 8, 0, 2},
    {1, 1, 0,  8, 1, 1, 2, 2},
    {2, 2, 0,  8, 1, 1, 2, 2},
    {3, 3, 0,  1, 8, 1, 2, 2},
    {4, 0, 1,  8, 8, 8, 2, 1},
};
//amount of NoC channels (NoC or same core!!)
const int CHAN_AMOUNT = 5;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 8, 8, 8, 8, 8, };
// column: FX_ID source   ,   row: CHAN_ID dest
const int SEND_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {1, 1, 0, 0, 0, },
    {0, 0, 1, 0, 0, },
    {0, 0, 0, 1, 0, },
    {0, 0, 0, 0, 1, },
    {0, 0, 0, 0, 0, },
};
// column: FX_ID dest   ,   row: CHAN_ID source
const int RECV_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {0, 0, 0, 0, 0, },
    {1, 0, 0, 0, 0, },
    {0, 1, 0, 0, 0, },
    {0, 0, 1, 1, 0, },
    {0, 0, 0, 0, 1, },
};
//latency from input to output in samples (without considering NoC)
const int LATENCY = 4;
*/
/*
//how many cores take part in the audio system
const int AUDIO_CORES = 4;
//how many effects are on the system in total
const int FX_AMOUNT = 5;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | S | IN_TYPE | OUT_TYPE //
const int FX_SCHED[FX_AMOUNT][8] = {
    {0, 0, 0,  1, 1, 1, 0, 2},
    {1, 1, 0,  1, 8, 1, 2, 2},
    {2, 2, 0,  1, 8, 1, 2, 2},
    {3, 3, 0,  8, 1, 1, 2, 2},
    {4, 0, 0,  1, 1, 1, 2, 1},
};
//amount of NoC channels (NoC or same core!!)
const int CHAN_AMOUNT = 5;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 8, 8, 8, 8, 8, };
// column: FX_ID source   ,   row: CHAN_ID dest
const int SEND_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {1, 1, 0, 0, 0, },
    {0, 0, 1, 0, 0, },
    {0, 0, 0, 1, 0, },
    {0, 0, 0, 0, 1, },
    {0, 0, 0, 0, 0, },
};
// column: FX_ID dest   ,   row: CHAN_ID source
const int RECV_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {0, 0, 0, 0, 0, },
    {1, 0, 0, 0, 0, },
    {0, 1, 0, 0, 0, },
    {0, 0, 1, 1, 0, },
    {0, 0, 0, 0, 1, },
};
//latency from input to output in samples (without considering NoC)
const int LATENCY = 11;
*/
/*
//how many cores take part in the audio system
const int AUDIO_CORES = 4;
//how many effects are on the system in total
const int FX_AMOUNT = 5;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | S | IN_TYPE | OUT_TYPE //
const int FX_SCHED[FX_AMOUNT][8] = {
    {0, 0, 0,  1, 1, 1, 0, 2},
    {1, 1, 0,  1, 8, 1, 2, 2},
    {2, 2, 0,  8, 1, 1, 2, 2},
    {3, 3, 0,  8, 1, 1, 2, 2},
    {4, 0, 0,  1, 1, 1, 2, 1},
};
//amount of NoC channels (NoC or same core!!)
const int CHAN_AMOUNT = 5;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 8, 8, 8, 8, 8, };
// column: FX_ID source   ,   row: CHAN_ID dest
const int SEND_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {1, 0, 0, 0, 0, },
    {0, 1, 1, 0, 0, },
    {0, 0, 0, 1, 0, },
    {0, 0, 0, 0, 1, },
    {0, 0, 0, 0, 0, },
};
// column: FX_ID dest   ,   row: CHAN_ID source
const int RECV_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {0, 0, 0, 0, 0, },
    {1, 0, 0, 0, 0, },
    {0, 1, 0, 0, 0, },
    {0, 0, 1, 0, 0, },
    {0, 0, 0, 1, 1, },
};
//latency from input to output in samples (without considering NoC)
const int LATENCY = 11;
*/
/*
//how many cores take part in the audio system
const int AUDIO_CORES = 4;
//how many effects are on the system in total
const int FX_AMOUNT = 5;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | S | IN_TYPE | OUT_TYPE //
const int FX_SCHED[FX_AMOUNT][8] = {
    {0, 0, 1,  8, 8, 8, 0, 2},
    {1, 1, 0,  8, 1, 1, 2, 2},
    {2, 2, 0,  1, 8, 1, 2, 2},
    {3, 3, 0,  1, 8, 1, 2, 2},
    {4, 0, 1,  8, 8, 8, 2, 1},
};
//amount of NoC channels (NoC or same core!!)
const int CHAN_AMOUNT = 5;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 8, 8, 8, 8, 8, };
// column: FX_ID source   ,   row: CHAN_ID dest
const int SEND_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {1, 0, 0, 0, 0, },
    {0, 1, 1, 0, 0, },
    {0, 0, 0, 1, 0, },
    {0, 0, 0, 0, 1, },
    {0, 0, 0, 0, 0, },
};
// column: FX_ID dest   ,   row: CHAN_ID source
const int RECV_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {0, 0, 0, 0, 0, },
    {1, 0, 0, 0, 0, },
    {0, 1, 0, 0, 0, },
    {0, 0, 1, 0, 0, },
    {0, 0, 0, 1, 1, },
};
//latency from input to output in samples (without considering NoC)
const int LATENCY = 4;
*/
/*
//how many cores take part in the audio system
const int AUDIO_CORES = 4;
//how many effects are on the system in total
const int FX_AMOUNT = 5;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | S | IN_TYPE | OUT_TYPE //
const int FX_SCHED[FX_AMOUNT][8] = {
    {0, 0,  1,  16, 16, 8, 0, 2},
    {1, 1,  7,  16, 32, 1, 2, 2},
    {2, 2,  2,  32, 16, 1, 2, 2},
    {3, 3,  6,  32, 16, 1, 2, 2},
    {4, 0,  1,  16, 16, 8, 2, 1},
};
//amount of NoC channels (NoC or same core!!)
const int CHAN_AMOUNT = 5;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 4, 4, 4, 4, 4, };
// column: FX_ID source   ,   row: CHAN_ID dest
const int SEND_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {1, 0, 0, 0, 0, },
    {0, 1, 1, 0, 0, },
    {0, 0, 0, 1, 0, },
    {0, 0, 0, 0, 1, },
    {0, 0, 0, 0, 0, },
};
// column: FX_ID dest   ,   row: CHAN_ID source
const int RECV_ARRAY[FX_AMOUNT][CHAN_AMOUNT] = {
    {0, 0, 0, 0, 0, },
    {1, 0, 0, 0, 0, },
    {0, 1, 0, 0, 0, },
    {0, 0, 1, 0, 0, },
    {0, 0, 0, 1, 1, },
};
//latency from input to output in samples (without considering NoC)
const int LATENCY = 11;
*/


/*
//how many cores take part in the audio system
const int AUDIO_CORES = 1;
//how many effects are on the system in total
const int FX_AMOUNT = 1;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
const int FX_SCHED[FX_AMOUNT][10] = {
    {0, 0, 2, 1, 1, 1, 0, 0, -1,  -1}
};
//amount of NoC channels
const int CHAN_AMOUNT = 0;
//amount of buffers on each NoC channel ID
const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = {};
//latency from input to output in samples (without considering NoC)
const int LATENCY = 0;
*/


#endif /* _AUDIOINIT_H_ */
