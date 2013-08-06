#
# Expected Result: 
#
		.word   48;
		addi	r5 = r0, 5;                
#		lwm     r1  = [r31 + 0];
                sres     4;
                sws     [r0 + 0] = r5;
                shs     [r0 + 2] = r5;
                sbs     [r0 + 6] = r5;
                lws     r2  = [r0 + 0];
		addi    r1 = r0, 0; #load delay 
		add	r7 = r0, r2;
		add	r8 = r0, r2;
		add	r9 = r0, r2;
#                lhs     r3  = [r0 + 0]  ||     lbs     r4  = [r0 + 1];
#               lhus    r4  = [r0 + 0]  ||     lbus    r6  = [r0 + 1];
#                sfree   1;
                halt;
