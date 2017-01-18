
        #ifndef _AUDIOINIT_H_
        #define _AUDIOINIT_H_

        
        //input/output buffer sizes
        const unsigned int BUFFER_SIZE = 128;
        //amount of configuration modes
        const int MODES = 3;
        //how many cores take part in the audio system (from all modes)
        const int AUDIO_CORES = 4;
        //how many effects are on each mode in total
        const int FX_AMOUNT[MODES] = {6, 5, 4, };
        //maximum amount of effects per core
        const int MAX_FX_PER_CORE[AUDIO_CORES] = {2, 1, 2, 1, };
        //maximum FX_AMOUNT
        const int MAX_FX = 6;
        // FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | S | IN_TYPE | OUT_TYPE //
        const int FX_SCHED_0[6][8] = {
            { 0, 0, 0, 16, 16, 1, 0, 2 },
            { 1, 1, 2, 16, 4, 1, 2, 2 },
            { 2, 2, 0, 4, 4, 1, 2, 3 },
            { 3, 2, 7, 4, 4, 1, 3, 2 },
            { 4, 3, 5, 4, 16, 1, 2, 2 },
            { 5, 0, 0, 16, 16, 1, 2, 1 },
        };
        const int FX_SCHED_1[5][8] = {
            { 0, 0, 0, 16, 16, 1, 0, 2 },
            { 1, 1, 2, 16, 4, 1, 2, 2 },
            { 2, 2, 6, 16, 4, 1, 2, 2 },
            { 3, 3, 12, 4, 16, 1, 2, 2 },
            { 4, 0, 0, 16, 16, 1, 2, 1 },
        };
        const int FX_SCHED_2[4][8] = {
            { 0, 0, 0, 16, 16, 1, 0, 2 },
            { 1, 1, 3, 16, 4, 1, 2, 2 },
            { 2, 2, 11, 4, 16, 1, 2, 2 },
            { 3, 0, 0, 16, 16, 1, 2, 1 },
        };
        //pointer to schedules
        const int *FX_SCHED_P[MODES] = {
            (const int *)FX_SCHED_0,
            (const int *)FX_SCHED_1,
            (const int *)FX_SCHED_2,
        };
        //amount of NoC channels (NoC or same core) on all modes
        const int CHAN_AMOUNT = 13;
        //amount of buffers on each NoC channel ID
        const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 3, 3, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, };
        // column: FX_ID source   ,   row: CHAN_ID dest
        const int SEND_ARRAY_0[6][CHAN_AMOUNT] = {
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
            {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
            {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
            {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
            {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, },
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
        };
        const int SEND_ARRAY_1[5][CHAN_AMOUNT] = {
            {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, },
            {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, },
            {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, },
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, },
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
        };
        const int SEND_ARRAY_2[4][CHAN_AMOUNT] = {
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, },
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, },
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
        };
        //pointer to send arrays
        const int *SEND_ARRAY_P[MODES] = {
            (const int *)SEND_ARRAY_0,
            (const int *)SEND_ARRAY_1,
            (const int *)SEND_ARRAY_2,
        };
        // column: FX_ID dest   ,   row: CHAN_ID source
        const int RECV_ARRAY_0[6][CHAN_AMOUNT] = {
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
            {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
            {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
            {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
            {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, },
        };
        const int RECV_ARRAY_1[5][CHAN_AMOUNT] = {
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
            {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, },
            {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, },
            {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, },
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, },
        };
        const int RECV_ARRAY_2[4][CHAN_AMOUNT] = {
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, },
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, },
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        };
        //pointer to receive arrays
        const int *RECV_ARRAY_P[MODES] = {
            (const int *)RECV_ARRAY_0,
            (const int *)RECV_ARRAY_1,
            (const int *)RECV_ARRAY_2,
        };
        //latency from input to output in samples (without considering NoC)
        const unsigned int LATENCY[MODES] = {4, 4, 4, };

        #endif /* _AUDIOINIT_H_ */