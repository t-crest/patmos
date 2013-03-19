#
# Simple test of a SPM
#

	addi	r1 = r0, 255;  # first instruction not executed
	addi	r1 = r0, 0; # r1 = 255
	addi	r2 = r0, 5;
	swl	[r1 + 4] = r2;
	lwl	r3  = [r1 + 4];
	lwl	r4  = [r1 + 4];
	lwl	r5  = [r1 + 4];
	lwl	r6  = [r1 + 4];
	lwl	r7  = [r1 + 4];
	lwl	r8  = [r1 + 4];
	lwl	r9  = [r1 + 4];
	addi	r0 = r0, 0;
	addi	r0 = r0, 0;
	halt; 
