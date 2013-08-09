#
# Expected Result: r1 = 5 & r2 = 10
#

                .word   16;
        (p0)    addi    r1  = r0 , 5;
        (p0)    addi    r2  = r1 , 5;
                halt;
