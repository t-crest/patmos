#
# Basic instructions test
# 

	.word   116;
	addi	r1 = r0, 255;  # first instruction not executed 0
	addi	r1 = r0, 2; #1 r1 = 2
	addi	r2 = r0, 3; #2 r2 = 3
	add     r1  = r1, r2; #3 r1 = 5
	addi    r2 = r0, 1; # r2 = 1
	sub     r1 = r1, r2; # r1 = 4
	sl	r3 = r1, r2; # r3 = 6
	sr	r3 = r3, r2; #r3 = 3
	addi    r10 = r0 , 31;
	addi    r11 = r0, 1;
	sl      r11 = r11, r10;
	addi	r10 = r0, 1;
	sra     r11 = r11, r10; #r11 = 1100...0
	or      r11 = r11, r10;
	and     r11 = r10, r11;
	addi    r15 = r0, 1;
	addi    r16 = r0, 0;
	xor     r16 = r15, r16; # r16 = 1
	nor     r16 = r16, r15; # just the right most bit of r16 is zero the rest are 1
	shadd   r15 = r15, r3; # r15 = 5
	shadd2  r15 = r15, r3; # r15 = 5 shift 2 to left + 3  = 23!
	addi    r1 = r0, 1;
	sli	r1 = r1, 31;
	ori     r1 = r1, 1;
	addi    r2 = r0, 2;
#	rl	r1 = r1, r2;	
	addi    r2 = r2, 1;
	addi    r3 = r0, 1;
#	rr	r2 = r2, r3;
	halt; 


