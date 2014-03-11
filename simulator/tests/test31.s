#
# Expected Result: r2 = 1
#

                .word   x-f
f:              lwc     r1 = [r0 + x];
                nop;
                nop;
                brcfr   r1, r0;
                nop;
                nop;
                halt;
                nop;
                nop;
		nop;
x:              .word   y
y:              add     r2 = r0, 1
                halt
                nop
                nop
		nop
