#
# Expected Result: 
#
		addi	r5 = r0, 0;                
		addi    r1 = r0, 0;
#		addi    r1 = r1, 250;
#		addi    r1 = r1, 10; # 8 will be spilled so 10 should suffice
		addi	r10 = r0, 5;
		addi    r16 = r0, 250;
		addi    r16 = r16, 250;
		mts     s6 = r16; # stack cache pointer init
		addi    r15 = r0, 0; #nop
		addi    r15 = r0, 0; #nop
		addi    r15 = r0, 0; #nop
		addi    r15 = r0, 0; #nop
                sres    64;
l1:             sws     [r1 + 0] = r5;
		addi	r1 = r1, 4;
		addi    r5 = r5, 1;
		cmpneq  p2  = r10, r5;
		(p2)	br      l1;
		addi    r0 = r0, 0;
		addi    r0 = r0, 0;
		sres    8; # this will cause spill
		addi    r11 = r0, 1;# check if stall works in case of spill
		addi    r12 = r0, 2;# check if stall works in case of spill
#		addi 	r2 = r0, 0;
#		addi    r6 = r0, 1;
#		sws	[r2 + 0] = r6; # we should store to 
                halt;
