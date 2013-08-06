#
# Expected Result: stack exceeded address space exception
#

                .word   4;
                sres    1;
                lws     r1  = [r31 + 2];
                halt;
