#
# Expected Result: r2 = 1
#

                addi    r1 = r0, y;
                bs      x;
                nop     0;
                nop     0;
                .word   48;
x:              nop     0;
                nop     0;
                nop     0;
                bsr     r1;
                nop     0;
                addi    r2 = r0, 7;
                halt;
                nop     0;
                nop     0;
                .word   20;
y:              add     r2 = r0, 1;
                ret;
                nop     0;
                nop     0;
