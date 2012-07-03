#
# Expected Result: r1 = 0xfb
#
                add     r1  = r0, 0x80000000;
                addi    r2  = r0, 2;
                lbl     r3  = [r1 + 0];
                andi    r3  = r3, 2;
                bne     r3 != r2, -4;
                nop     0;
                nop     0;
                lbul    r1  = [r1 + 1];
                halt;
