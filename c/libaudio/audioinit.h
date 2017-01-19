
        #ifndef _AUDIOINIT_H_
        #define _AUDIOINIT_H_

        
        //input/output buffer sizes
        const unsigned int BUFFER_SIZE = 128;
        //amount of configuration modes
        const int MODES = 2;
        //how many cores take part in the audio system (from all modes)
        const int AUDIO_CORES = 2;
        //how many effects are on each mode in total
        const int FX_AMOUNT[MODES] = {4, 4, };
        //maximum amount of effects per core
        const int MAX_FX_PER_CORE[AUDIO_CORES] = {2, 2, };
        //maximum FX_AMOUNT
        const int MAX_FX = 4;
        // FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | S | IN_TYPE | OUT_TYPE //
        const int FX_SCHED_0[4][8] = {
            { 0, 0, 0, 32, 32, 1, 0, 2 },
            { 1, 1, 0, 32, 32, 1, 2, 3 },
            { 2, 1, 1, 32, 32, 8, 3, 2 },
            { 3, 0, 0, 32, 32, 1, 2, 1 },
        };
        const int FX_SCHED_1[4][8] = {
            { 0, 0, 0, 32, 32, 1, 0, 2 },
            { 1, 1, 7, 32, 32, 1, 2, 3 },
            { 2, 1, 12, 32, 32, 1, 3, 2 },
            { 3, 0, 0, 32, 32, 1, 2, 1 },
        };
        //pointer to schedules
        const int *FX_SCHED_P[MODES] = {
            (const int *)FX_SCHED_0,
            (const int *)FX_SCHED_1,
        };
        //amount of NoC channels (NoC or same core) on all modes
        const int CHAN_AMOUNT = 6;
        //amount of buffers on each NoC channel ID
        const int CHAN_BUF_AMOUNT[CHAN_AMOUNT] = { 3, 1, 3, 3, 1, 3, };
        // column: FX_ID source   ,   row: CHAN_ID dest
        const int SEND_ARRAY_0[4][CHAN_AMOUNT] = {
            {1, 0, 0, 0, 0, 0, },
            {0, 1, 0, 0, 0, 0, },
            {0, 0, 1, 0, 0, 0, },
            {0, 0, 0, 0, 0, 0, },
        };
        const int SEND_ARRAY_1[4][CHAN_AMOUNT] = {
            {0, 0, 0, 1, 0, 0, },
            {0, 0, 0, 0, 1, 0, },
            {0, 0, 0, 0, 0, 1, },
            {0, 0, 0, 0, 0, 0, },
        };
        //pointer to send arrays
        const int *SEND_ARRAY_P[MODES] = {
            (const int *)SEND_ARRAY_0,
            (const int *)SEND_ARRAY_1,
        };
        // column: FX_ID dest   ,   row: CHAN_ID source
        const int RECV_ARRAY_0[4][CHAN_AMOUNT] = {
            {0, 0, 0, 0, 0, 0, },
            {1, 0, 0, 0, 0, 0, },
            {0, 1, 0, 0, 0, 0, },
            {0, 0, 1, 0, 0, 0, },
        };
        const int RECV_ARRAY_1[4][CHAN_AMOUNT] = {
            {0, 0, 0, 0, 0, 0, },
            {0, 0, 0, 1, 0, 0, },
            {0, 0, 0, 0, 1, 0, },
            {0, 0, 0, 0, 0, 1, },
        };
        //pointer to receive arrays
        const int *RECV_ARRAY_P[MODES] = {
            (const int *)RECV_ARRAY_0,
            (const int *)RECV_ARRAY_1,
        };

        #endif /* _AUDIOINIT_H_ */