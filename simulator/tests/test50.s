#
# Expected Result: 
#

                .word   240;
# initialize the stack cache registers: stack top and mem top
                addi    r1  = r0, 0x900;
                mts     s5 = r1;
                mts     s6 = r1;
# setup some stack frame
                sres    10;
                nop;
                nop;
                addi    r1 = r0, 1;
                sws     [r0 + 0] = r1 || addi    r1 = r1, 1;
                sws     [r0 + 2] = r1 || addi    r1 = r1, 1;
                sws     [r0 + 3] = r1 || addi    r1 = r1, 1;
                sws     [r0 + 4] = r1 || addi    r1 = r1, 1;
                sws     [r0 + 5] = r1 || addi    r1 = r1, 1;
                sws     [r0 + 6] = r1 || addi    r1 = r1, 1;
                sws     [r0 + 7] = r1 || addi    r1 = r1, 1;
                sws     [r0 + 8] = r1 || addi    r1 = r1, 1;
                sws     [r0 + 9] = r1 || addi    r1 = r1, 1;
                nop;
                nop;
                nop;
# spill all stack cache content and free all space again
                sres    256; # no spilling here
                nop;
                nop;
                nop;
                sres    256; # spill 12 words here, 2 more than usual
                nop;
                nop;
                nop;
                sfree   256; # no filling here
                nop;
                nop;
                nop;
                sfree   256; # fill 4 words here
                nop;
                nop;
                nop;
# reload the stack frame -- fill another 8 words here
                sens    10;
                nop;
                nop;
                nop;
                lws     r1 = [r0 + 7];
                halt;
                nop;
                nop;
                nop;
