# This test case tests the delay slots of branches and loads
	addi 	r1 = r0, 8;
	add 	r2 = r0, r0;
	br 		xA;
	br 		xB;
	br 		xC;
xC:	add		r2 = r2, r1;
xB:	add 	r2 = r2, r1;
xA:	add 	r2 = r2, r1;
	br 		xD;
	add 	r2 = r0, r0;
	add 	r2 = r2, r1;
	add 	r2 = r2, r1;
xD: lwc		r4 = [r1+0];
	add 	r5 = r4, r1;
	add 	r5 = r4, r1;
	halt;
