#
# Expected Result: r1 = 0xfb
#

                 .word   68;
		add     r31  = r0, 0x200;
		addi    r1  = r0, 0x200;
		nop;
                swc     [r31 + 0] = r1;
                shc     [r31 + 2] = r1;
                sbc     [r31 + 8] = r1;
                lwc     r3  = [r31 + 0];
                lwc     r4  = [r31 + 1];
                lwc     r5  = [r31 + 2];
		swc     [r1 + 0] = r1;
                shc     [r1 + 2] = r1;
                halt;
		nop;
		nop;
		nop;
