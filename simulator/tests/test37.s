#
# Expected Result: r1 = 0x0283f181 & r2 = 0x0283f181 & r3 = 0x003ffff0 & r4 = 0x003fffe8 & r5 = 0x00000008
#

                .word    25;
                lwm      r1  = [r31 + 1];
		add      r2 = r0, 0x400000;
		mts      s5 = r2;
		mts      s6 = r2;
                sres     4;
                sws     [r0 + 3] = r1;
		sspill   2;
		nop;
		sfree    2;
		nop;
		sres     4;
		addi     r2 = r0, 8;
		sspillr  r2;
		nop;
		mfs      r3 = s5;
		mfs      r4 = s6;
		sub      r5 = r3, r4;
		sfree    4;
		nop;
		sensr    r2;
		lws      r2 = [r0 + 1];
                sfree    2;
                halt;
