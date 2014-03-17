# Test case for data cache load and store
# Initialization begin
	.word	236;
	add 	r1 = r0, 0xFF0FF000;
	add 	r1 = r0, 0xFF0FF000;
	addi	r2 = r0, 4;
	swc		[r2 + 0] = r0;
	swc		[r2 + 1] = r0;
	swc		[r2 + 2] = r0;
	add		r0 = r0, r0;
# Initialization end
	swc		[r2 + 0] = r1;
	shc		[r2 + 2] = r1;
	sbc 	[r2 + 8] = r1;
	lbc		r3 = [r2 + 0];
	lbc		r4 = [r2 + 4];
	lhc		r5 = [r2 + 4];
	lhc		r3 = [r2 + 0];
	lwc		r4 = [r2 + 1];
	lwc		r5 = [r2 + 2];

	swc		[r2 + 0] = r1;
	shc		[r2 + 1] = r1;
	sbc 	[r2 + 3] = r1;
	lbc		r3 = [r2 + 0];
	lbc		r4 = [r2 + 1];
	lbc		r4 = [r2 + 2];
	lbc		r4 = [r2 + 3];
	lhc		r5 = [r2 + 0];
	lhc		r3 = [r2 + 1];
	lwc		r4 = [r2 + 0];

	swc		[r0 + 1] = r1;
	shc		[r0 + 2] = r1;
	sbc 	[r0 + 4] = r1;
	lbc		r3 = [r2 + 4];
	lbc		r4 = [r2 + 5];
	lbc		r4 = [r2 + 6];
	lbc		r4 = [r2 + 7];
	lhc		r5 = [r2 + 2];
	lhc		r3 = [r2 + 3];
	lwc		r4 = [r2 + 1];

	addi	r1 = r0, 111;
	sli		r2 = r1, 8;
	addi	r2 = r2, 222;
	sli 	r3 = r2, 16;
	addi 	r3 = r3, 84;
	swc		[r0 + 0] = r3;
	shc		[r0 + 1] = r2;
	sbc		[r0 + 1] = r1;
	lwc		r4 = [r0 + 1];
	lhc		r5 = [r0 + 1];
	lbc		r6 = [r0 + 1];
	swc		[r0 + 2] = r3;
	shc		[r0 + 2] = r2;
	sbc		[r0 + 2] = r1;
	lhuc	r5 = [r0 + 2];
	lbuc	r6 = [r0 + 2];
	halt;
	nop;
	nop;
	nop;
