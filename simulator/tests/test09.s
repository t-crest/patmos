#
# Expected Result: r1 = 0x0283f180 & r2 = 0xfffff180 & r3 = 0xfffffff1 & r4 = 0x0000f180 & r5 = 0x000000f1
#

                lwm     r1  = [r31 + 0];
                lhm     r2  = [r0 + 1];
                lbm     r3  = [r0 + 2];
                lhum    r4  = [r0 + 1];
                lbum    r5  = [r0 + 2];
                halt;