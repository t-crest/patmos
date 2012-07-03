#
# Expected Result: r1 = 0x0283f180 & r2 = 0x0283f180 & r3 = 0xfffff180 & 
#                  r4 = 0xfffffff1 & r5 = 0x0000f180 & r6 = 0x000000f1
#
                lwm     r1  = [r31 + 0];
                sres     4;
                sws     [r0 + 0] = r1;
                shs     [r0 + 2] = r1;
                sbs     [r0 + 6] = r1;
                lws     r2  = [r0 + 0];
                lhs     r3  = [r0 + 0]  ||     lbs     r4  = [r0 + 1];
                lhus    r5  = [r0 + 0]  ||     lbus    r6  = [r0 + 1];
                sfree   1;
                halt;
