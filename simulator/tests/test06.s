#
# Expected Result: p1 = 1 & p2 = 0 & p3 = 0 & p4 = 1 & r0 = 5 & r1 = 7 & r2 = 12
#

                addi    r0  = r0 , 5    ||                addi    r1  = r0 , 7;
                cmplt   p1  = r0, r1    ||                cmpeq   p2  = r0, r1;
                pand    p3  = p1, p2    ||                pxor    p4  = p1, p2;
           (p4) add     r2  = r0, r1;
                halt;