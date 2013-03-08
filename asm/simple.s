#
# A very simple example to get stuff running on a (chisel) pipeline.
# Now addition with forwarding.
#

	addi	r0 = r0, 0;  # first instruction not executed

	addi	r1 = r0, 1;
	addi	r2 = r0, 2;
	add	r3 = r1, r2;
	sub	r4 = r2, r1;
	subi	r4 = r1, 3;
	rsub	r4 = r2, r1;
	or	r4 = r1, r2;
	and	r4 = r3, r1;
	and	r4 = r3, r2;
	sl	r4 = r1, 8;
	sr	r4 = r2, 1;
	subi	r3 = r0, 2;
	sra	r4 = r3, 1;
	sr	r4 = r3, 1;



	addi	r0 = r0, 0;
	addi	r0 = r0, 0;
	addi	r0 = r0, 0;

	halt;
