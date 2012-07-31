#
# This is a simple echo program on the UART
#
# Expected Result: echo entered characters
#

	addi	r0 = r0, 0;  # first instruction not executed

		addi    r16  = r16 , 64;
		addi	r1   = r0 , 2;
		lwm     r10  = [r5 + 0];
                addi    r0  = r0 , 1;
                and     r11  = r10 , r1;
		bne     r1 != r11 , 4;
                addi    r0  = r0 , 1;
                lwm     r15  = [r5 + 1];
                lwm     r15  = [r5 + 1];
		addi	r3 = r19 , 1;
		lwm     r10  = [r5 + 0];
		addi    r0  = r0 , 1;
		and     r11 = r3 , r10;
		bne     r1 != r11 , 4;
		addi    r0  = r0 , 1;
		swm	[r7 + 1] = r15;
		addi    r0  = r0 , 1;
		addi	r3  = r0 , 2;
		addi	r3  = r0 , 2;
		addi    r0  = r0 , 1;
		andi    r3 = r3 , 0;
		bne	r16 != r0 , 20;
                halt;

