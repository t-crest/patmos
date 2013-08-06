#
# Expected Result: unaligned access
#

                .word   2;
                ori     r1  = r1, 1;
                shm     [r1 + 1] = r1;
