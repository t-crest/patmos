#
# MS: what does this test case test?
# SA: it tests forwarding along with dual issue (as the name suggests)
#
# Expected Result: 
#
		.word   140;
		addi	r0 = r0, 0;  # first instruction not executed
		addi	r1 = r0, 1;
		add	r2   = r0 , 65536; 
		add     r3 = r0, 3;
		add     r5 = r0, 100000;
		addi    r4 = r0, 4;
		add     r6 = r0, 200000;
		add     r7 = r0, 300000;

#		addi	r1 = r0, 1;
#		addi	r2 = r0, 2;
		add	r3 = r1, r2; #r3 = 65537
		add	r4 = r1, r2; #r4 = 65537
		add	r5 = r1, r2;
		add	r6 = r1, r2;
		add	r7 = r1, r2;

		addi	r1 = r6, 3; #r1 = 65540
		addi	r2 = r7, 4; #r2 = 65541
		add	r3 = r2, r1; #r3 = 131081
		add	r4 = r2, r1;
		add	r5 = r2, r1;
		add	r6 = r2, r1;
		add	r7 = r2, r1;
		add	r10 = r1, r2;
		add	r11 = r1, r2;

		add	r12 = r10, r11; #r12 = 262162
		add	r13 = r10, r11;
		add	r14 = r10, r11;
		add	r15 = r10, r11;
		add	r16 = r10, r11;
		add	r17 = r10, r11;
                halt;
