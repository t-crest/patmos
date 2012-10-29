#
# This is a simple output of a single character on the UART
#
# TODO: maybe this should just switch a LED to see the result.
#

# TODO: looks like the UART is in memory address 0....

	addi	r0 = r0, 0;  # first instruction not executed
	addi	r1 = r0, 0;
	addi	r2 = r0, 42; # '*'
#x2:	swm	[r1 + 1] = r2;
	addi	r1   = r0 , 2;
	addi	r3 = r19 , 1;
x1:	lwm     r10  = [r5 + 0];
	nop	0;
        and     r11  = r10 , r3;
	cmpneq  p1 = r11 , r1;
	nop	0;
	nop	0;
	nop	0;
	(p1) 	bc x1;
	nop 	0;
	nop	0;
	swm	[r7 + 1] = r2;
#	br	x2;
	halt
