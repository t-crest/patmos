# Test case for forwarding from the ALU
	nop		0;
	addi	r1 = r0, 2;		||	addi	r1 = r0, 5;
	add 	r2 = r1, r1;	||	add 	r3 = r0, r0;
	add 	r4 = r3, r2;	||	add 	r5 = r4, r2;
	add 	r6 = r3, r5;	||	add 	r7 = r4, r5;
	sub		r1 = r2, r3;	||	sub		r1 = r1, r4;
	halt;
