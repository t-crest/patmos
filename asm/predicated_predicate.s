#
# Test predicates and branch
#
	.word   180
	addi	r0 = r0, 0  # first instruction not executed
	addi	r1 = r0, 2
	addi	r2 = r0, 2
	cmpeq   p1 = r1, r2
	pand    p2 = p0 , p1
	addi    r10 = r0, 1
(p2)    pand	p5 = p0, p1
(p5)	addi    r1 = r1, 1

# ALUi instructions
	addi    r3 = r0, 0
	addi    r1 = r0, 5
	addi	r2 = r0, 6
	cmpeq   p1 = r1, r2
(p1)	addi    r3 = r3, 1

	addi    r3 = r0, 3
	addi    r1 = r0, 5
	addi	r2 = r0, 5
	cmpeq   p2 = r1, r2
(p2)	subi    r3 = r3, 1

	addi    r3 = r0, 13
	pand	p2 = p0, p1
(p2)	subi   r3 = r3, 1

	addi    r3 = r0, 4
	por	p3 = p0, p1
(p3)	sli     r3 = r3, 1

	addi    r3 = r0, 3
	addi    r1 = r0, 5
	addi	r2 = r0, 5
	or	r5 = r1, r2 # fw
	cmple   p4 = r1, r2
(p4)	sri     r3 = r3, 1
	add	r15 = r5, r3

	addi    r10 = r0 , 31
	addi    r11 = r0, 1
	sl      r11 = r11, r10
	addi	r10 = r0, 1
	cmpult  p5 = r11, r2
(p5)	srai    r3 = r3, 1
	ori     r9 = r3, 2
	cmplt   p6 = r9, r3
(p6)	add     r15 = r9, r2
	halt
	nop
	nop
	nop
