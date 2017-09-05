#
# Basic instructions test
#

	.word   56
	addi	r1 = r0, 255
	addi	r1 = r0, 15  # r1 = 15
	subi	r1 = r1, 5   # r1 = 10
	sli	r1 = r1, 1   # r1 = 20
	sri	r1 = r1, 1   # r1 = 10
	srai	r1 = r1, 2   # r1 = 2
	ori 	r1 = r1, 512 # r1 = 514
	andi	r1 = r1, 3   # r1 = 2
	addi    r2 = r0, 24  # r2 = 24
	halt 
	nop
	nop
	nop
