
        #ifndef _AUDIOINIT_H_
        #define _AUDIOINIT_H_


        //max amount of cores (from all modes)
        const int ALL_CORES = 4;
        //configuration modes
        const int MODES = 2;
        //how many cores take part in each mode
        const int AUDIO_CORES[MODES] = {4, 3, };
        //how many effects are on each mode in total
        const int FX_AMOUNT[MODES] = {5, 4, };
        //maximum FX_AMOUNT
        const int MAX_FX = 5;
        // FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
        const int FX_SCHED_0[5][10] = {
            { 0, 0, 2, 1, 1, 1, 0, 1, -1, 0 },
            { 1, 1, 7, 1, 8, 1, 1, 1, 0, 1 },
            { 2, 3, 1, 8, 8, 8, 1, 1, 1, 2 },
            { 3, 2, 11, 8, 1, 1, 1, 1, 2, 3 },
            { 4, 0, 0, 1, 1, 1, 1, 0, 3, -1 },
        };
        const int FX_SCHED_1[4][10] = {
            { 0, 0, 8, 8, 8, 1, 0, 1, -1, 4 },
            { 1, 1, 1, 8, 8, 8, 1, 1, 4, 5 },
            { 2, 3, 3, 8, 8, 1, 1, 1, 5, 6 },
            { 3, 0, 0, 8, 8, 1, 1, 0, 6, -1 },
        };
        const int *FX_SCHED_PNT[MODES] = {
            (const int *)FX_SCHED_0,
            (const int *)FX_SCHED_1,
        };
        //amount of NoC channels
        const int CHAN_AMOUNT = 7;
        //amount of buffers on each NoC channel ID
        const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 8, 8, 8, 8, 8, 8, 8, };
        //latency from input to output in samples (without considering NoC)
        //for each mode:
        const int LATENCY[MODES] = {19, 4, };

        #endif /* _AUDIOINIT_H_ */