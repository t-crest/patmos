#
# A very simple example to et stuff running on a (chisel) pipeline
# that has no forwarding yet.
#

	addi	r0 = r0, 0;  # first instruction not executed

	addi	r1 = r0, 1;
	addi	r0 = r0, 0;
	addi	r0 = r0, 0;
	addi	r0 = r0, 0;

	addi	r2 = r0, 2;
	addi	r0 = r0, 0;
	addi	r0 = r0, 0;
	addi	r0 = r0, 0;

	add	r3 = r1, r2;
	addi	r0 = r0, 0;
	addi	r0 = r0, 0;
	addi	r0 = r0, 0;

	halt;
