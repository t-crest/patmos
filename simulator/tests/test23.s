#
# Expected Result: r2 = 0x21
#

                .word   68;
                add     r1  = r0, 0xF0000800;
                addi    r2  = r0, 1;
                lwl     r3  = [r1 + 0];
		nop;
                andi    r3  = r3, 1;
		cmpneq  p1 = r3, r2;
          (p1)  br      -3;
                nop;
                nop;
                addi    r2 = r0, 0x21;
                swl     [r1 + 1] = r2;
                halt;
		nop;
		nop;
		nop;
