#
# Expected Result: r1 = 0xfb
#

                 .word   116;
		add     r31  = r0, 0x200;
		add     r13  = r0, 0x200;
		addi    r1  = r0, 0x200;
		addi    r11  = r0, 0x300;
		nop;
                swc     [r13 + 0] = r1;
                shc     [r10 + 2] = r1;
                sbc     [r10 + 8] = r1;
                lwc     r3  = [r31 + 0];
                lwc     r4  = [r31 + 1];
                lwc     r5  = [r31 + 2];
		swc     [r1 + 0] = r1;
		swc     [r31 + 0] = r11;
		nop;
                lwc     r23  = [r13 + 0];
		nop;
		nop;
		addi    r21  = r0, 0x300;
                halt;
		nop;
		nop;
		nop;
