#
# FIXME: add comment
#
# Expected Result: just checking if echo works along with other instructions.
#

x0:		addi	r0 = r0, 0;  # first instruction not executed
		addi	r5 = r0, 15;
		sli	r5 = r5, 28;

		addi	r1   = r0 , 2;
x1:		lwl     r10  = [r5 + 0];
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

		addi	r13 = r0, 255; # r13 = 255
		swl	[r13 + 1] = r15; # memory address 259 (255 + (1 sl 2)) = whatever from uart 
		nop;
		lwl	r25  = [r13 + 1]; # register(25) = whatever from uart 		
		nop;
		swl	[r5 + 1] = r25;
		br	x0;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;
                halt;

