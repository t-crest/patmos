#
# Expected Result: r1 = 0xfb
#
                add     r1  = r0, 0xF0000000;
                addi    r2  = r0, 2;
                lwl     r3  = [r1 + 0];
                andi    r3  = r3, 2;
                bne     r3 != r2, -4;
                nop     0;
                nop     0;
                lwl     r1  = [r1 + 1];
                halt;
