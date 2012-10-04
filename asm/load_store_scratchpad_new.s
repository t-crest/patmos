#
# Basic instructions test
# different ld/st from/to scratchpad memory
	addi    r1 = r0, 256;
	sbl     [r1 + 4] = r0;
	sbl     [r1 + 5] = r0;
	sbl     [r1 + 6] = r0;
	sbl	[r1 + 7] = r0;
	lwl     r22 = [r1 + 1];

	addi    r10 = r0, 10;
	shl     [r1 + 1] = r10;
	shl	[r1 + 0] = r10;
	lwl	r11 = [r1 + 1];

	addi    r5 = r0, 3;
	sli     r5 = r5, 7;
	addi    r10 = r0, 251;
	swl     [r1 + 2] = r0;
	shl	[r1 + 4] = r5; # 
	lwl	r21 = [r1 + 2];	# 
	nop	0;
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
