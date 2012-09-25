# Test the SDRAM I/O device
#
# Outputs '?' to show it is ready, and waits for character input to start the test.
# During the test each memory word (in test range) is written and checked on read back.
# First all words are written, in to the cache_line and stored to memory on the block boundaries.
# Next all words are read_back, by loading the chache_line on the block boundaries.
# If error is detected during read out, the 'E' is printed and error count incremented.
# At the end 'OK' is printed or '##' if errors where detected.
# Expected output: 'WROK'
#
# Register conventions:
# r0==0, temp: r1, r2, r3
# r5==uart base
# r6==sdram base
# r9== not(63): mask for address block/offset separation
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
#    addr(5) selects between special registers/word access, so the offset has to be added to 32
#	constant DMA_OFFSET_ADDR_REG : integer := 0;
#	constant DMA_OFFSET_CMD_STAT : integer := 1;
#
#	constant DMA_CMD_LOAD_LINE  : integer := 0;
#	constant DMA_CMD_STORE_LINE : integer := 1;
#
#	constant DMA_STATUS_READY : integer := 0;
#	constant DMA_STATUS_BUSY  : integer := 1;

#   This test writes all words in block of 64 bytes
#   1. We write incremented value to first word of each block
#   2. We read and check the values back
#test_init:   
	addi	r10 = r0, 0;	# r10 == addr_cnt                        	#13
	addi	r11 = r0, 1;	# r11 == test_limit                      	#14
	sli	r11 = r11, 26;                                          	#15
	addi	r12 = r0, 0; # r12==error count                       	#16
	addi	r1= r0, 87; 'W'                                       	#17
	swl     [r5 + 1] = r1;                                     	#18
	subi	r9 = r0, 64;	# r9 == mask (not 63)                    	#19

#write_word:
#	Set the value in the cache line 
	addi    r1  = r10, 1;   # val = addr+1 (+1 just for fun, to have slightly different value)	#20
	andi	r2  = r10, 63;  # offset                              	#21
	add	r2  = r6, r2;                                          	#22
	swl	[r2 + 0] = r1;	# sram.word[offset] <= value            	#23

# check if next word starts the block. Write the block in such case
	addi	r1 = r10, 4;                                          	#24
	andi	r1 = r1, 63;                                          	#25
	cmpneq	p1 = r1, r0;                                        	#26

	(p1)	bc 40; #l:skip_store                           	#27
                addi    r0  = r0 , 0;                       	#28
                addi    r0  = r0 , 0;                       	#29

#	Ask sdram I/O to store the line into memory
	and	r1 = r10, r9; # sdram controller uses 64 byte aligned addressess	#30
	swl     [r6 + 32] = r1; # sram.addr<=block_addr            	#31
	addi    r1  = r0 , 1;                                      	#32
	swl     [r6 + 33] = r1; # sram.cmd<=cmd_store_line         	#33

#poll_sdram_ready:
	lwl     r1  = [r6 + 33]; # sdram.status                    	#34
	addi	r0 = r0, 0;                                           	#35
	cmpneq  p1 = r1, r0;     # ?busy                           	#36
	(p1)	bc	34; #l:poll_sdram_ready                            	#37
                addi    r0  = r0 , 0;                       	#38
                addi    r0  = r0 , 0;                       	#39

#skip_store:
#	check the memory range for the test and advance to next word
	addi	r10 = r10, 4;                                         	#40
	cmplt  p1 = r10, r11;                                     	#41
	(p1)	bc	20; # l:write_word                                 	#42
                addi    r0  = r0 , 0;                       	#43
                addi    r0  = r0 , 0;                       	#44


#read_init:
	addi	r1= r0, 82; 'R'                                       	#45
	swl     [r5 + 1] = r1;                                     	#46
	addi	r10 = r0, 0;	# addr_cnt <= 0                          	#47

#read_word:
# check if word starts the block. Load the block in such case
	andi	r1 = r10, 63;                                         	#48
	cmpneq	p1 = r1, r0;                                        	#49

	(p1)	bc 61; #l:skip_load                            	#50
                addi    r0  = r0 , 0;                       	#51
                addi    r0  = r0 , 0;                       	#52

#	 Ask sdram I/O to load the block into the cache line
# r10 is aligned at block start already
	swl     [r6 + 32] = r10; # sram.addr<=block_addr           	#53
	swl     [r6 + 33] = r0; # sram.cmd<=cmd_load_line          	#54

#poll_sdram_ready:
	lwl     r1  = [r6 + 33]; # sdram.status                    	#55
	addi	r0 = r0, 0;                                           	#56
	cmpneq  p1 = r1, r0;     # ?busy                           	#57
	(p1)	bc	55; #l:poll_sdram_ready                            	#58
                addi    r0  = r0 , 0;                       	#59
                addi    r0  = r0 , 0;                       	#60

#skip_load:
#	 Check the value in the cache line
	andi	r2  = r10, 63;  # offset                              	#61
	add	r2  = r6, r2;                                          	#62
	lwl	r2 = [r2 + 0];	# sram.word[offset] => value            	#63
	addi    r1  = r10 , 1;  # val = addr+1 (use the same mod as during write)	#64

	cmpeq	p1 = r1, r2; # should be the same                    	#65
	(p1)	bc  70; #l:+3                                         	#66
        (!p1)   addi    r1  = r0 , 69;  'E'                 	#67
	(!p1)	addi    r12 = r12, 1; #error_cnt++                   	#68
	swl     [r5 + 1] = r1; # we write to UART without pooling for ready here,	#69


#	check the memory range for the test and advance to next word
	addi	r10 = r10, 4;                                         	#70
	cmplt  p1 = r10, r11;                                     	#71
	(p1)	bc	48; # l:read_word                                  	#72
                addi    r0  = r0 , 0;                       	#73
                addi    r0  = r0 , 0;                       	#74

# read test done, output result and go back to start
# output 'OK' or '##' if err	
#poll_stdout:
	lwl     r1 = [r5 + 0];                                     	#75
	addi	r2 = r0, 1;                                           	#76
	and     r1 = r2, r1;                                       	#77
	cmpneq  p1 = r1, r2;                                       	#78
	(p1)	bc	75;   #l:poll_stdout                               	#79
                addi    r0  = r0 , 0;                       	#80
                addi    r0  = r0 , 0;                       	#81

	cmpneq  p2 = r12, r0; # ?error_cnt != 0                    	#82

#	 79 '0', 35 '#'
		addi	r1 = r0, 79;                                         	#83
	(p2)	addi	r1 = r0, 35;                                     	#84
	swl     [r5 + 1] = r1;                                     	#85

#poll_stdout:
	lwl     r1 = [r5 + 0];                                     	#86
	addi	r2 = r0, 1;                                           	#87
	and     r1 = r2, r1;                                       	#88
	cmpneq  p1 = r1, r2;                                       	#89
	(p1)	bc	86;   #l:poll_stdout                               	#90
                addi    r0  = r0 , 0;                       	#91
                addi    r0  = r0 , 0;                       	#92

#	 75 'K', 35 '#'
		addi	r1 = r0, 75;                                         	#93
	(p2)	addi	r1 = r0, 35;                                     	#94
	swl     [r5 + 1] = r1;                                     	#95

	lwl     r0 = [r5 + 1];  # purge input                      	#96
	bc	1;                                                      	#97
		addi    r0 = r0, 0;                                       	#98
		addi    r0 = r0, 0;                                       	#99
halt;                                                       	#100
