#
# Expected Result: stack exceeded address space exception
#

                .word   4;
                sres    4;
                sfree   128;
                halt;
