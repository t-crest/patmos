#
# Expected Result: p1 = 1 & p2 = 0 & r1 = 5 & r2 = 7 & r3 = 12 & r4 = 0
#

                addi    r1  = r0 , 5    ||                addi    r2  = r0 , 7;
                cmplt   p1  = r1, r2    ||                cmpeq   p2  = r1, r2;
           (p2) add     r4  = r1, r2    ||           (p1) add     r3  = r1, r2;
                halt;