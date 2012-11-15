#
# Expected Result: 
#
		addi	r5 = r0, 5;                
#		lwm     r1  = [r31 + 0];
                sres     4;
                sws     [r0 + 0] = r5;
                shs     [r0 + 2] = r5;
                sbs     [r0 + 6] = r5;
                lws     r2  = [r0 + 0];
#                lhs     r3  = [r0 + 0]  ||     lbs     r4  = [r0 + 1];
#               lhus    r4  = [r0 + 0]  ||     lbus    r6  = [r0 + 1];
#                sfree   1;
                halt;
