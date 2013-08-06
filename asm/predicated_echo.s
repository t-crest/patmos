#
# This is a simple echo program on the UART
#
# Expected Result: echo entered characters
# SA: this tests predicated ld/st which was a bug
#

		.word   116;
x0:		addi	r0 = r0, 0;  # first instruction not executed
		addi	r5 = r0, 15;
		sli	r5 = r5, 28;
		addi	r20 = r0, 1;
		addi	r21 = r0, 2;
		cmpneq  p2 = r2, r21;

		addi	r1   = r0 , 2;
x1:	(p2)	lwl     r10  = [r5 + 0];
		addi	r0 = r0, 0;
                and     r11  = r10 , r1;
		cmpneq  p1 = r1, r11;
	(p1)	br	x1;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;		
#		addi	r5 = r5, 1;

                lwl     r15  = [r5 + 1];

#		subi	r5 = r5, 1;
		addi	r3 = r0, 1;
x2:		lwl     r10  = [r5 + 0];
		addi	r0 = r0, 0;
		and     r11 = r3 , r10;
		cmpneq  p1 = r3, r11;
	(p1)	br	x2;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;

#		addi    r5 = r5, 1;
		swl	[r5 + 1] = r15;
		br	x0;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;
                halt;

