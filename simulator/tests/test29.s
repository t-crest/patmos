#
# Expected Result: r1 = 9 & r2 = 1 & PRR = 00000001
#

                .word   36;
                cmpeq   p3  = r0 , r0;
                mfs     r1  = s0;
                mts     s0  = r0;
		mfs     r2  = s0;
                halt;
		nop;
		nop;
		nop;
