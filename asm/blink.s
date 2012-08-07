#
# This shall become a blinking LED
#
# Expected Result: LED blinks
# Toggle LED with input from UART.
#

		addi	r0 = r0, 0;  # first instruction not executed

		addi	r10 = r0, 16;
		addi	r9 = r0, 0;
		addi	r8 = r0, 1;

		xor	r9 = r9, r8;
		swm	[r10+0] = r9;  # set the LED on

		addi	r1 = r0, 2048;		
		addi	r5 = r0, 0;

		addi	r1   = r0 , 2;
		lwm     r10  = [r5 + 0];
		addi	r0 = r0, 0;
                and     r11  = r10 , r1;
		cmpneq  p1 = r1, r11;
	(p1)	bc	7;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;

                lwm     r15  = [r5 + 1];

		addi	r3 = r0, 1;
		lwm     r10  = [r5 + 0];
		addi	r0 = r0, 0;
		and     r11 = r3 , r10;
		cmpneq  p1 = r3, r11;
	(p1)	bc	17;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;

		swm	[r5 + 1] = r15;
		bc	0;
                addi    r0  = r0 , 0;
                addi    r0  = r0 , 0;
		halt;
