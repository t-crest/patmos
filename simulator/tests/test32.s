#
# Expected Result: r2 = 1
#

                .word   68;
                nop;
		addi    r1 = r0, x;
                nop;
                brcfr   r1, r0;
                nop;
                nop;
                halt;
                nop;
		nop;
                nop;
                .word   52;
x:              add     r2 = r0, 1;
                halt;
                nop;
                nop;
		nop;
