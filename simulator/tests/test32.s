#
# Expected Result: r2 = 1
#

                nop;
                lwc     r1 = [r0 + 10];
                nop;
                brr     r1;
                nop;
                nop;
                halt;
                nop;
                nop;
                .word   44;
                add     r2 = r0, 1;
                halt;
                nop;
                nop;
