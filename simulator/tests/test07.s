#
# Expected Result: sl = 35 & sh = 0
#

                addi    r0  = r0 , 5    ||                addi    r1  = r0 , 7;
                mul           r0, r1;
                halt;