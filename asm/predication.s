#
# Test predicates and branch
#
	.word   268;
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
	pand	p1 = !p0, !p0; #p1 = 0
	pand    p1 = p0, !p1;#p1 = 1

####################
# predicated load/store
####################
	addi	r10 = r0, 0; # the address of memory operations (to allow the changes in one place)
	subi	r1 = r0, 3; # just arbitrary value
####################
# predicated store
#	Test the p0 seperately, assuming it might be handled specially
(p0)	swl	[r10+0] = r1;
	lwl	r2 = [r10+0];
	subi	r1 = r1, 1;
(!p0)	swl	[r0+0] = r1;
	lwl	r3 = [r10+0];
#	Test arbitrary other pridicate, assuming the code would be general, and all predicates would work the same way
	pand	p1 = p0, p0; #p1 = 1
	subi	r1 = r1, 1;
(p1)	swl	[r10+0] = r1;
	lwl	r2 = [r10+0];
	subi	r1 = r1, 1;
(!p1)	swl	[r10+0] = r1;
	lwl	r3 = [r10+0];

####################	
# predicated load
#	Test the p0 seperately, assuming it might be handled specially
	subi	r1 = r1, 1;
	swl	[r10+0] = r1;
(p0)	lwl	r2 = [r10+0];
	subi	r1 = r1, 1;
	swl	[r10+0] = r1;
(!p0)	lwl	r3 = [r10+0];
#	Test arbitrary other pridicate, assuming the code would be general, and all predicates would work the same way
	pand	p2 = p0, p0; #p2 = 1
	subi	r1 = r1, 1;
	swl	[r10+0] = r1;
(p2)	lwl	r2 = [r10+0];
	subi	r1 = r1, 1;
	swl	[r10+0] = r1;
(!p2)	lwl	r3 = [r10+0];
		addi	r0 = r0, 0;

####################	
# predicated branches (other jump instructions should probably be added also)
####################	
#	Test the p0 seperately, assuming it might be handled specially
	(p0)	br	pb1;
		  addi	r0 = r0, 0;
		  addi	r0 = r0, 0;
		subi	r1 = r1, 1;
pb1:	(!p0)	br	pb2;
		  addi	r0 = r0, 0;
		  addi	r0 = r0, 0;
		subi	r1 = r1, 1;
pb2: 		subi	r1 = r1, 1;
#	Test arbitrary other pridicate, assuming the code would be general, and all predicates would work the same way
		pand	p3 = p0, p0; #p3 = 1
	(p3)	br	pb3;
		  addi	r0 = r0, 0;
		  addi	r0 = r0, 0;
		subi	r1 = r1, 1;
pb3:	(!p3)	br	pb4;
		  addi	r0 = r0, 0;
		  addi	r0 = r0, 0;
		subi	r1 = r1, 1;
pb4:		subi	r1 = r1, 1;

#############
# Done
#############
	halt;
	nop;
	nop;
	nop;
