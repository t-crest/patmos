#
# Basic instructions test
#

	addi	r1 = r0, 255;  # first instruction not executed
	addi	r1 = r0, 1; #
	sli	r1 = r1, 31;
	abs	r1 = r1;
	addi	r2 = r0, 1;
	sli     r2 = r2, 7;
	sext8   r2 = r2;
	addi	r3 = r0, 1;
	sli     r3 = r3, 15;
	sext16   r3 = r3;
	addi	r4 = r0, 1;
	sli     r4 = r4, 15;
	zext16   r4 = r4;
	halt; 
