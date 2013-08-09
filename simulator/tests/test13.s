#
# Expected Result: base = 0x00000028 & pc = 0x00000050 &
#                  r1 = 41 & r2 = 00000000 & r3 = 00000014
#

                .word    32;
                addi     r30 = r0, 0;
                call     x;
                addi     r1  = r0, 1;
                addi     r1  = r1, 2;
                addi     r1  = r1, 3;
                addi     r1  = r1, 4;
                halt;
                .word    40;
x:              addi     r1  = r1, 5;
                addi     r1  = r1, 6;
                addi     r1  = r1, 7;
                addi     r1  = r1, 8;
                addi     r1  = r1, 9;
                addi     r2  = r30, 0;
                addi     r3  = r31, 0;
                ret      r30, r31;
