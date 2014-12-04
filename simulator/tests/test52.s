#
# Expected Result: base = 0x0000001c &
#                  r1 = 0x0000002d & r2 = 00000010 & r3 = 00000000
#

                .word    44;
                addi     r30 = r0, 0;
                callsbnd x;
                .word    42;
                addi     r1  = r1, 4;
                addi     r1  = r1, 5;
                addi     r1  = r1, 6;
                halt;
		nop;
		nop;
		nop;
                .word    52;
 x:             addi     r1  = r0, 1;
                addi     r1  = r1, 2;
                addi     r1  = r1, 3;
                addi     r1  = r1, 7;
                addi     r1  = r1, 8;
                addi     r1  = r1, 9;
                mfs      r2  = srb;
                mfs      r3  = sro;
		ret;
		nop;
		nop;
		nop;
