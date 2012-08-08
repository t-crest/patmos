#
# Basic instructions test
#

	addi	r1 = r0, 255;  # first instruction not executed
	addi	r1 = r0, 255; # r1 = 255
	addi	r2 = r0, 5;
	swl	[r1 + 1] = r2;
	nop	0;
	lwl	r10  = [r1 + 1];
	halt; 
