#
# Simple test of a SPM
#

	.word   76;
	addi	r1 = r0, 255;  # first instruction maybe not executed
	addi	r1 = r0, 32;
	addi	r2 = r0, 5;
	swl	[r1 + 4] = r2;
	lwl	r3  = [r1 + 4];
	lwl	r4  = [r1 + 4];
	lwl	r5  = [r1 + 8];
	lwl	r6  = [r1 + 4];
	lwl	r7  = [r1 + 4];
	addi	r0 = r0, 0;
	addi	r0 = r0, 0;
	add	r2 = r0, 0xdeadbeef;
	swl	[r1 + 8] = r2;
	lwl	r5  = [r1 + 8];
	lwl	r6  = [r1 + 4];
	lwl	r7  = [r1 + 8];
	halt; 
