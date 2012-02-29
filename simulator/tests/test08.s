#
# Expected Result: r2 = 0 & r3 = 35 & r4 = 35
#

                addi    r0  = r0 , 5    ||                addi    r1  = r0 , 7;
                mul           r0, r1;
                mfs     r2  = s2;
                mfs     r3  = s2;
                mfs     r4  = s2;
                halt;