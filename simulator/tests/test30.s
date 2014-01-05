#
# Expected Result: r2 = 1
#

                .word   28;
                lwc     r1 = [r0 + 60];
                addi    r2 = r2, 1;
                halt;
                nop;
                nop;
		nop;
