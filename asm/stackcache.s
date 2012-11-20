#
# Just a few basic instructions to watch the pipeline going in ModelSim
#

	addi	r1 = r0, 255;  # first instruction not executed
	sres     4; # do we reserve to store? so there should be the same number of stores after sres?
        sws     [r0 + 0] = r1;
	halt;
