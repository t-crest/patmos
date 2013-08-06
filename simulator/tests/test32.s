#
# Expected Result: r2 = 1
#

                .word   16;
                nop;
                lwc     r1 = [r0 + 11];
                nop;
                brr     r1;
                nop;
                nop;
                halt;
                nop;
                nop;
                .word   48;
                add     r2 = r0, 1;
                halt;
                nop;
                nop;
