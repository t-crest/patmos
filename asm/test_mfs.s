#
# Testing hazard with mfs
#

	.word   84
	addi	r0 = r0, 0  # first instruction not executed
	addi    r1 = r0, 4
	addi    r2 = r0, 6
	cmplt   p1 = r1, r2 # set p1 = true
	mfs     r2 = s0     # read s0, r2 should be 3
	addi    r3 = r2, 0  # r3 should be 3
	addi    r4 = r0, 16 # test mfs on split-load result
	swc     [r4 + 0] = r3
#	dlwc    [r4 + 0]
#	wait
#	mfs     r4 = s1     # should set r4 = r3 = 3
#	addi    r9 = r4, 0  # should set r9 = 3
	addi    r5 = r0, 2  # multiply 2*4
	addi    r6 = r0, 4
	mul     r5, r6
	addi    r8 = r0, 0  #  clear r8
	addi    r0 = r0, 0
	addi    r0 = r0, 0
	mfs     r7 = s2     # r7 should be 8
	addi    r8 = r7, 0  # r8 should be 8
	halt		      
	nop
	nop
	nop
