#
# Expected Result: r1 = 1
#
                .word   16;
		call    x;
		nop;
		nop;
		nop;
		.word   64;
x:              addi    r1 = r0, 0;
		addi    r30 = r0, 0;
		addi    r31 = r0, (y-x);
		addi    r30 = r0, x;
		brcfr    r30, r31;
                nop;
                nop;
		nop;
		addi    r1 = r1, 1;
y:              addi    r1 = r1, 1;
		addi    r31 = r0, 0;
		addi    r30 = r0, 0;
		brcfr   r30, r31;
		nop;
		nop;
		nop;
