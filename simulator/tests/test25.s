#
# Expected Result: sl = 0x00000D00, sh = 0x34
#
                .word   24;
                add     r1  = r0, 0x80000020;
                addi    r2  = r0, 0x68;
                mulu    r1, r2;
                halt;
