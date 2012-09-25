#
# Basic instructions test
# different ld/st from/to scratchpad memory
# this is a deprecated version there are other tests on scratchpad

	addi	r1 = r0, 255;  # first instruction not executed
	addi	r1 = r0, 255; # r1 = 255
	addi	r2 = r0, 5;
	addi    r4 = r0, 4;
	swl	[r1 + 1] = r2; # memory address 259 (255 + (1 sl 2)) = 5 
#	lwl	r10  = [r1 + 1]; # register(10) = 5
	addi	r2 = r0, 10;
	shl	[r1 + 3] = r2; # memory address 261 = 10
#	lhl	r11  = [r1 + 3]; # register(11) = 10
	sbl	[r1 + 3] = r4; # memory address 258 = 4
#	lbl
#	lhul
#	lbul	
#	halt; 
