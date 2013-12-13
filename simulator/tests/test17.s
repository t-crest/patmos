#
# Expected Result: stack exceeded address space exception
#

                .word   44;
		addi    r1 = r0, 0x100;
		mts     s5 = r1;
		mts     s6 = r1;
                sres    4;
		subi	r1  = r31, 8;
                lws     r1  = [r1 + 0];
                halt;
		nop;
		nop;
		nop;
