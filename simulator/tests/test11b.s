#
# Expected Result: base = 0x00000004 & pc = 0x00000048 & r1 = 38 & r2 = 255
#

                .word    15;
                addi     r1  = r0, 1;
                br       x;
                addi     r1  = r1, 2;
                addi     r1  = r1, 3  || addi r2 = r0, 255;
                addi     r1  = r1, 4;
                halt;
x:              addi     r1  = r1, 5;
                addi     r1  = r1, 6;
                addi     r1  = r1, 7;
                addi     r1  = r1, 8;
                addi     r1  = r1, 9;
                halt;
