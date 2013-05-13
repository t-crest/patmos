# Test case for local memory load and store
# Initialization begin
	add 	r1 = r0, 0xFF0FF000;
	add 	r1 = r0, 0xFF0FF000;
	addi	r2 = r0, 4;
	swl		[r2 + 0] = r0;
	swl		[r2 + 1] = r0;
	swl		[r2 + 2] = r0;
	add		r0 = r0, r0;
# Initialization end
	swl		[r2 + 0] = r1;
	shl		[r2 + 2] = r1;
	sbl 	[r2 + 8] = r1;
	lbl		r3 = [r2 + 0];
	lbl		r4 = [r2 + 4];
	lhl		r5 = [r2 + 4];
	lhl		r3 = [r2 + 0];
	lwl		r4 = [r2 + 1];
	lwl		r5 = [r2 + 2];

	swl		[r2 + 0] = r1;
	shl		[r2 + 1] = r1;
	sbl 	[r2 + 3] = r1;
	lbl		r3 = [r2 + 0];
	lbl		r4 = [r2 + 1];
	lbl		r4 = [r2 + 2];
	lbl		r4 = [r2 + 3];
	lhl		r5 = [r2 + 0];
	lhl		r3 = [r2 + 1];
	lwl		r4 = [r2 + 0];

	swl		[r0 + 1] = r1;
	shl		[r0 + 2] = r1;
	sbl 	[r0 + 4] = r1;
	lbl		r3 = [r2 + 4];
	lbl		r4 = [r2 + 5];
	lbl		r4 = [r2 + 6];
	lbl		r4 = [r2 + 7];
	lhl		r5 = [r2 + 2];
	lhl		r3 = [r2 + 3];
	lwl		r4 = [r2 + 1];

	addi	r1 = r0, 111;
	sli		r2 = r1, 8;
	addi	r2 = r2, 222;
	sli 	r3 = r2, 16;
	addi 	r3 = r3, 84;
	swl		[r0 + 0] = r3;
	shl		[r0 + 1] = r2;
	sbl		[r0 + 1] = r1;
	lwl		r4 = [r0 + 1];
	lhl		r5 = [r0 + 1];
	lbl		r6 = [r0 + 1];
	swl		[r0 + 2] = r3;
	shl		[r0 + 2] = r2;
	sbl		[r0 + 2] = r1;
	lhul	r5 = [r0 + 2];
	lbul	r6 = [r0 + 2];
	halt;