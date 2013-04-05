#
# Basic instructions test
# test if memory works fine

	addi	r1 = r0, 255;  # first instruction not executed
	addi	r1 = r0, 256; # r1 = 256
	addi    r29 = r0, 10;
	addi    r30 = r0, 0;
	addi	r2 = r0, 10;
#	swl	[r1 + 1] = r2;
	nop;
x1:	swm	[r1 + 1] = r2;
	addi    r5 = r0, 5;
	lwm	r10  = [r1 + 1];
	addi    r2 = r2, 1;
	subi    r29 = r29, 1;
	cmpneq  p1 = r0, r29;
(p1)	br	x1; #
	halt; 
