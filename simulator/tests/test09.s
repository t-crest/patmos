#
# Expected Result: r1 = 0283f181 & r2 = 0xfffff181 & r3 = 0xfffffff1 & r4 = 0x0000f181 & r5 = 0x000000f1
#

                .word   28;
                lwm     r1  = [r31 + 1];
                lhm     r2  = [r0 + 3];
                lbm     r3  = [r0 + 6];
                lhum    r4  = [r0 + 3];
                lbum    r5  = [r0 + 6];
                halt;
