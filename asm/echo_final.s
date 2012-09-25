#
# This is a simple echo program on the UART
#
# Expected Result: echo entered characters
#
# MS: is this roughly the same as echo.s? 
# SA: This is a deprecated version of echo, it is no longer valid.

		addi	r0 = r0, 0;  # first instruction not executed
		addi	r5 = r0, 15;
		sli	r5 = r5, 28;

		addi	r1   = r0 , 2;
		lwl     r10  = [r5 + 0];
		addi	r0 = r0, 0;
                and     r11  = r10 , r1;
		cmpneq  p1 = r1, r11;
	(p1)	bc	4;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;		
		addi	r5 = r5, 1;

                lwl     r15  = [r5 + 0];

		subi	r5 = r5, 1;
		addi	r3 = r0, 1;
		lwl     r10  = [r5 + 0];
		addi	r0 = r0, 0;
		and     r11 = r3 , r10;
		cmpneq  p1 = r3, r11;
	(p1)	bc	12;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;

		addi    r5 = r5, 1;
		swl	[r5 + 0] = r15;
		bc	0;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;
                halt;

