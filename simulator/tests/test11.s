#
# Expected Result: base = 0x00000000 & pc = 0x00000044 & r0 = 38
#
                bc       7;
                addi     r0  = r0, 1;
                addi     r0  = r0, 2;
                addi     r0  = r0, 3;
                addi     r0  = r0, 4;
                halt;
                addi     r0  = r0, 5;
                addi     r0  = r0, 6;
                addi     r0  = r0, 7;
                addi     r0  = r0, 8;
                addi     r0  = r0, 9;
                halt;
