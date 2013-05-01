#
# Basic load/store tests

	addi	r0 = r0, 0;
	addi	r1 = r0, 4;
	add	r2 = r0, 0xabcd1234;
	swl	[r1 + 0] = r2;
	lwl	r3 = [r1 + 0];
	lbl	r4 = [r1 + 0];
	lbl	r4 = [r1 + 1];
	lbl	r4 = [r1 + 2];
	lbl	r4 = [r1 + 3];
	addi	r1 = r0, 8;
	addi	r2 = r0, 0x12;
	sbl	[r1 + 0] = r2;
	lwl	r3 = [r1 + 0];
	addi	r2 = r0, 0x34;
	sbl	[r1 + 1] = r2;
	lwl	r3 = [r1 + 0];
	addi	r2 = r0, 0x45;
	sbl	[r1 + 2] = r2;
	lwl	r3 = [r1 + 0];
	addi	r2 = r0, 0x67;
	sbl	[r1 + 3] = r2;
	lwl	r3 = [r1 + 0];
	halt; 
