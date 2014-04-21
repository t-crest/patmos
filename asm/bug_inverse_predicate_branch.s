# Author: Edgar Lakis
# Bug Report: The reversed condition on the branch doesn't work
# Operation: compares 1 and 0; Outputs '1' on the uart if equal, '0' if not equal
# Expected Result: '0'
# Current output: '1'

	.word	84;
x0:		addi	r0 = r0, 0;  # first instruction not executed
		addi	r5 = r0, 15;
		sli	r5 = r5, 28;

	  addi r1 = r0, 1;
          cmpeq p1 = r1, r0;
    (!p1) br not_equal;		# BUG here! this doesn't work
	  addi r0 = r0, 0;
	  addi r0 = r0, 0;

equal:    addi r1 = r0, 49;  # '1'
          swm [r5 + 1] = r1;
	  br done;
	  addi r0 = r0, 0;
	  addi r0 = r0, 0;

not_equal:   addi r1 = r0, 40; # '0'
          swm [r5 + 1] = r1;

# wait forever
done:     br 0;
          add r0 = r0, 0;
          add r0 = r0, 0;
