#
# Basic instructions test
# 

	.word   232;
	addi	r1 = r0, 255;  # first instruction not executed 0
	addi	r1 = r0, 2; #1 r1 = 2
x0:	addi	r2 = r0, 2; #2 r2 = 2
	cmpeq   p1  = r1, r2; #3
	(p1) 	br	x0; #4
	addi	r1 = r0, 1; #5 r1 = 1 
	nop; #5 branch delay
x1:	addi	r3 = r0, 3; #7 r3 = 3
	cmpneq  p2  = r1, r3; #8 
	(p2) 	br	x1; #9 
	addi	r1 = r1, 1; #10 r1 = 2, 3, 4
x3:	nop; #11 branch delay#11
	cmplt	p3 = r3, r1; #12
	(p3) 	br	x3; #13
	subi	r1 = r1, 1; #14 r1 equals 2 after all!
	nop; #15
	addi    r1 = r1 , 5; #16
x4:	cmple   p4 = r3, r1; #17
	(p4) 	br	x4; #18
	subi	r1 = r1, 1; #19 r1 equals 1 after all since <= still holds for r = 3
	nop; #20
	addi 	r10 = r0, 10; #21
x5:	addi 	r11 = r0, 3; #22
	btest   p5 = r10, r11; #23
	(p5) 	br	x5; #24
	addi    r10 = r10, 8;	#25
	nop;	#26

	addi    r12 = r0, 0;	
	addi    r1 = r0, 2;
	addi    r2 = r0, 3;
x6:	cmpult	p6 = r1, r2; #30
	(p6) 	br	x6; #
	addi    r12 = r12, 1;
	addi    r1 = r1, 1;

	addi    r13 = r0, 0;	
	addi    r1 = r0, 2;
	addi    r2 = r0, 4;
x7:	cmpule	p7 = r1, r2; #37
	(p7) 	br	x7; #
	addi    r13 = r13, 1;
	addi    r1 = r1, 1;
	nop;
#signed check
	addi	r1 = r0, 1;  
	sli	r1 = r1, 31;
	ori	r1 = r1, 15;
	addi	r2 = r0, 1;  
	sli	r2 = r2, 31;
	ori     r2 = r2, 9;
	nop;
	nop;
	nop;
x8:	cmplt   p2  = r1, r2; 
	(p2) 	br	x8; 
	addi	r1 = r2, 1; 
	nop;
	addi	r23 = r0, 1;  	

	halt; 


