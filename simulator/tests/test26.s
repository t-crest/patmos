#
# Expected Result: unaligned access
#

                .word   20;
                ori     r1  = r1, 2;
                lwm     r1  = [r1 + 1];
		nop;
		nop;
		nop;
