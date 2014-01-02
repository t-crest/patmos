#
# Expected Result: r1 = 0xa00000
#

                .word   36;
        (p0)    add     r1  = r0 , 0x500000;
        (p0)    add     r1  = r1 , 0x500000;
                halt;
		nop;
		nop;
		nop;
