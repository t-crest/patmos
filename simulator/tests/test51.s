#
# Tests that if an instruction's predicate is false, no data hazard error is thrown
# if an operand register of the instruction is being loaded by the previous instruction.
# Expected Result: r1 = 0x00400000 & r2 = 0x003ffffb & r3 = 5
#
                .word   72;
        (p0)    lwc     r1 = [r0 + x];
        (!p0)   add     r1 = r1 , 5;
        (p0)    sub     r2 = r1 , 5;
                add     r3 = r0, 0;
        (!p0)   lwc     r3 = [r0 + x];
        (p0)    add     r3 = r3 , 5;
        (!p0)   sub     r4 = r3 , 5;
                halt;
        nop;
        nop;
        nop;
x:	
        nop;
	