# Test cache behavior (cache misses and writes to cached lines)

        .word   400
        add     r1  = r0, 0x00800
        add     r2  = r0, 0x10800
        add     r3  = r0, 0x20800

        add     r4  = r0, 0x11111111
        add     r5  = r0, 0x22222222
        add     r6  = r0, 0x33333333
        add     r7  = r0, 0x44444444
        swc     [r1 + 0] = r4
        swc     [r1 + 1] = r5
        swc     [r1 + 2] = r6
        swc     [r1 + 3] = r7
        add     r4  = r0, 0x55555555
        add     r5  = r0, 0x66666666
        add     r6  = r0, 0x77777777
        add     r7  = r0, 0x88888888
        swc     [r2 + 0] = r4
        swc     [r2 + 1] = r5
        swc     [r2 + 2] = r6
        swc     [r2 + 3] = r7
        add     r4  = r0, 0x99999999
        add     r5  = r0, 0xaaaaaaaa
        add     r6  = r0, 0xbbbbbbbb
        add     r7  = r0, 0xcccccccc
        swc     [r3 + 0] = r4
        swc     [r3 + 1] = r5
        swc     [r3 + 2] = r6
        swc     [r3 + 3] = r7

        lwc     r10 = [r1 + 0]
        lwc     r11 = [r1 + 1]
        lwc     r12 = [r1 + 2]
        lwc     r13 = [r1 + 3]
        lwc     r14 = [r2 + 0]
        lwc     r15 = [r2 + 1]
        lwc     r16 = [r2 + 2]
        lwc     r17 = [r2 + 3]
        lwc     r18 = [r3 + 0]
        lwc     r19 = [r3 + 1]
        lwc     r20 = [r3 + 2]
        lwc     r21 = [r3 + 3]

        add     r4  = r0, 0x00112233
        add     r5  = r0, 0x44556677
        add     r6  = r0, 0x8899aabb
        add     r7  = r0, 0xccddeeff
        swc     [r3 + 0] = r4
        swc     [r3 + 1] = r5
        swc     [r3 + 2] = r6
        swc     [r3 + 3] = r7
        add     r4  = r0, 0xffeeddcc
        add     r5  = r0, 0xbbaa9988
        add     r6  = r0, 0x77665544
        add     r7  = r0, 0x33221100
        swc     [r2 + 0] = r4
        swc     [r2 + 1] = r5
        swc     [r2 + 2] = r6
        swc     [r2 + 3] = r7

        lwc     r22 = [r2 + 0]
        lwc     r23 = [r2 + 1]
        lwc     r24 = [r2 + 2]
        lwc     r25 = [r2 + 3]
        lwc     r26 = [r3 + 0]
        lwc     r27 = [r3 + 1]
        lwc     r28 = [r3 + 2]
        lwc     r29 = [r3 + 3]
        
        nop
        halt
        nop
        nop
        nop
