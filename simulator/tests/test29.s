#
# Expected Result: r1 = 9 & PRR = 00000001
#

                .word   5;
                cmpeq   p3  = r0 , r0;
                mfs     r1  = s0;
                mts     s0  = r0;
                halt;
