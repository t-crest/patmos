# Test the SDRAM I/O device
#
# Outputs '?' to show it is ready, and waits for character input to start the test.
# During the test the I/O device's cache line is written with 16 characters ('A','B',...,'P')
# The line is read back and printed on the output
# Expected output: ABCDEFGHIJKLMNOP
#
# Register conventions:
# r0==0, temp: r1, r2, r3
# r5==uart base
# r6==sdram base
# r10==counts address for memory read/write test
# r11==read/write test bound (address limit)
# r12==memory read error count


	addi	r0 = r0, 0;  # first instruction not executed         	#0
begin:	addi	r12 = r0, 0; # r12==error count                       	#1
	addi	r5 = r0, 15;                                          	#2
	sli	r5 = r5, 28; # r5==uart base                           	#3
	addi	r6 = r5, 768;# r6==sdram base                         	#4

# wait_start:   # Output '?' and wait for any key press
	addi	r1 = r0, 63; # '?'                                    	#5
	swl     [r5 + 1] = r1;                                     	#6

poll_stdin: lwl     r1 = [r5 + 0];                                     	#7
	addi	r2 = r0, 2;                                           	#8
	and     r1 = r2, r1;                                       	#9
	cmpneq  p1 = r1, r2;                                       	#10
	(p1)	br poll_stdin;                                 	#11
                addi    r0  = r0 , 0;                       	#12
                addi    r0  = r0 , 0;                       	#13

	lwl     r1 = [r5 + 1];                                     	#14

	addi    r21 = r0, 65; 'A'                                  	#15
	addi    r22 = r6, 0;                                       	#16
	addi	r24 = r0, 80; 16 chars from 'A'                       	#17

write_word: swl	[r22 + 0] = r21;                                       	#18
	cmpneq	p1 = r21, r24;                                      	#19
	(p1)	br write_word;                                 	#20
		addi	r21 = r21, 1;                                        	#21
		addi    r22  = r22, 4;                                    	#22


	addi    r22 = r6, 0;                                       	#23
	addi	r23 = r0, 0;                                          	#24
	addi	r24 = r0, 15;                                         	#25
read_word: lwl	r21 = [r22 + 0];                                       	#26

poll_stdout: lwl     r1 = [r5 + 0];                                     	#27
	addi	r2 = r0, 1;                                           	#28
	and     r1 = r2, r1;                                       	#29
	cmpneq  p1 = r1, r2;                                       	#30
	(p1)	br poll_stdout;                               	#31
                addi    r0  = r0 , 0;                       	#32
                addi    r0  = r0 , 0;                       	#33
	swl     [r5 + 1] = r21;                                    	#34

	cmpneq	p1 = r23, r24;                                      	#35
	(p1)	br read_word;                                  	#36
		addi	r23 = r23, 1;                                        	#37
		addi    r22  = r22, 4;                                    	#38

	br begin;


		addi r0 = r0, 0;                                          	#41
		addi r0 = r0, 0;                                          	#42

halt;                                                       	#43
