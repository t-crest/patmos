#
# Expected Result: unaligned access
#

                .word   8;
                ori     r1  = r1, 2;
                lwm     r1  = [r1 + 1];
