# Test case for addi instruction
	.word	52
	addi	r1 = r0, 2	||	addi	r1 = r0, 5
	addi	r2 = r1, 3	||	addi	r3 = r1, 3
	addi	r4 = r1, 4	||	addi	r5 = r1, 4
	addi	r6 = r1, 5	||	addi	r7 = r1, 5
	halt
	nop
	nop
	nop
