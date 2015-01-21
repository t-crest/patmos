#
# Expected Result: r1 = 0x00000023
#

# size: 64 bytes | disposable: 0
                .word    64;
                addi     r1  = r0, 0;
                callnd   x;
                addi     r1  = r1, 1;
                callnd   x;
                addi     r1  = r1, 2;
                callnd   y;
                addi     r1  = r1, 3;
                callnd   z;
                addi     r1  = r1, 4;
                addi     r1  = r1, 5;
                addi     r1  = r1, 6;
                halt;
				nop;
				nop;
				nop;

# size: 64 bytes | disposable: 0
                .word    64;
x:              addi     r1  = r0, 0;
                addi     r1  = r1, 1;
                addi     r1  = r1, 2;
				nop;
				ret;

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
