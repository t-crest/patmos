#
# Minimum program to test the new assembler
#

	.word   48;
	addi	r1 = r0, 255;  # first instruction not executed

label2:	addi	r1 = r0, 15;

	addi	r2 = r0, 4;
	addi	r3 = r0, 3;
	add	r4 = r2, r3;
	nor 	r6 = r1, r4		||	and 	r7 = r1, r5;
	halt;
	nop;
	nop;
	nop;
