#
# Basic instructions test
# different ld/st from/to scratchpad memory
# MS: is SPM accessed with lxc/sxc? I thought it is via lxl/sxl.
# SA: this test case is for data cache with lxc/sxc which is mapped to scratchpad at the moment

	.word   188;
	addi	r1 = r0, 255;  # first instruction not executed
	addi	r1 = r0, 256; # r1 = 256
	addi	r2 = r0, 5;
	addi    r4 = r0, 4;
	addi	r13 = r0, 256;
	addi    r15 = r0, 257;
	swc	[r1 + 1] = r2; # memory address 260 (256 + (1 sl 2)) = 5 (all memory banks are updated)
	lwc	r10  = [r1 + 1]; # register(10) = 5
	nop;
	add	r2 = r2, r10; # r2 = 5 + 5 = 10 

	shc     [r1 + 2] = r0;
	shc	[r1 + 3] = r2; # memory address 262 = 10 (two memory banks are updated)
	lhc	r11  = [r1 + 3]; # register(11) = 10
	nop;

	swc	[r1 + 0] = r0;	
	sbc	[r1 + 0] = r4; # memory address 258 (255 +3 ) = 4 (one memory bank updated)
	lbc	r12 = [r1 + 0]; # r12 = 4;

	shc	[r1 + 2] = r0;
	lhuc    r13 = [r1 + 3]; # r13 = memory address (255 + (3 sl 1)) = 261 (10);
	lbuc	r14 = [r1 + 3]; # r14 = 4; #memory address (255 + 3) = 258 (4) 
addi    r1 = r0 , 256;
	addi    r5 = r0, 3; # r5 = 3
	sli	r5 = r5, 15; # r5(16, 15) = 11
	swc     [r1 + 1] = r5; # memory address 267 = 98304
	lhc	r20 = [r1 + 2];   # r1 + 6 = r1 + 3 to check signed!
	addi    r5 = r0, 1;
	sli	r5 = r5, 7;
	swc     [r1 + 3] = r5; # memory address 267 = 128
	lbc	r19 = [r1 + 12]; # bit (7) of address 267 is 1, so the rest of upper bits are filled with 1

	addi    r5 = r0, 3; # r0 =3
	sli     r5 = r5, 7; # r5(8,7) = 11
	shc	[r1 + 4] = r5; # memory address 263 #25
	nop;
	swc     [r1 + 2] = r0;
	lwc	r21 = [r1 + 2];	# 16 upper bits of r21 are invalid since 16 bits are loaded from memory
	addi    r16 = r0, 15;
	addi    r30 = r0, 31;
	addi    r29 = r0, 15;
	addi    r31 = r0, 1;
x1:	sl	r31 = r31, r30;
	ori     r31 = r31, 384;
	or	r21 = r21, r31;
	subi    r30 = r30, 1;
	cmpneq  p1 = r31, r29;
(p1)	br	x1; #r21 equals to all upper bits 1 
	halt; 
