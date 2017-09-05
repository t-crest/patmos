# Test case for add instruction
	.word	64
	addi	r0 = r0, 0
	addi	r1 = r0, 2		||	addi	r1 = r0, 5
	subi 	r2 = r1, 2		||	addi 	r3 = r1, 347
	ori 	r4 = r1, 5		||	subi 	r5 = r1, 1024
	nor 	r6 = r1, r4		||	and 	r7 = r1, r5
	add 	r7 = r0, 1234567
	halt
	nop
	nop
	nop
