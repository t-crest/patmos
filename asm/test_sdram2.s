# Test the SDRAM I/O device
#
# Outputs '?' to show it is ready, and waits for character input to start the test.
# During the test the first word in each block is written and read back.
# If error is detected during read out, the 'E' is printed and error count incremented.
# At the end 'OK' is printed or '##' if errors where detected.
# Expected output: WROK
#
# Register conventions:
# r0==0, temp: r1, r2, r3
# r5==uart base
# r6==sdram base
# r10==counts address for memory read/write test
# r11==read/write test bound (address limit)
# r12==memory read error count


	addi	r0 = r0, 0;  # first instruction not executed         	#0
	addi	r5 = r0, 15;                                          	#1
	sli	r5 = r5, 28; # r5==uart base                           	#2
	addi	r6 = r5, 768;# r6==sdram base                         	#3

# wait_start:   # Output '?' and wait for any key press
	addi	r1 = r0, 63; # '?'                                    	#4
	swl     [r5 + 1] = r1;                                     	#5

#poll_stdin:
	lwl     r1 = [r5 + 0];                                     	#6
	addi	r2 = r0, 2;                                           	#7
	and     r1 = r2, r1;                                       	#8
	cmpneq  p1 = r1, r2;                                       	#9
	(p1)	bc	6;   #l:poll_stdin                                 	#10
                addi    r0  = r0 , 0;                       	#11
                addi    r0  = r0 , 0;                       	#12


# SDRAM I/O address space
#    addr(4) selects between special registers/word access, so the offset has to be added to 16
#	constant DMA_OFFSET_ADDR_REG : integer := 0;
#	constant DMA_OFFSET_CMD_STAT : integer := 1;
#
#	constant DMA_CMD_LOAD_LINE  : integer := 0;
#	constant DMA_CMD_STORE_LINE : integer := 1;
#
#	constant DMA_STATUS_READY : integer := 0;
#	constant DMA_STATUS_BUSY  : integer := 1;

#   This test uses only first word in block of 64 bytes
#   1. We write incremented value to first word of each block
#   2. We read and check the values back
#test_init:   
	addi	r10 = r0, 0;	# r10 == addr_cnt                        	#13
	addi	r11 = r0, 1;	# r11 == test_limit                      	#14
	sli	r11 = r11, 20;                                         	#15
	addi	r12 = r0, 0; # r12==error count                       	#16
	addi	r1= r0, 87; 'W'                                       	#17
	swl     [r5 + 1] = r1;                                     	#18

#write:
#	Set the value in the cache line (we don't care if it is loaded as we just use one word)
	addi    r1  = r10, 1;   # val = addr+1 (+1 just for fun, to have slightly different value)	#19
	swl	[r6 + 0] = r1;	# sram.word[0] <= value                 	#20

#	Ask sdram I/O to store the line into memory
	sli	r1 = r10, 6; # sdram controller uses 64 byte aligned addressess	#21
	swl     [r6 + 16] = r1; # sram.addr<=addr_cnt              	#22
	addi    r1  = r0 , 1;                                      	#23
	swl     [r6 + 17] = r1; # sram.cmd<=cmd_store_line         	#24

#poll_sdram_ready:
	lwl     r1  = [r6 + 17]; # sdram.status                    	#25
	addi	r0 = r0, 0;                                           	#26
	cmpneq  p1 = r1, r0;     # ?busy                           	#27
	(p1)	bc	25; #l:poll_sdram_ready                            	#28
                addi    r0  = r0 , 0;                       	#29
                addi    r0  = r0 , 0;                       	#30


#	check the memory range for the test
	addi	r10 = r10, 1;                                         	#31
	cmpneq  p1 = r10, r11;                                     	#32
	(p1)	bc	19; # l:write                                      	#33
                addi    r0  = r0 , 0;                       	#34
                addi    r0  = r0 , 0;                       	#35

#read_init:
	addi	r1= r0, 82; 'R'                                       	#36
	swl     [r5 + 1] = r1;                                     	#37
	addi	r10 = r0, 0;	# addr_cnt <= 0                          	#38
#	 r10 should have the value from before

#read:
#	 Ask sdram I/O to load the line into memory
	sli	r1 = r10, 6; # sdram controller uses 64 byte aligned addressess	#39
	swl     [r6 + 16] = r1; # sram.addr<=addr_cnt              	#40
	swl     [r6 + 17] = r0; # sram.cmd<=cmd_load_line          	#41

#poll_sdram_ready:
	lwl     r1  = [r6 + 17]; # sdram.status                    	#42
	addi	r0 = r0, 0;                                           	#43
	cmpneq  p1 = r1, r0;     # ?busy                           	#44
	(p1)	bc	42; #l:poll_sdram_ready                            	#45
                addi    r0  = r0 , 0;                       	#46
                addi    r0  = r0 , 0;                       	#47

#	 Check the value in the cache line (we only used first word)
	lwl	r2 = [r6 + 0];	# sram.word[0] => value                 	#48
	addi    r1  = r10 , 1;  # val = addr+1 (use the same mod as during write)	#49
	cmpeq  p1 = r1, r2; # should be the same                   	#50
	(p1)	bc  55; #l:+3                                         	#51
        (!p1)   addi    r1  = r0 , 69;  'E'                 	#52
	(!p1)	addi    r12 = r12, 1; #error_cnt++                   	#53
		swl     [r5 + 1] = r1; # we write to UART without pooling for ready here,	#54


#	 check the memory range if we are done
	addi	r10 = r10, 1;                                         	#55
	cmpneq  p1 = r10, r11;                                     	#56
	(p1)	bc	39; #l:read                                        	#57
                addi    r0  = r0 , 0;                       	#58
                addi    r0  = r0 , 0;                       	#59

# read test done, output result and go back to start
# output 'OK' or '##' if err	
#poll_stdout:
	lwl     r1 = [r5 + 0];                                     	#60
	addi	r2 = r0, 1;                                           	#61
	and     r1 = r2, r1;                                       	#62
	cmpneq  p1 = r1, r2;                                       	#63
	(p1)	bc	60;   #l:poll_stdout                               	#64
                addi    r0  = r0 , 0;                       	#65
                addi    r0  = r0 , 0;                       	#66

	cmpneq  p2 = r12, r0; # ?error_cnt != 0                    	#67

#	 79 '0', 35 '#'
		addi	r1 = r0, 79;                                         	#68
	(p2)	addi	r1 = r0, 35;                                     	#69
	swl     [r5 + 1] = r1;                                     	#70

#poll_stdout:
	lwl     r1 = [r5 + 0];                                     	#71
	addi	r2 = r0, 1;                                           	#72
	and     r1 = r2, r1;                                       	#73
	cmpneq  p1 = r1, r2;                                       	#74
	(p1)	bc	71;   #l:poll_stdout                               	#75
                addi    r0  = r0 , 0;                       	#76
                addi    r0  = r0 , 0;                       	#77

#	 75 'K', 35 '#'
		addi	r1 = r0, 75;                                         	#78
	(p2)	addi	r1 = r0, 35;                                     	#79
	swl     [r5 + 1] = r1;                                     	#80

	lwl     r0 = [r5 + 1];  # purge input                      	#90
	bc	1;                                                      	#91
		addi    r0 = r0, 0;                                       	#92
		addi    r0 = r0, 0;                                       	#93
halt;                                                       	#94
