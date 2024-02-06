# Test case for add instruction
	.word	52
	addi	r0 = r0, 2		||	addi	r1 = r0, 5
	add 	r2 = r1, r1		||	add 	r3 = r1, r1
	add 	r4 = r1, r1		||	add 	r5 = r1, r1
	add 	r6 = r1, r1		||	add 	r7 = r1, r1
	halt
	nop
	nop
	nop
