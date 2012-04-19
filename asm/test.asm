label:
	p1 br lab1
	addi r1, 10 -> r13 // hhj
	p1 addi r1, 10 -> r13 // hhj
	sub r2, r3 -> r2
	p5 sub r2, r3 -> r31
	br label
lab1:
	br lab1
	p1 cmp r1 != r2 -> p3


// That's not Patmos, but Leros assembler....
// Register definitions


