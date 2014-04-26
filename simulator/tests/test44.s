#
# Expected Result: r1 = 8, r2 = 12, p1 = 1
#

                .word   32;
		addi    r2  = r0, 12;
                bcopy   r1  = r2,  2, !p0;
                btesti  p1  = r1,  3;
                halt;
		nop;
		nop;
		nop;
