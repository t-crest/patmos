#
# Expected Result: unaligned access
#
                ori     r1  = r1, 2;
                lwm     r1  = [r1 + 1];