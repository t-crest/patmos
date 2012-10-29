#
# Test branch
#
	addi	r0 = r0, 0;  # first instruction not executed
	addi	r1 = r0, 1;
	addi	r1 = r0, 2;
	bc	l1;
	addi	r1 = r0, 3;
	addi	r1 = r0, 4;
	addi	r1 = r0, 100;
	addi	r1 = r0, 101;
	addi	r1 = r0, 102;

l2:	addi	r1 = r0, 9;
	addi	r1 = r0, 10;
	br	end;
	addi	r1 = r0, 11; 
	addi	r1 = r0, 12;
	addi	r1 = r0, 104;

l1:	addi	r1 = r0, 5;
	addi	r1 = r0, 6;
	br	l2;
	addi	r1 = r0, 7;
	addi	r1 = r0, 8;
	addi	r1 = r0, 103;

end:	addi	r1 = r0, 13;
	addi	r1 = r0, 14;
	addi	r1 = r0, 15;

	halt;
