#
# Expected Result: stack exceeded address space exception
#

                .word   16;
                sres    1;
                lws     r1  = [r31 + 2];
                halt;
