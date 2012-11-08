#
# Test predicates and branch
#
	addi	r0 = r0, 0;  # first instruction not executed
	addi	r1 = r0, 2;
	addi	r2 = r0, 2;
	cmpeq   p1 = r1, r2;
	pand    p2 = p0 , p1;
x1:	addi    r10 = r0, 1; # this is just to jump!
(p2)    pand	p5 = p0, p1;
(p5)	addi    r1 = r1, 1;

	halt;

