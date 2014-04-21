#
# Expected Result: r0 = 0, r1 = 1, p0 = 1, r2 = 5
#

                .word   40;
		addi    r0  = r0,  1;
		addi    r0  = r0,  1;
		addi    r1  = r0,  1;
                cmpineq p0  = r0,  0;
        (p0)    addi    r2  = r0 , 5;
                halt;
		nop;
		nop;
		nop;
