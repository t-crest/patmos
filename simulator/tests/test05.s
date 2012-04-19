#
# Expected Result: p1 = 1 & p2 = 0 & r0 = 5 & r1 = 7 & r2 = 12 & r3 = 0
#

                addi    r0  = r0 , 5    ||                addi    r1  = r0 , 7;
                cmplt   p1  = r0, r1    ||                cmpeq   p2  = r0, r1;
           (p2) add     r3  = r0, r1    ||           (p1) add     r2  = r0, r1;
                halt;