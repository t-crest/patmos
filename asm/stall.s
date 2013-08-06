#
# Basic instructions test
#

	.word   24;
	addi	r1 = r0, 255;  # first instruction not executed
	waitm;
	addi	r1 = r1, 5;
	addi	r1 = r1, 10;
	halt; 
