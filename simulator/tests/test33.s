#
# Expected Result: r2 = 1
#

                addi    r1 = r0, 16;
                call    5;
                nop     0;
                nop     0;
                .word   12;
                nop     0;
                nop     0;
                addi    r30 = r0, 20;
                callr   r1;
                nop     0;
                addi    r2 = r0, 7;
                halt;
                nop     0;
                nop     0;
                .word   20;
                add     r2 = r0, 1;
                ret     r30, r31;
                nop     0;
                nop     0;
