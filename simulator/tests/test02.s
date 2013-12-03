#
# Expected Result: r1 = 5 & r2 = 5
#

                .word   28;
        (p0)    addi    r1  = r0 , 5    ||        (p0)    addi    r2  = r1 , 5;
                halt;
		nop;
		nop;
		nop;
