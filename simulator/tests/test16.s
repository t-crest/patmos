#
# Expected Result: stack exceeded address space exception
#
# Note that the block stack cache allocates multiple of 
# burst size.
#

                .word   40;
		addi    r1 = r0, 0x100;
		mts     s5 = r1;
		mts     s6 = r1;
                sres    4;
                lws     r1  = [r0 + 5];
                halt;
		nop;
		nop;
		nop;
