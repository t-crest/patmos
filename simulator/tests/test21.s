#
# Expected Result: base = 0x00000000 & pc = 0x00000050 & r0 = 65
#
                addi     r0  = r0, 1;
                addi     r0  = r0, 2;
                bne      r0 != r1, 6;
                addi     r0  = r0, 3;
                addi     r0  = r0, 4;
                addi     r0  = r0, 5;
                addi     r0  = r0, 6;
                addi     r0  = r0, 7;
                halt;
                addi     r0  = r0, 8;
                addi     r0  = r0, 9;
                addi     r0  = r0, 10;
                addi     r0  = r0, 11;
                addi     r0  = r0, 12;
                halt;
