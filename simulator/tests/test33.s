#
# Expected Result: r2 = 1
#

                .word   20;
                addi    r1 = r0, y;
                call    x;
                nop;
                nop;
		nop;
                .word   52;
x:              nop;
                nop;
                addi    r30 = r0, x;
                callr   r1;
                nop;
		nop;
                addi    r2 = r0, 7;
                halt;
                nop;
                nop;
		nop;
                .word   20;
y:              addi    r2 = r0, 1;
                ret;
                nop;
                nop;
		nop;
