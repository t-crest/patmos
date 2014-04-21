# This test case  tests the different instructions of the ALU
	.word	128;
	addi	r1 = r0, 10;
	addi	r1 = r0, 10;
	add 	r2 = r1, r0;
	subi	r3 = r1, 5;
	sub 	r4 = r1, r3;
	subi	r4 = r3, 50; # r4 = 50
	sl		r4 = r4, 1; # r4 = 100
	sr		r4 = r4, 2; # r4 = 25
	addi	r5 = r0, 4095;
	sl		r5 = r5, 20;
	sra		r4 = r4, 20; # r4 = 0xffffffff
	or		r4 = r4, 0; # r4 = 0xffffffff
	and		r4 = r4, 0; # r4 = 0x0
	addi	r6 = r0, 1;
# rotates have been removed form then Patmos ISA
#	rl		r6 = r6, r1;
#	rr		r6 = r6, r1; # r6 = 1
	xor		r6 = r6, r1; # r6 = 2
	addi	r7 = r0, 0;
	nor		r6 = r6, r7; # r6 = 4294967293
	shadd	r8 = r1, 1; # r8 = 11
	shadd2	r8 = r8, 1; # r8 = 23
	halt;
	nop;
	nop;
	nop;
