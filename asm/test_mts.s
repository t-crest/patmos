#
# Testing hazard with mts
#

	addi	r0 = r0, 0;  # first instruction not executed
	addi    r1 = r0, 1;
	addi    r2 = r0, 5;
	cmplt   p1 = r2, r1;   # set p1 = true
	addi    r3 = r0, 7;    
	(!p1) addi r3 = r0, 5; # should not be executed
	mts     s0 = r1;       # set p1 = false
	(!p1) addi r4 = r0, 9; # should be executed, sets r4 = 9
	addi	r0 = r0, 0;    # nop
	cmplt   p1 = r3, r2;   # set p1 = true
	addi	r1 = r0, 1;
	(!p1) swc [r2 + 2] = r1; # should not be executed, will cause unaligned access
	mts     s0 = r1;       # sets p1 = false
	halt;		       # r3 should be 7, r4 should be 9
