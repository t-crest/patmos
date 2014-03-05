#
# Basic load/store tests

    .word 312;

    addi        r30 = r0, 4; # Method base
    addi        r6 = r0, 15;
    sli         r6 = r6, 28;
    addi        r6 = r6, 2048;  # r6 now points to the UART base

    addi        r1 = r0, 90;    # Print Z
    swl         [r6 + 1] = r1;

    call        delay;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;

    addi        r1 = r0, 89;
    swl         [r6 + 1] = r1;

    call        delay;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;

# Write stuff to RAM (unrolled...I'm lazy...)
    addi        r2 = r0, 8;     # Initial address
    addi        r3 = r0, 67;    # ASCII A

    swl         [r6 + 1] = r3;
    call delay;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;

    swm         [r2 + 0] = r3;  # A
    addi        r2 = r2, 4;     # Increment address
    addi        r3 = r3, 1;     # And character
    swl         [r6 + 1] = r3; 

    call delay;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;

    swm         [r2 + 0] = r3;  # B
    addi        r2 = r2, 4;     # Increment address
    addi        r3 = r3, 1;      # And character
    swl         [r6 + 1] = r3; 

    call delay;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;

    swm         [r2 + 0] = r3;  # C
    addi        r2 = r2, 4;     # Increment address
    addi        r3 = r3, 1;      # And character
    swl         [r6 + 1] = r3; 

    call delay;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;

    swm         [r2 + 0] = r3;  # D
    addi        r2 = r2, 4;     # Increment address
    addi        r3 = r3, 1;      # And character
    swl         [r6 + 1] = r3;

    call delay;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;

# Now load...    
    addi        r2 = r0, 8;

    addi        r0 = r0, 0; # Pipeline hazard?
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;

    lwm         r3 = [r2 + 0];
    addi        r2 = r2, 4;
    swl         [r6 + 1] = r3; # And print it...

    call delay;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;

    lwm         r3 = [r2 + 0];
    addi        r2 = r2, 4;
    swl         [r6 + 1] = r3; # And print it...

    call delay;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;

    lwm         r3 = [r2 + 0];
    addi        r2 = r2, 4;
    swl         [r6 + 1] = r3; # And print it...

    call delay;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;

    lwm         r3 = [r2 + 0];
    addi        r2 = r2, 4; 
    swl         [r6 + 1] = r3; # And print it...

    call delay;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;

    halt;

    addi        r0 = r0, 0;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;

    .word 44;
delay:  addi    r15 = r0, 4000;
    addi        r16 = r0, 0;
delay_top: cmpneq      p1 = r15, r16;
    (p1)        br delay_top;
    addi        r16 = r16, 1;
    addi        r0 = r0, 0;
    ret         r30, r31;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;
    addi        r0 = r0, 0;
