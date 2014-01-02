#
# Expected Result: stack exceeded address space exception
#

                .word   28;
                sres    4;
                sfree   128;
                halt;
		nop;
		nop;
		nop;
