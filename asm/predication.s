#
# Test predicates and branch
#
	addi	r0 = r0, 0;  # first instruction not executed
	addi	r1 = r0, 2;
	addi	r2 = r0, 2;
	cmpeq   p1 = r1, r2;
	pand    p2 = p0 , p1;
x1:	addi    r10 = r0, 1; # this is just to jump!
	(p2)    br	x1;
	addi    r1 = r1, 1;
	cmpeq   p2 = r1, r2;
	addi	r3 = r0, 1;
	subi    r1 = r1, 1; # just to check register file, r1 = 2, 3, 4, 3
	por     p3 = p0, p1; #p3 = 1
	pxor    p3 = p0, p1; #p3 = 0	
	pnor    p1 = p0, p3; #p1 = 0
	pand    p1 = p0, !p1;#p1 = 1
	halt;

