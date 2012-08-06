#
# Just a few basic instructions to watch the pipeline going in ModelSim
#

	addi	r1 = r0, 255;  # first instruction not executed
	addi	r1 = r0, 15;
	subi	r1 = r1, 5;
	rsubi	r1 = r1, 25;
	sl	r1 = r1, 7;
	halt; 
