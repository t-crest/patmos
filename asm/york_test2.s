# No stacks, no nothing, just straightline ASM writing to/from memory
# r15 = read/write pointer
# r16 = counter
# r17 = counter max
# r18 = UART base
# r19 = Temporary for printing
# r20 = Temporary for loading
# r21 = UART spin temporary
# r22 = UART spin compare
    .word 248;

top:    add r15 = r0, 0x1000000;
    addi r16 = r0, 0;
    addi r17 = r0, 2048;
    add r18 = r0, 0xF0000800;
    addi r19 = r0, 90;
    addi r22 = r0, 1;

# Spin to write to memory
ps1:  lwl r21 = [r18 + 0];
    addi r0 = r0, 0;
    and  r21 = r21, r22;
    cmpneq p1 = r21, r22;
    (p1) br ps1;
    nop;
    nop;

# Write to UART
    swl [r18 + 1] = r19;
    
writeloop:
    swm [r15 + 0] = r16;
    addi r16 = r16, 1;    # Increment counter
    addi r15 = r15, 4;    # Increment write address
    cmpneq p1 = r16, r17; # Have we done x writes yet?
    (p1) br writeloop;
    nop;
    nop;    # Do delay slot optimisation later...

writeloop_end:  addi r16 = r0, 0; # Reset the counters...
    add r15 = r0, 0x1000000;

readloop: lwm r20 = [r15 + 0];
    addi r0 = r0, 0;
    cmpneq p1 = r20, r16;
    (p1)   br fail;
    nop;
    nop; # Again...delay slot optimisation later...
    addi r15 = r15, 4; # Increment address
    addi r16 = r16, 1; # Increment compare counter.
    cmpneq p1 = r16, r17;
    (p1) br readloop;
    nop;
    nop;

readloop_end: addi r19 = r0, 75; # K

# Spin to write to UART
ps2:  lwl r21 = [r18 + 0];
    addi r0 = r0, 0;
    and  r21 = r21, r22;
    cmpneq p1 = r21, r22;
    (p1) br ps2;
    nop;
    nop;

# Write to UART
    swl [r18 + 1] = r19;
    br top;
    nop;
    nop;

fail: addi r19 = r0, 70; # F

# Spin to write to UART
ps3:  lwl r21 = [r18 + 0];
    addi r0 = r0, 0;
    and  r21 = r21, r22;
    cmpneq p1 = r21, r22;
    (p1) br ps3;
    nop;
    nop;

# Write to UART
    swl [r18 + 1] = r19;
faildie:    br top;
    nop;
    nop;