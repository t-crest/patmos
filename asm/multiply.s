#
# Basic multiplication tests
#

	.word   400 # TODO set size
	addi	r1 = r0, 3
	addi	r2 = r0, 2
	mul	r1, r2
	mfs	r8 = s2

	halt
	nop
	nop
	nop
