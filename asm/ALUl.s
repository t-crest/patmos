#
# Basic instructions test
# long immediate instructions

	.word   168;
	addi	r1 = r0, 255;  # first instruction not executed 0
	addi	r1 = r0, 2; # r1 = 2
	add     r1  = r1, 65536; # r1 = 65538
	add     r2 = r0, 65536; # r2 = 65536
	sub     r1 = r1, 65500; # r1 = 38
	addi    r3 = r0, 1;
	sl	r3 = r3, 1; # 
	sli	r3 = r3, 30;
	sr	r3 = r3, 46; # just 5 bits are used so r3 is shifted to right 14 times
	add     r5 = r0, 0;
	add     r6 = r0, 0;
	add	r5 = r5, 1048576;
	add     r6 = r6, 1073741824;
	or      r5 = r5, 1048575; # r5 = 2097151
	and     r5 = r6, 1073741824;
	xor     r5 = r5, 1048576; # two bits are 1...
	nor	r5 = r5, 1074790400; # two bits are not 1
	addi    r5 = r0, 1;
	shadd   r5 = r5, 1048576;
	addi    r5 = r0, 1;
	shadd2	 r5 = r5, 1048576;
	add	r10 = r0, 1048576;
#	rl	r10 = r10, 13; # r10(1) = 1
#	rr	r10 = r10, 2; # r10(31) = 1
	sra	r10 = r10, 5; # fills in 5 upper bits with 1
	halt; 


