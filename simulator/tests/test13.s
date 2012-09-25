#
# Expected Result: base = 0x00000000 & pc = 0x00000048 &
#                  r1 = 45 & r2 = 00000000 & r3 = 0000000c
#
                call     8;
                addi     r1  = r0, 1;
                addi     r1  = r1, 2;
                addi     r1  = r1, 3;
                addi     r1  = r1, 4;
                halt;
                .word    10;
                addi     r1  = r1, 5;
                addi     r1  = r1, 6;
                addi     r1  = r1, 7;
                addi     r1  = r1, 8;
                addi     r1  = r1, 9;
                add      r2  = r30, r0;
                add      r3  = r31, r0;
                ret      r30, r31;
