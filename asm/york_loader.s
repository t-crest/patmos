# This is a very simple loader. It just jumps to 0x8000, where
# the actual code is located
    .word 40
top: addi r30 = r0, 0x0
    add r16 = r0, 0x40004
    nop
    nop
    callr r16
    nop
    nop
    nop
