# Test case for global data memory load and store
# Initialization begin
	add 	r1 = r0, 0xFF0FF000;
	add 	r1 = r0, 0xFF0FF000;
	addi	r2 = r0, 4;
	swm		[r2 + 0] = r0;
	swm		[r2 + 1] = r0;
	swm		[r2 + 2] = r0;
	add		r0 = r0, r0;
# Initialization end
	swm		[r2 + 0] = r1;
	shm		[r2 + 2] = r1;
	sbm 	[r2 + 8] = r1;
	lbm		r3 = [r2 + 0];
	lbm		r4 = [r2 + 4];
	lhm		r5 = [r2 + 4];
	lhm		r3 = [r2 + 0];
	lwm		r4 = [r2 + 1];
	lwm		r5 = [r2 + 2];

	swm		[r2 + 0] = r1;
	shm		[r2 + 1] = r1;
	sbm 	[r2 + 3] = r1;
	lbm		r3 = [r2 + 0];
	lbm		r4 = [r2 + 1];
	lbm		r4 = [r2 + 2];
	lbm		r4 = [r2 + 3];
	lhm		r5 = [r2 + 0];
	lhm		r3 = [r2 + 1];
	lwm		r4 = [r2 + 0];

	swm		[r0 + 1] = r1;
	shm		[r0 + 2] = r1;
	sbm 	[r0 + 4] = r1;
	lbm		r3 = [r2 + 4];
	lbm		r4 = [r2 + 5];
	lbm		r4 = [r2 + 6];
	lbm		r4 = [r2 + 7];
	lhm		r5 = [r2 + 2];
	lhm		r3 = [r2 + 3];
	lwm		r4 = [r2 + 1];

	addi	r1 = r0, 111;
	sli		r2 = r1, 8;
	addi	r2 = r2, 222;
	sli 	r3 = r2, 16;
	addi 	r3 = r3, 84;
	swm		[r0 + 0] = r3;
	shm		[r0 + 1] = r2;
	sbm		[r0 + 1] = r1;
	lwm		r4 = [r0 + 1];
	lhm		r5 = [r0 + 1];
	lbm		r6 = [r0 + 1];
	swm		[r0 + 2] = r3;
	shm		[r0 + 2] = r2;
	sbm		[r0 + 2] = r1;
	lhum	r5 = [r0 + 2];
	lbum	r6 = [r0 + 2];
	halt;