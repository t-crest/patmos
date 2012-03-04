#
# Expected Result: r1 = 0x0283f180 & r2 = 0x0283f180 & r3 = 0xfffff180 & 
#                  r4 = 0xfffffff1 & r5 = 0x0000f180 & r6 = 0x000000f1
#
                lwm     r1  = [r31 + 0];
                sres     12;
                sws     [r0 + 1] = r1;
                shs     [r0 + 3] = r1;
                sbs     [r0 + 7] = r1;
                lws     r2  = [r0 + 1];
                lhm     r3  = [r0 + 1];
                lbm     r4  = [r0 + 2];
                lhum    r5  = [r0 + 1];
                lbum    r6  = [r0 + 2];
                sfree   6;
                halt;
