#
# Expected Result: ...
#

		addi    r16  = r16 , 64;
		addi	r1   = r0 , 2;
		lwm     r10  = [r5 + 0];
                addi    r0  = r0 , 1;
                and     r11  = r10 , r1;
                and     r11  = r10 , r1;
#		benq
                addi    r0  = r0 , 1;
                lwm     r15  = [r5 + 1];
                lwm     r15  = [r5 + 1];
		addi	r3 = r19 , 1;
		lwm     r10  = [r5 + 0];
		addi    r0  = r0 , 1;
		and     r11 = r3 , r10;
		and     r11 = r3 , r10;
#		benq
		addi    r0  = r0 , 1;
		swm	[r7 + 1] = r15;
		addi    r0  = r0 , 1;
		addi	r3  = r0 , 2;
		addi	r3  = r0 , 2;
#		benq
		addi    r0  = r0 , 1;
                halt;

