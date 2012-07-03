#
# Expected Result: stack exceeded address space exception
#
                sres    1;
                lws     r1  = [r31 + 2];
                halt;
