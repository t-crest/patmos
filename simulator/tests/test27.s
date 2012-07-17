#
# Expected Result: unaligned access
#
                ori     r1  = r1, 1;
                shm     [r1 + 1] = r1;