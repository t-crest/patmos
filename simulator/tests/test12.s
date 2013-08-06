#
# Expected Result: base = 0x0000001c & pc = 0x00000044 & r0 = 38
#

                .word    7;
                brcf     x;
                addi     r1  = r0, 1;
                addi     r1  = r1, 2;
                addi     r1  = r1, 3;
                addi     r1  = r1, 4;
                halt;
                .word    7;
x:              addi     r1  = r1, 5;
                addi     r1  = r1, 6;
                addi     r1  = r1, 7;
                addi     r1  = r1, 8;
                addi     r1  = r1, 9;
                halt;
