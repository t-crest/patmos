#
# Expected Result: r2 = 1
#

                .word   5;
                addi    r1 = r0, y;
                call    x;
                nop;
                nop;
		nop;
                .word   52;
x:              nop;
                nop;
                addi    r30 = r0, y;
                callr   r1;
                nop;
		nop;
                addi    r2 = r0, 7;
                halt;
                nop;
                nop;
                .word   24;
y:              add     r2 = r0, 1;
                ret     r30, r31;
                nop;
                nop;
		nop;
