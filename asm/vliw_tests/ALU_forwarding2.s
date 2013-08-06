# Test case for forwarding of ALU instructions
	.word	68;
	addi	r4 = r0, 0;
	addi	r4 = r0, 0;
	addi	r1 = r0, 2		||	addi	r1 = r0, 5;
	add 	r2 = r1, r1		||	add 	r3 = r1, r1;
	add 	r6 = r3, r5		||	add 	r7 = r4, r5;
#	subi	r8 = r0, 8		||	subi	r9 = r0, 5;
	addi	r12 = r0, 70;
	subi	r1 = r0, 3		||	or		r13 = r12, r11;
	andi	r15 = r12, 5	||	ori 	r16 = r12, 5;
#	rl		r16 = r10, r1	||	rr 		r17 = r10, r1;
#	rll		r16 = r10, 3	||	rrl		r17 = r10, 2;
#	xor		r19 = r12, r1	||	xori 	r18 = r12, 5;
#	nor		r19 = r12, r1	||	nori 	r18 = r12, 5;
	shadd	r14 = r12, r1	||	shadd2	r15 = r12, r1;
#	shaddi	r14 = r12, r1;	||	shadd2i	r15 = r12, r1;
	halt;
