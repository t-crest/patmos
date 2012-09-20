#
# FIXME: add comment
#
# Expected Result: just checking if echo works along with other instructions... UART addresses are not valid anymore
#

		addi	r0 = r0, 0;  # first instruction not executed
		addi	r5 = r0, 0;

		addi	r1   = r0 , 2;
		lwl     r10  = [r5 + 0];
		addi	r0 = r0, 0;
                and     r11  = r10 , r1;
		cmpneq  p1 = r1, r11;
	(p1)	bc	0;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;

                lwl     r15  = [r5 + 1];

		addi	r3 = r0, 1;
		lwl     r10  = [r5 + 0];
		addi	r0 = r0, 0;
		and     r11 = r3 , r10;
		cmpneq  p1 = r3, r11;
	(p1)	bc	11;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;

		addi	r13 = r0, 255; # r13 = 255
		swl	[r13 + 1] = r15; # memory address 259 (255 + (1 sl 2)) = whatever from uart 
		nop	0;
		lwl	r25  = [r13 + 1]; # register(25) = whatever from uart 		
		nop 	0;
		swl	[r5 + 1] = r25;
		bc	0;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;
                halt;

