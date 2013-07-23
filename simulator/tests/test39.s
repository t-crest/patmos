#
# Testing memory delay slots
#
# Expected Result: r1 = 5 & r2 = 8 & r3 = 9  & r4 = 1 & r5 = 5 & r6 = 9 & r7 = 9
#
		addi    r1 = r0, 5;
		addi    r2 = r0, 8;
		addi    r3 = r0, 1;
		swm	[r2 + 0] = r1;
		lwm	r3 = [r2 + 0];
		addi    r4 = r3, 0;
		addi    r5 = r3, 0;
		lwc     r3 = [r2 + 0];
		addi    r3 = r0, 9;
		addi    r6 = r3, 0;
		addi    r7 = r3, 0;
		halt;
