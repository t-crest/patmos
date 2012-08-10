#
# Basic instructions test
# 

	addi	r1 = r0, 255;  # first instruction not executed 0
	addi	r1 = r0, 2; #1 r1 = 2
	addi	r2 = r0, 2; #2 r2 = 2
	cmpeq   p1  = r1, r2; #3
	(p1) 	bc	2; #4
	addi	r1 = r0, 1; #5 r1 = 1 
	nop	0; #5 branch delay
	addi	r3 = r0, 3; #7 r3 = 3
	cmpneq  p2  = r1, r3; #8 
	(p2) 	bc	7; #9 
	addi	r1 = r1, 1; #10 r1 = 2, 3, 4
	nop	0; #11 branch delay#11
	cmplt	p3 = r3, r1; #12
	(p3) 	bc	11; #13
	subi	r1 = r1, 1; #14 r1 equals 2 after all!
	nop	0; #15
	addi    r1 = r1 , 5; #16
	cmple   p4 = r3, r1; #17
	(p4) 	bc	17; #18
	subi	r1 = r1, 1; #19 r1 equals 1 after all since <= still holds for r = 3
	nop     0; #20
	addi 	r10 = r0, 10; #21
	addi 	r11 = r0, 3; #22
	btest   p5 = r10, r11; #23
	(p5) 	bc	22; #24
	addi    r10 = r10, 8;	#25
	nop 	0;	#26
#	halt; 


