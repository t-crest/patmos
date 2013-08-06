#
# Expected Result: p1 = 1 & p2 = 0 & p3 = 0 & p4 = 1 & r1 = 5 & r2 = 7 & r3 = 12
#

                .word   36;
                addi    r1  = r0 , 5    ||                addi    r2  = r0 , 7;
                cmplt   p1  = r1, r2    ||                cmpeq   p2  = r1, r2;
                pand    p3  = p1, p2    ||                pxor    p4  = p1, p2;
           (p4) add     r3  = r1, r2;
                halt;
