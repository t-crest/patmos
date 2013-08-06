#
# Expected Result: r1 = 0xfb
#

                .word   13;
                add     r1  = r0, 0xF0000100;
                addi    r2  = r0, 2;
                lwl     r3  = [r1 + 0];
		nop;
                andi    r3  = r3, 2;
		cmpneq  p1 = r3, r2;
           (p1) br      -3;
                nop;
                nop;
                lwl     r1  = [r1 + 1];
                halt;
