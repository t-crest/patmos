#
# Expected Result: r2 = 0x21
#
                add     r1  = r0, 0xF0000000;
                addi    r2  = r0, 1;
                lbl     r3  = [r1 + 0];
                andi    r3  = r3, 1;
                bne     r3 != r2, -4;
                nop     0;
                nop     0;
                addi    r2 = r0, 0x21;
                sbl     [r1 + 1] = r2;
                halt;
