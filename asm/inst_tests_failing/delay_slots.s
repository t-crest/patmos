# This test case tests the delay slots of branches and loads
	addi 	r1 = r0, 5;
	addi 	r1 = r0, 5;
	addi	r2 = r0, r0;
	br 		x1;
	br 		x2;
	br 		x3;
x3:	add		r2 = r2, r1;
x2:	add 	r2 = r2, r1;
x1:	add 	r2 = r2, r1;
	br 		x4;
	add 	r2 = r0, r0;
	add 	r2 = r2, r1;
	add 	r2 = r2, r1;
x4: lws		r4 = r1, 0;
	add 	r5 = r4, r1;
	add 	r5 = r4, r1;
	halt;
