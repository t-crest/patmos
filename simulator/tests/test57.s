#
# Expected Result: r1 = 0x0000001e
#

# size: 64 bytes | disposable: 0
                .word    64;
                addi     r1  = r0, 0;
                callnd   y;
                addi     r1  = r1, 1;
                callnd   y;
                addi     r1  = r1, 2;
                callnd   z;
                addi     r1  = r1, 3;
                addi     r1  = r1, 4;
                addi     r1  = r1, 5;
                halt;
				nop;
				nop;
				nop;

# size: 32 bytes | disposable: 1
                .word    65568;
y:              addi     r1  = r1, 1;
                addi     r1  = r1, 2;
                addi     r1  = r1, 3;
                ret;
				nop;
				nop;
				nop;

# size: 64 bytes | disposable: 1
                .word    65600;
z:              addi     r1  = r1, 1;
				addi     r1  = r1, 2;
				nop;
				ret;
