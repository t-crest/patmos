#
# Basic instructions test
#

	.word   44;
	addi	r1 = r0, 255;  # first instruction not executed
	addi	r1 = r0, 15; # r1 = 15
	subi	r1 = r1, 5; # r1 = 10
	sli	r1 = r1, 1;# r1 = 30
	sri	r1 = r1, 1;# r1 =15
	srai	r1 = r1, 2; # r1 = 3
	ori 	r1 = r1, 512; # r1 = 515
	andi	r1 = r1, 3; # r1 = 3
	addi    r2 = r0, 24;# init r2
	halt; 
