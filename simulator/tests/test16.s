#
# Expected Result: stack exceeded address space exception
#
                sres    4;
                lws     r1  = [r31 + 2];
                halt;
