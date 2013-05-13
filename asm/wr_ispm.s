#
# Test write of instruction scratchpad (ISPM)
#
# ISPM is currently mapped to 0x10000000
#
# Author: Martin Schoeberl (martin@jopdesign.com)
#

# Target code:
#	addi	r10 = r0, 0x123;
#	addi	r11 = r0, 0xabc;
#140123
#160abc

	addi	r0 = r0, 0;
	add	r1 = r0, 0x10000000;
	add	r2 = r0, 0x140123;
	swl	[r1 + 0] = r2;
	add	r2 = r0, 0x160abc;
	swl	[r1 + 1] = r2;
	add	r2 = r0, 0xabcd1234;
	swl	[r1 + 2] = r2;

	halt; 
