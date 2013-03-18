#
# Basic instructions test
#

	addi	r1 = r0, 255;  # first instruction not executed
	addi	r1 = r0, 0; # r1 = 255
	addi	r2 = r0, 5;
	addi	r0 = r0, 0;
	swl	[r1 + 4] = r2;
	addi	r0 = r0, 0;
	addi	r0 = r0, 0;
	lwl	r3  = [r1 + 4];
	addi	r0 = r0, 0;
	addi	r0 = r0, 0;
	halt; 
