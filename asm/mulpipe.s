#
# Test the multiplication pipeline
#

	.word   96
	addi	r1 = r0, 1
	addi	r2 = r0, 2
	addi	r3 = r0, 3
	addi	r4 = r0, 4
	addi	r5 = r0, 5
	addi	r6 = r0, 6
	addi	r7 = r0, 7

	mul	r1, r2 || mfs r8 = s2
	mul r1, r3 || mfs r9 = s2
	mul r1, r4 || mfs r10 = s2
	mul r1, r5 || mfs r11 = s2
	mul r1, r6 || mfs r12 = s2
	mul r1, r7 || mfs r13 = s2

	halt
	nop
	nop
	nop
