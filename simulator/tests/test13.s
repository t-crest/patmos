#
# Expected Result: base = 0x00000000 & pc = 0x00000028 &
#                  r0 = 45 & r1 = 00000000 & r2 = 0000000c
#
                bs       8;
                addi     r0  = r0, 1;
                addi     r0  = r0, 2;
                addi     r0  = r0, 3;
                addi     r0  = r0, 4;
                halt;
                .word    10;
                addi     r0  = r0, 5;
                addi     r0  = r0, 6;
                addi     r0  = r0, 7;
                addi     r0  = r0, 8;
                addi     r0  = r0, 9;
                mfs      r1  = s4;
                mfs      r2  = s5;
                ret;
