#
# This small test bench showed a former forwarding issue. Is fixed quite some time.
#

		.word   112;
		addi	r0 = r0, 0;  # first instruction not executed

		addi	r1 = r0, 1;
		addi	r2 = r0, 2;
		add	r3 = r1, r2;
		add	r4 = r1, r2;
		add	r5 = r1, r2;
		add	r6 = r1, r2;
		add	r7 = r1, r2;

		addi	r1 = r0, 3;
		addi	r2 = r0, 4;
		add	r3 = r2, r1;
		add	r4 = r2, r1;
		add	r5 = r2, r1;
		add	r6 = r2, r1;
		add	r7 = r2, r1;


		add	r10 = r1, r2;
		add	r11 = r1, r2;
		add	r12 = r10, r11;
		add	r13 = r10, r11;
		add	r14 = r10, r11;
		add	r15 = r10, r11;
		add	r16 = r10, r11;
		add	r17 = r10, r11;

		halt;
		nop;
		nop;
		nop;
