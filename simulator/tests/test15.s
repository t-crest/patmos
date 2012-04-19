#
# Expected Result: unmapped address space exception
#
                lwm     r1  = [r31 - 1];
                halt;
