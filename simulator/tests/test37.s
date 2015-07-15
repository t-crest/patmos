#
# Expected Result: r1 = 0x02820181 & r2 = 0x02820181 & r3 = 0x001fffe0 & r4 = 0x001fffd0 & r5 = 0x00000010
#

                .word    92;
                lwm      r1  = [r0 + 1];
		add      r2 = r0, 0x200000;
		mts      s5 = r2;
		mts      s6 = r2;
                sres     8;
                sws      [r0 + 7] = r1;
		sspill   4;
		sfree    4;
		sres     8;
		addi     r2 = r0, 16;
		sspillr  r2;
		mfs      r3 = s5;
		mfs      r4 = s6;
		sub      r5 = r3, r4;
		sfree    8;
		sensr    r2;
		lws      r2 = [r0 + 3];
                sfree    4;
                halt;
		nop;
		nop;
		nop;
