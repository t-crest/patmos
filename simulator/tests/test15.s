#
# Expected Result: unmapped address space exception
#

                .word   4;
		subi	r1  = r31, 4;
                lwm     r1  = [r1 + 0];
                halt;
