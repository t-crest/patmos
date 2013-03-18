# This test case tests the branching
	nop 	0;
	addi 	r1 = r0, 2;
	addi	r2 = r0, 2;
	cmpneq	p1 = r0, r1;
x2:	(p1) br x1;
	add		r2 = r2, 2;
	nop 	0;
	add 	r2 = r1, r1;
x1:	cmpeq 	p2 = r2, r1;
	(p2) br x2;
	nop		0;
	nop		0;
	halt;