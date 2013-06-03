# Test case for stack cache load and store
# Initialization begin
	add 	r1 = r0, 0xFF0FF000;
	add 	r1 = r0, 0xFF0FF000;
	addi	r2 = r0, 4;
	addi    r3 = r0, 0x100;
	mts     s6 = r3;
	sres    32;
	sws		[r2 + 0] = r0;
	sws		[r2 + 1] = r0;
	sws		[r2 + 2] = r0;
	add		r0 = r0, r0;
# Initialization end
	sws		[r2 + 0] = r1;
	shs		[r2 + 2] = r1;
	sbs 	[r2 + 8] = r1;
	lbs		r3 = [r2 + 0];
	lbs		r4 = [r2 + 4];
	lhs		r5 = [r2 + 4];
	lhs		r3 = [r2 + 0];
	lws		r4 = [r2 + 1];
	lws		r5 = [r2 + 2];

	sws		[r2 + 0] = r1;
	shs		[r2 + 1] = r1;
	sbs 	[r2 + 3] = r1;
	lbs		r3 = [r2 + 0];
	lbs		r4 = [r2 + 1];
	lbs		r4 = [r2 + 2];
	lbs		r4 = [r2 + 3];
	lhs		r5 = [r2 + 0];
	lhs		r3 = [r2 + 1];
	lws		r4 = [r2 + 0];

	sws		[r0 + 1] = r1;
	shs		[r0 + 2] = r1;
	sbs 	[r0 + 4] = r1;
	lbs		r3 = [r2 + 4];
	lbs		r4 = [r2 + 5];
	lbs		r4 = [r2 + 6];
	lbs		r4 = [r2 + 7];
	lhs		r5 = [r2 + 2];
	lhs		r3 = [r2 + 3];
	lws		r4 = [r2 + 1];

	addi	r1 = r0, 111;
	sli		r2 = r1, 8;
	addi	r2 = r2, 222;
	sli 	r3 = r2, 16;
	addi 	r3 = r3, 84;
	sws		[r0 + 0] = r3;
	shs		[r0 + 1] = r2;
	sbs		[r0 + 1] = r1;
	lws		r4 = [r0 + 1];
	lhs		r5 = [r0 + 1];
	lbs		r6 = [r0 + 1];
	sws		[r0 + 2] = r3;
	shs		[r0 + 2] = r2;
	sbs		[r0 + 2] = r1;
	lhus	r5 = [r0 + 2];
	lbus	r6 = [r0 + 2];
	halt;
