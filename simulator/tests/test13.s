#
# Expected Result: base = 0x00000004 & pc = 0x00000028 &
#                  r1 = 45 & r2 = 00000004 & r3 = 00000014
#

                .word    44;
                addi     r30 = r0, 0;
                call     x;
                addi     r1  = r0, 1;
                addi     r1  = r1, 2;
                addi     r1  = r1, 3;
                addi     r1  = r1, 4;
                halt;
		nop;
		nop;
		nop;
                .word    52;
x:              addi     r1  = r1, 5;
                addi     r1  = r1, 6;
                addi     r1  = r1, 7;
                addi     r1  = r1, 8;
                addi     r1  = r1, 9;
                mfs      r2  = srb;
                mfs      r3  = sro;
		ret;
		nop;
		nop;
		nop;
