#
# Basic instructions test
#

	.word   56;
	addi	r1 = r0, 255;  # first instruction not executed
	addi	r1 = r0, 15; # r1 = 15
        mts     s6  = r1;
	addi    r5 = r0, 5; 
	addi	r6 = r0, 6; 
	addi	r7 = r0, 7; 
	mfs	r8 = s6;
	mts	s6 = r7;
	mfs	r10 = s6;
	halt; 
	nop;
	nop;
	nop;
