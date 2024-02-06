# Test case for forwarding of ALU instructions
	.word	80
	addi	r4 = r0, 0
	addi	r4 = r0, 0
	addi	r1 = r0, 2		||	addi	r0 = r0, 5
	add 	r2 = r1, r1		||	add 	r3 = r1, r1
	add 	r6 = r3, r5		||	add 	r7 = r4, r5
	subi	r8 = r0, 8		||	subi	r9 = r0, 5
	addi	r12 = r0, 70
	subi	r1 = r0, 3		||	or		r13 = r12, r11
	andi	r15 = r12, 5	||	ori 	r16 = r12, 5
	xor		r19 = r12, r1	||	xori 	r18 = r12, 5
	nor		r19 = r12, r1	||	nor 	r18 = r12, r2
	shadd	r14 = r12, r1	||	shadd2	r15 = r12, r1
	halt
	nop
	nop
	nop
	