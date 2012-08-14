#
# This is a simple output of a single character on the UART
#
# TODO: maybe this should just switch a LED to see the result.
#

# TODO: looks like the UART is in memory address 0....

	addi	r0 = r0, 0;  # first instruction not executed
	addi	r1 = r0, 1;
	add	r2 = r0, 65536;
	addi	r3 = r0, 3;
	add	r5 = r0, 100000;
	addi	r4 = r0, 4;
	add	r6 = r0, 200000;
	add	r7 = r0, 300000; # add for a long immediate is strange
	halt;
