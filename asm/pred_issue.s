
# Try to extract the issue Sahar observed,
# but this works - al variations of r1/r2 lt, eq, gt tried
	.word   48
	addi	r1 = r0, 2
	addi	r2 = r0, 1
	cmple   p4 = r1, r2
	addi    r15 = r0, 1
	addi    r16 = r0, 0
	xor     r16 = r15, r16 # r16 = 1
(!p4)   nor     r16 = r16, r15
	halt
	nop
	nop
	nop
