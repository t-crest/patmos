#
# Expected Result: r1 = 0x0283f180 & r2 = 0x000000f1 & r3 = 0x0283f180 & r4 = 0xf1800000 & r5 = 0x80000000
#
                lwm     r1  = [r31 + 0];
                lbum    r2  = [r0 + 2];
                swm     [r2 + 0] = r1;
                shm     [r2 + 2] = r1;
                sbm     [r2 + 8] = r1;
                lwm     r3  = [r2 + 0];
                lwm     r4  = [r2 + 1];
                lwm     r5  = [r2 + 2];
                halt;