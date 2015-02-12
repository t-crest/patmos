#
# Expected Result: 
#
		.word   180;
		addi	r5 = r0, 0;                
		addi    r1 = r0, 0;
		addi	r10 = r0, 32;
		addi    r16 = r0, 250;
		addi    r16 = r16, 250;
		mts     s6 = r16; # stack cache pointer init
		mts     s5 = r16; 
		addi    r15 = r0, 0; #nop
		addi    r15 = r0, 0; #nop
		addi    r15 = r0, 0; #nop
		addi    r15 = r0, 0; #nop
		addi    r23 = r0, 32; # dont know the addresses so store to the whole cache!
		addi    r13 = r0, 0;
		addi    r14 = r0, 80;
		addi    r16 = r0, 0;
                sres    32;
l1:             sws     [r1 + 0] = r5;
		addi	r1 = r1, 4;
		addi    r5 = r5, 1;
		cmpneq  p2  = r10, r5;
		(p2)	br      l1;
		addi    r0 = r0, 0;
		addi    r0 = r0, 0;
#		sspill  8; # this will cause spill
		addi    r11 = r0, 1;# check if stall works in case of spill
		addi    r12 = r0, 2;# check if stall works in case of spill
		nop;
l2:		sres	8;
		addi    r13 = r13, 1;
		addi    r14 = r14, 1;
		addi    r16 = r16, 4;
		cmpneq  p3 = r13, r23;
		(p3)    br       l2;
		addi    r0 = r0, 0;# branch delay
		addi    r0 = r0, 0;# branch delay
		sfree	8;
		addi	r5 = r0, 0;
		addi	r5 = r0, 0;
		addi	r5 = r0, 0;
		sens    64;
		addi    r11 = r0, 1;# check if stall works in case of spill
		addi    r12 = r0, 2;# check if stall works in case of spill
		halt;
		nop;
		nop;
		nop;
