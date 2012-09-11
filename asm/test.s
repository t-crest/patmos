#
# This is a simple output of a single character on the UART
#
# TODO: maybe this should just switch a LED to see the result.
#

# TODO: looks like the UART is in memory address 0....

	addi	r0 = r0, 0;  # first instruction not executed
	addi	r1 = r0, 15;
	sli	r1 = r1, 28;
	addi	r2 = r0, 42; # '*'
	swm	[r1 + 1] = r2;
	bc	4;
	halt;
