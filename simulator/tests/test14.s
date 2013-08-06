#
# Expected Result: r1 = 0x0283f181 & r2 = 0x0283f181 & r3 = 0x00000283 & 
#                  r4 = 0xffffff83 & r5 = 0x00000283 & r6 = 0x00000083
#
                .word   13;
                lwm     r1  = [r31 + 1];
                sres     4;
                sws     [r0 + 0] = r1;
                shs     [r0 + 2] = r1;
                sbs     [r0 + 6] = r1;
                lws     r2  = [r0 + 0];
                lhs     r3  = [r0 + 0]  ||     lbs     r4  = [r0 + 1];
                lhus    r5  = [r0 + 0]  ||     lbus    r6  = [r0 + 1];
                sfree   1;
                halt;
