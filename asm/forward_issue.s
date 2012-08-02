#
# This small test bench shows the forwarding issue within the register file
#

		addi	r0 = r0, 0;  # first instruction not executed

		addi	r1 = r0, 1;
		addi	r2 = r0, 2;
		add	r3 = r1, r2;
		add	r4 = r1, r2;
		add	r5 = r1, r2;
		add	r6 = r1, r2;
		add	r7 = r1, r2;
		add	r10 = r1, r2;
		add	r11 = r1, r2;
		add	r12 = r1, r2;
