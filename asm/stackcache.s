#
# Just a few basic instructions to watch the pipeline going in ModelSim
#

	addi	r1 = r0, 255;  # first instruction not executed
	sres     4; # do we reserve to store? so there should be the same number of stores after sres?
        sws     [r0 + 0] = r1;
	addi	r10 = r0, 0;
	addi	r11 = r0, 0;# just to have some instruction to check stall
	or      r11 = r11, r10;#just to have some instruction to check stall
	and     r11 = r10, r11;#just to have some instruction to check stall
	addi    r15 = r0, 1;#just to have some instruction to check stall
	addi    r16 = r0, 0;#just to have some instruction to check stall
	halt;
