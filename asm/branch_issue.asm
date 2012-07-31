#
# There is an issue on branching to the same instruciton
#

# TODO: looks like the UART is in memory address 0....

	addi	r0 = r0, 0;  # first instruction not executed
	addi	r1 = r0, 1;
	addi	r2 = r0, 2;
	addi	r3 = r0, 3;
	bc	4;
	halt;
