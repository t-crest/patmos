#
# Expected Result: r2 = 1
#

                .word   68;
                lwc     r1 = [r0 + 12];
                nop;
                nop;
                brcfr   r1;
                nop;
                nop;
                halt;
                nop;
                nop;
		nop;
                .word   52;
                add     r2 = r0, 1;
                halt;
                nop;
                nop;
		nop;
