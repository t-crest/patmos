#
# Test of load/use delay in stack memory
#
# Expected Result: 
#   first load (in the load use delay slot) will get old value
#   second load the correct value from memory
#
	.word   72;
	addi    r3 = r0, 0x100;
	mts     s6 = r3;
	sres	10;
	addi	r1 = r0, 4;
	addi	r2 = r0, 2;
	addi	r3 = r0, 3;
	addi	r4 = r0, 4;
	addi	r5 = r0, 5;
	sws	[r1+4] = r2;
	lws	r3 = [r1+4];
	addi    r0 = r0, 0;	# This is the delay slot
	add	r4 = r0, r3;	# that one is in the delay slot and will add 3
	add	r5 = r0, r3;	# that one shall add 2
	add	r1 = r0, r5;	# that one shall now be 2
	halt;
	nop;
	nop;
	nop;
