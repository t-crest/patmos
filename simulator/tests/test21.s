#
# Expected Result: base = 0x00000000 & pc = 0x00000054 & r1 = 60
#
                addi     r1  = r0, 1;
                addi     r1  = r1, 2;
		cmpneq	 p1 = r1, r2;
           (p1) br       next;
                addi     r1  = r1, 3;
                addi     r1  = r1, 4;
                addi     r1  = r1, 5;
                addi     r1  = r1, 6;
                addi     r1  = r1, 7;
                halt;
next:           addi     r1  = r1, 8;
                addi     r1  = r1, 9;
                addi     r1  = r1, 10;
                addi     r1  = r1, 11;
                addi     r1  = r1, 12;
                halt;
