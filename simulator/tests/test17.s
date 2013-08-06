#
# Expected Result: stack exceeded address space exception
#

                .word   5;
                sres    1;
		subi	r1  = r31, 8;
                lws     r1  = [r1 + 0];
                halt;
