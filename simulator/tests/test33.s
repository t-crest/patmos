#
# Expected Result: r2 = 1
#

                addi    r1 = r0, y;
                call    x;
                nop;
                nop;
                .word   48;
x:              nop;
                nop;
                addi    r30 = r0, y;
                callr   r1;
                nop;
                addi    r2 = r0, 7;
                halt;
                nop;
                nop;
                .word   20;
y:              add     r2 = r0, 1;
                ret     r30, r31;
                nop;
                nop;
