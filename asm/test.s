#
# This is a simple output of a single character on the UART
#
# TODO: maybe this should just switch a LED to see the result.
#

# TODO: looks like the UART is in memory address 0....

	.word   44;
	addi	r0 = r0, 0;  # first instruction not executed
	add	r1 = r0, 0xf0000800;
	addi	r2 = r0, 42; # '*'
x1:	swl	[r1 + 1] = r2;
	br	x1;
	halt;
	nop;
	nop;
	nop;
