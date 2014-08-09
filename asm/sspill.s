        .word 184
        add    r1 = r0, 0x1000;
        mts    s5 = r1;
        mts    s6 = r1;
        add    r2 = r0, 0x01234567;
        add    r3 = r0, 0x89abcdef;
        add    r4 = r0, 0x77553311;
        add    r5 = r0, 0x22446688;

        add    r1 = r0, 0x0ff0;
        swc    [r1+0] = r4;
        swc    [r1+1] = r5;
        swc    [r1+2] = r2;
        swc    [r1+3] = r3;
        
        sres   4;
        sws    [0] = r2;
        sws    [1] = r3;
        sws    [2] = r4;
        sws    [3] = r5;
        sspill 2;

        mfs    r6 = s5;
        mfs    r7 = s6;
        lwc    r8 = [r1+0];
        lwc    r9 = [r1+1];
        lwc    r10 = [r1+2];
        lwc    r11 = [r1+3];
        
        swc    [r1+2] = r4;
        swc    [r1+3] = r5;
        sens   4;
        
        mfs    r12 = s5;
        mfs    r13 = s6;
        lws    r14 = [0];
        lws    r15 = [1];
        lws    r16 = [2];
        lws    r17 = [3];

        nop;
        nop;
        halt;
        nop;
        nop;
        nop;
