#
# Basic instructions test
#

	addi	r1 = r0, 255;  # first instruction not executed
	addi	r1 = r0, 15; # r1 = 15
        mts     s0  = r1;
	addi    r5 = r0, 5; 
	addi	r6 = r0, 6; 
	addi	r7 = r0, 7; 
	mfs	r8 = s0;
	mts	s5 = r7;
	mfs	r10 = s5;
	halt; 
