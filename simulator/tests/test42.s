#
# Lazy pointer
# lp_pulldown == false
#

                .word    160;
		add      r2 = r0, 0x400000;
		add      r3 = r0, 0x500000;
		mts      s5 = r2;
		mts      s6 = r2;
		addi     r30 = r0, 4;
		addi     r15 = r0, 15;
x:              sres     4;
                sws      [r0 + 0] = r2;
		sws      [r0 + 1] = r3;
		addi     r10  = r0 , 10;
loop:		lws      r4 = [r0 + 0];
		lws      r5 = [r0 + 1];
		subi     r10 = r10 , 1;
		call 	 foo;
 		nop;
                nop;
		nop;
		sens     4;
		cmpeq    p1  = r10 , r0;
          (!p1) br       loop;
		nop;
		nop;
		sfree    4;
                halt;
		nop;
		nop;
		nop;
		.word   28; 
foo:		sres     64;
                sws      [r0 + 3] = r2;
		sws      [r0 + 4] = r3;
#		sws      [r0 + 1] = r15; #write to the spilled blocks
		ret     r30, r31;
		nop;
		nop;
		nop;

