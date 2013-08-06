#
# Expected Result: r1 = 0x0283f181 & r2 = 0x000000f0 & r3 = 0x0283f181 & r4 = 0xf1810000 & r5 = 0x81000000
#

                .word   12;
                lwm     r1  = [r31 + 1];
                lbum    r2  = [r0 + 6];
		nop;
                andi    r2  = r2, 0xf0;
                swm     [r2 + 0] = r1;
                shm     [r2 + 2] = r1;
                sbm     [r2 + 8] = r1;
                lwm     r3  = [r2 + 0];
                lwm     r4  = [r2 + 1];
                lwm     r5  = [r2 + 2];
                halt;
