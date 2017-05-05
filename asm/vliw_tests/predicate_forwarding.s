# Test case for predicate forwarding.
	.word	96
	addi	r0 = r0, 0
	addi	r1 = r0, 5		||	addi	r2 = r0, 10
	por		p1 = p0, p0 	||	pxor	p2 = p0, p0
(p1) add 	r5 = r1, r2 	||(p1) add 	r6 = r1, r2
(p1) add 	r5 = r1, r2 	||(p1) add 	r6 = r1, r2
(p1) add 	r5 = r1, r2 	||(p1) add 	r6 = r1, r2
	por		p5 = p0, p0 	||	pxor	p6 = p0, p0
	pxor	p3 = p5, p6 	||	por	p4 = p5, p6
	pxor	p3 = p5, p6 	||	por	p4 = p5, p6
	pxor	p3 = p5, p6 	||	por	p4 = p5, p6

	halt
	nop
	nop
	nop
