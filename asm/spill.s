#
# Expected Result: 
#
		addi	r5 = r0, 0;                
		addi    r1 = r0, 0;
		addi	r10 = r0, 10;
                sres    6;
l1:             sws     [r1 + 0] = r5;
		addi	r1 = r1, 1;
		addi    r5 = r5, 1;
		cmpneq  p2  = r10, r5;
		(p2)	br      l1;
		addi    r0 = r0, 0;
		addi    r0 = r0, 0;
		sres    8; # this will cause spill
#		addi 	r2 = r0, 0;
#		addi    r6 = r0, 1;
#		sws	[r2 + 0] = r6; # we should store to 
                halt;
