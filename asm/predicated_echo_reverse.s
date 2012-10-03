#
# This is a simple echo program on the UART
#
# Expected Result: echo entered characters
# SA: this test predicated ld/st which was a bug
#

		addi	r0 = r0, 0;  # first instruction not executed
		addi	r21 = r0, 0;
		addi    r8 = r0, 0;
x0:		addi	r5 = r0, 15;
		addi    r7 = r0, 25;
		sli	r5 = r5, 28;
		addi	r20 = r0, 1;
		addi	r21 = r0, 2;
		cmpneq  p2 = r20, r21;

		addi	r1   = r0 , 2;
x1:	(p2)	lwl     r10  = [r5 + 0];
		addi	r0 = r0, 0;
                and     r11  = r10 , r1;
		cmpneq  p1 = r1, r11;
	(p1)	bc	x1;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;		

                lwl     r15  = [r5 + 1];		

		addi	r3 = r0, 1;
x2:		lwl     r10  = [r5 + 0];
		addi	r0 = r0, 0;
		and     r11 = r3 , r10;
                cmpneq  p1 = r3, r11;
	(p1)	bc	x2;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;

		swl	[r5 + 1] = r15;
		swm     [r21 + 0] = r15;
		addi	r21 = r21 , 1;
		addi    r8 = r8, 1;
		cmpneq  p3 = r7, r8;
	(p3)	bc      x0;
		nop	0;
		nop	0;

x5:		lwl     r10  = [r5 + 0];
		addi	r0 = r0, 0;
		and     r11 = r3 , r10;
                cmpneq  p5 = r3, r11;
	(p5)	bc	x5;
		addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;

		lwm     r15 = [r21 + 0];
		nop	0;
		addi    r13 = r0, 33;
		swl	[r5 + 1] = r13;
#		addi	r0 = r0, 0;
		subi    r21 = r21, 1;
		cmpneq	p4 = r21, r0;
	(p4)	bc      x5;
		addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;
		bc      x0;
                halt;

