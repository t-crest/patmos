#
# Basic instructions test
# different ld/st from/to scratchpad memory

	addi	r1 = r0, 255;  # first instruction not executed
	addi	r1 = r0, 255; # r1 = 255
	addi	r2 = r0, 5;
	addi    r4 = r0, 4;
	swl	[r1 + 1] = r2; # memory address 259 (255 + (1 sl 2)) = 5 (all memory banks are updated)
	lwl	r10  = [r1 + 1]; # register(10) = 5
	nop 	0;
	add	r2 = r2, r10; # r2 = 15 

	shl	[r1 + 3] = r2; # memory address 261 = 10 (two memory banks are updated)
	lhl	r11  = [r1 + 3]; # register(11) = 10
	nop	0;

	sbl	[r1 + 3] = r4; # memory address 258 (255 +3 ) = 4 (one memory bank updated)
	lbl	r12 = [r1 + 3]; # r12 = 4;

	lhul    r13 = [r1 + 3]; # r13 = memory address (255 + (3 sl 1)) = 261 (10);
	lbul	r14 = [r1 + 3]; # r14 = 4;

	addi    r5 = r0, 3;
	sli	r5 = r5, 15;
	swl     [r1 + 3] = r5; # memory address 267 = 98304
	lhl	r20 = [r1 + 6];   # r1 + 6 = r1 + 3 to check signed!
	addi    r5 = r0, 1;
	sli	r5 = r5, 7;
	swl     [r1 + 3] = r5; # memory address 267 =
	lbl	r19 = [r1 + 12];

	addi    r5 = r0, 3;
	sli     r5 = r5, 7;
	shl	[r1 + 4] = r5; # memory address 263 
	lwl	r21 = [r1 + 2];	# 16 upper bits of r21 are invalid since 16 bits are loaded from memory
	addi    r16 = r0, 15;
	addi    r30 = r0, 31;
	addi    r29 = r0, 15;
	addi    r31 = r0, 1;
	sl	r31 = r31, r30;
	ori     r31 = r31, 384;
	or	r21 = r21, r31;
	subi    r30 = r30, 1;
	cmpneq  p1 = r31, r29;
(p1)	bc	25; #r20 equals to all upper bits 1 
	halt; 
