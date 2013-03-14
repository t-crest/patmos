#
# Test predicates and branch
#
	addi	r0 = r0, 0;  # first instruction not executed
	addi	r1 = r0, 2;
	addi	r2 = r0, 2;
	cmpeq   p1 = r1, r2;
	pand    p2 = p0 , p1;
x1:	addi    r10 = r0, 1; # this is just to jump!
(p2)    pand	p5 = p0, p1;
(p5)	addi    r1 = r1, 1;

# ALUi instructions
	addi    r3 = r0, 0;
	addi    r1 = r0, 5;
	addi	r2 = r0, 6;
	cmpeq   p1 = r1, r2;
(p1)	addi    r3 = r3, 1;

	addi    r3 = r0, 3;
	addi    r1 = r0, 5;
	addi	r2 = r0, 5;
	cmpeq   p2 = r1, r2;
(p2)	subi    r3 = r3, 1;

	addi    r3 = r0, 13;
	pand	p2 = p0, p1;
(p2)	rsubi   r3 = r3, 1;

	addi    r3 = r0, 4;
	por	p3 = p0, p1;
(p3)	sli     r3 = r3, 1;

	addi    r3 = r0, 3;
	addi    r1 = r0, 5;
	addi	r2 = r0, 5;
	or	r5 = r1, r2; # fw
	cmple   p4 = r1, r2;
(p4)	sri     r3 = r3, 1;
	add	r15 = r5, r3;

	addi    r10 = r0 , 31;
	addi    r11 = r0, 1;
	sl      r11 = r11, r10;
	addi	r10 = r0, 1;
	cmpult  p5 = r11, r2;
(p5)	srai    r3 = r3, 1;
	ori     r9 = r3, 2;
	cmplt   p6 = r9, r3;
(p6)	add     r15 = r9, r2;

# ALU instructions
	addi    r3 = r0, 3;
	addi    r1 = r0, 5;
	addi	r2 = r1, 5;
	or	r5 = r1, r2; # fw
	por     p7 = p6, p5;
(p7)	sr      r3 = r3, r5;
	and	r15 = r5, r3;

	addi    r3 = r0, 3;
	or      r12 = r11, r3;
	btest	p2 = r12, r15;
	pxor    p1 = p6, p7;
	pand    p3 = p1, p2;
	addi	r2 = r1, 5;
	or	r5 = r1, r2; 
	por     p7 = p6, p5;
(p2)	sl      r3 = r3, r5;
	and	r15 = r5, r3;

	addi    r8 = r0, 3;
	and     r7 = r8, r5;
	or      r12 = r11, r3;
	cmpult	p3 = r12, r15;
	addi	r2 = r1, 5;
	or	r5 = r1, r2; 
	por     p7 = p6, p5;
(p3)	or      r3 = r8, 7;

	addi    r20 = r0, 1;
	addi    r2 = r0, 0;
	addi    r21 = r0, 20;
x0:	add     r20 = r20, r2; 
	cmpeq   p1  = r20, r21;
(p1) 	br	x0;
	addi    r1 = r0, 0;	
	addi    r1 = r0, 0;
(!p1)	xor     r3 = r2, r15;

	addi	r10 = r0, 0;	
	swl	[r10+0] = r11;
	lwl	r2 = [r10+0];
	cmple   p4 = r1, r2;
	addi    r15 = r0, 1;
	addi    r16 = r0, 0;
	xor     r16 = r15, r16; # r16 = 1
(!p4)   nor     r16 = r16, r15;

	pand    p1 = p0, p2;
	cmple   p4 = r1, r2;
	addi    r15 = r0, 1;
	addi    r16 = r0, 0;
	xor     r16 = r15, r16; # r16 = 1
(!p2)   sub     r16 = r16, r15;

(p0)    rsub     r16 = r16, r15;
	swl	[r10+0] = r16;
	lwl	r12 = [r10+0];
(p1)	sra	r25 = r12, r16;
(p2)	shadd   r2 = r2, r11;
(!p3)   shadd2  r13 = r2, r12;

# ALUl instructions
	addi	r1 = r0, 2; # r1 = 2
(p1)	add     r1  = r1, 65536; # r1 = 65538
	add     r2 = r0, 65536; # r2 = 65536
(p2)	sub     r1 = r1, 65500; # r1 = 38
(!p1)	rsub    r1 = r2, 66000; # r1 = 464
	addi    r3 = r0, 1;
	sl	r3 = r3, 1; # 
	sli	r3 = r3, 30;
	sr	r3 = r3, 46; # just 5 bits are used so r3 is shifted to right 14 times
	add     r5 = r0, 0;
	add     r6 = r0, 0;
	add	r5 = r5, 1048576;
	add     r6 = r6, 1073741824;
(p3)	or      r5 = r5, 1048575; # r5 = 2097151
(!p2)	and     r5 = r6, 1073741824;
(!p3)	xor     r5 = r5, 1048576; # two bits are 1...
(p5)	nor	r5 = r5, 1074790400; # two bits are not 1
	addi    r5 = r0, 1;
(!p5)	shadd   r5 = r5, 1048576;
	addi    r5 = r0, 1;
(p6)	shadd2	 r5 = r5, 1048576;
	add	r10 = r0, 1048576;
	rl	r10 = r10, 13; # r10(1) = 1
	rr	r10 = r10, 2; # r10(31) = 1
	sra	r10 = r10, 5; # fills in 5 upper bits with 1
# unary instructions

(p1)	abs	r1 = r1;
(!p2)	addi	r2 = r0, 1;
(p3)	sli     r2 = r2, 7;
(!p3)	sext8   r2 = r2;
(p4)	addi	r3 = r0, 1;
(p5)	sli     r3 = r3, 15;
(!p4)	sext16   r3 = r3;
(p5)	addi	r4 = r0, 1;
(p6)	sli     r4 = r4, 15;
(p7)	zext16   r4 = r4;
	halt;
