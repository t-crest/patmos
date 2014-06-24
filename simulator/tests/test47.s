#
# Expected Result: only 10 blocks spilled instead of 20
#

                .word   200;
# initialize the stack cache registers: stack top and mem top
                addi    r1  = r0, 0x900;
                mts     s5 = r1;
                mts     s6 = r1;
# reserve some space on the stack cache
                sres    10;
                nop;
                nop;
                nop;
# see if lazy pointer moves as it should
                sws     [r0 + 0] = r0;
                sws     [r0 + 1] = r0;
		sws     [r0 + 2] = r0;
		sws     [r0 + 3] = r0;
		sws     [r0 + 4] = r0;
		sws     [r0 + 5] = r0;
		sws     [r0 + 6] = r0;
                sws     [r0 + 7] = r0;
		sws     [r0 + 8] = r0;
		sws     [r0 + 9] = r0;
                nop;
                nop;
                nop;
# reserve and store on previous blocks
                sres   512;
                nop;
                nop;
                nop;
                sws     [r0 + 4] = r0;
                nop;
                nop;
                nop;
		sfree   512;
		nop;
                nop;
                nop;
		sens    10;
		nop;
                nop;
                nop;
		sres    512;
                nop;
                nop;
                nop;
		sws     [r0 + 4] = r0;
		nop;
                nop;
                nop;
                halt;
                nop;
                nop;
                nop;
