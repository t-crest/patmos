#
# This is a simple echo program on the UART
#
# Expected Result: echo entered characters
# SA: this test predicated ld/st which was a bug
#

		.word   224;
		addi	r0 = r0, 0;  # first instruction not executed
		addi	r22 = r0, 0;
		addi    r8 = r0, 0;
		addi    r7 = r0, 25;
x0:		addi	r5 = r0, 15;
		sli	r5 = r5, 28;
		addi	r20 = r0, 1;
		addi	r21 = r0, 2;
		cmpneq  p2 = r20, r21;

		addi	r1   = r0 , 2;
x1:	(p2)	lwl     r10  = [r5 + 0];
		addi	r0 = r0, 0;
                and     r11  = r10 , r1;
		cmpneq  p1 = r1, r11;
	(p1)	br	x1;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;		

                lwl     r15  = [r5 + 1];		

		addi	r3 = r0, 1;
x2:		lwl     r10  = [r5 + 0];
		addi	r0 = r0, 0;
		and     r11 = r3 , r10;
                cmpneq  p1 = r3, r11;
	(p1)	br	x2;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;

		swl	[r5 + 1] = r15;
		swl     [r8 + 0] = r15;
		addi	r22 = r22 , 1;
		addi    r8 = r8, 1;
		cmpneq  p3 = r7, r8;
	(p3)	br      x0;
		nop;
		nop;

		subi    r8 = r8, 1;
x5:		lwl     r10  = [r5 + 0];
		addi	r0 = r0, 0;
		and     r11 = r3 , r10;
                cmpneq  p5 = r3, r11;
	(p5)	br	x5;
		addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;

		lwl     r15 = [r8 + 0];
		nop;
		addi    r13 = r0, 33;
		swl	[r5 + 1] = r15;
#		addi	r0 = r0, 0;
		subi    r8 = r8, 1;
		cmpneq	p4 = r8, r0;
	(p4)	br      x5;
		addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;
		br      x0;
		addi	r8 = r0, 0;
		addi    r22 = r0, 0;		
                halt;

