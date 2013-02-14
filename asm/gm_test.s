	addi	r1 = r0, 255;  # first instruction not executed
	addi	r20 = r0, 15;
	sli	r20 = r20, 28;
	addi	r1 = r0, 256; # r1 = 256
	addi	r2 = r0, 48;
	addi	r3 = r0, 49;
	swm	[r1+0] = r2;
	swm	[r1+1] = r3;
	nop	0;
	lwm	r4  = [r1+0];
	lwm	r5  = [r1+1];
	nop	0;
	nop	0;

# 		addi	r13 = r0, 1;
# x1:		lwm     r10  = [r20 + 0];
# 		nop 0;
# 		and     r11 = r13 , r10;
# 		cmpneq  p1 = r13, r11;
# 	(p1)	br	x1;
#                 nop 0;
#                 nop 0;
# 
# 		swm	[r20 + 1] = r5;
# 
# 
# 		addi	r13 = r0, 1;
# x2:		lwm     r10  = [r20 + 0];
# 		nop 0;
# 		and     r11 = r13 , r10;
# 		cmpneq  p1 = r13, r11;
# 	(p1)	br	x2;
#                 nop 0;
#                 nop 0;
# 
# 		swm	[r20 + 1] = r5;

	halt; 
