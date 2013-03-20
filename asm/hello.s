#
# The embedded version of Hello World: a blinking LED
#
# Expected Result: LED blinks
#

	addi	r0 = r0, 0;  # first instruction maybe not executed

        add     r7  = r0, 0xF0000200;
	addi	r7 = r0, 16;
	addi	r8 = r0, 1;

loop:	xor	r9 = r9, r8;  # toggle value
	swl	[r7+0] = r9;  # set the LED

	addi	r1 = r0, 1024;		
	sli	r1 = r1, 10;
addi r1 = r0, 3;

wloop:	subi	r1 = r1, 1;
	cmpneq	p1 = r1, r0;
(p1)	br	wloop;
        addi    r0  = r0 , 0;
        addi    r0  = r0 , 0;
	br	loop;
