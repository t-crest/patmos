        .word 120
		add    r1 = r0, 0x1000;
        add    r2 = r0, 0x01234567;
        add    r3 = r0, 0x89abcdef;
        mts    s5 = r1;
        mts    s6 = r1;

        sres   4;
        sws    [0] = r2;
        sws    [1] = r3;
        sspill 2;
		add    r1 = r0, 0x0ff8;
        lwc    r4 = [r1+0];
        lwc    r5 = [r1+1];
        mfs    r6 = s5;
        mfs    r7 = s6;
        
        swc    [r1+0] = r3;
        swc    [r1+1] = r2;
        sens   2;
        lws    r8 = [0];
        lws    r9 = [1];        
        nop;
        nop;
        halt;
        nop;
        nop;
        nop;