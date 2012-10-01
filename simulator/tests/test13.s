#
# Expected Result: base = 0x00000000 & pc = 0x00000048 &
#                  r1 = 45 & r2 = 00000000 & r3 = 0000000c
#
                addi     r30 = r0, 0;
                call     x;
                addi     r1  = r0, 1;
                addi     r1  = r1, 2;
                addi     r1  = r1, 3;
                addi     r1  = r1, 4;
                halt;
                .word    10;
x:              addi     r1  = r1, 5;
                addi     r1  = r1, 6;
                addi     r1  = r1, 7;
                addi     r1  = r1, 8;
                addi     r1  = r1, 9;
                addi     r2  = r30, 0;
                addi     r3  = r31, 0;
                ret      r30, r31;
