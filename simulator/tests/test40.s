#
# Expected Result: r8 = 0x210 & r9 = 0x210 & r10 = 0x10 & r11 = 0xe0 & r12 = 0xe0 & r13 = 0x210 & r14 = 0x100
#

                .word   136;
		addi    r1  = r0, 0x100;
		addi    r2  = r0, 0x210;
		add     r3  = r0, 0x12345678;
		add     r4  = r0, 0x3A4B5C6D;
		add     r5  = r0, 0xABCDEF01;
		mts	s5  = r1;
		mts     s6  = r1;
		sres    8;
		sws     [r0 + 0] = r2;
		sws     [r0 + 1] = r3;
		sws     [r0 + 6] = r5;
		sws     [r0 + 7] = r1;
		lws     r8  = [r0 + 0];
		lhs     r9  = [r0 + 1];
		lbs     r10 = [r0 + 3];
		sspill  8;
		mfs     r11 = s5;
		mfs     r12 = s6;
		sres    8;
		sws     [r0 + 0] = r3;
		sws     [r0 + 7] = r4;
		mts     s5  = r11;
		mts     s6  = r12;
		sens    8;
		lws     r13 = [r0 + 0];
		lws     r14 = [r0 + 7];
		halt;
		nop;
		nop;
		nop;
