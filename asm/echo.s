#
# This is a simple echo program on the UART
#
# Expected Result: echo entered characters
# SA: this is the working version of echo.

		.word   104;
# Set up IO address
x0:		addi	r0 = r0, 0;
		add 	r5 = r0, 0xf0000100;

# Wait until receive bit is set
		addi	r1  = r0, 2;
x1:		lwl     r10 = [r5 + 0];
		addi	r0  = r0, 0;
		and     r11 = r10, r1;
		cmpneq  p1  = r1, r11;
	(p1)	br	x1;
		addi    r0  = r0, 0;
		addi    r0  = r0, 0;		

# Read character
		lwl     r15  = [r5 + 1];

# Wait until send ready bit is set
		addi	r3 = r0, 1;
x2:		lwl     r10  = [r5 + 0];
		addi	r0 = r0, 0;
		and     r11 = r3 , r10;
		cmpneq  p1 = r3, r11;
	(p1)	br	x2;
		addi    r0  = r0, 0;
		addi    r0  = r0, 0;

# Write character
		swl	[r5 + 1] = r15;
		br	x0;
		addi    r0  = r0, 0;
		addi    r0  = r0, 0;

# Never reached
		halt;
