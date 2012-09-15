#
# Test the SDRAM I/O device
# Note
#
# Register conventions:
# r0==0, temp: r1, r2, r3
# r5==uart base
# r6==sdram base
# r10==counts address for memory read/write test
# r11==read/write test bound (address limit)
# r12==memory read error count


	addi	r0 = r0, 0;  # first instruction not executed         	#0
	addi	r12 = r0, 0; # r12==error count                       	#1
	addi	r5 = r0, 15;                                          	#2
	sli	r5 = r5, 28; # r5==uart base                           	#3
	addi	r6 = r5, 768;# r6==sdram base                         	#4
	addi	r0 = r0, 0;  # Nop: (don't want to change all the branches again)	#5

# wait_start:   # Output 'S' and wait for any key press
	addi	r1 = r0, 83; # 'S'                                    	#6
	swm     [r5 + 1] = r1;                                     	#7

	lwm     r1 = [r5 + 0];                                     	#8
	addi	r2 = r0, 2;                                           	#9
	and     r1 = r2, r1;                                       	#10
	cmpneq  p1 = r1, r2;                                       	#11
	(p1)	bc	6;                                                 	#12
                addi    r0  = r0 , 0;                       	#13
                addi    r0  = r0 , 0;                       	#14


# SDRAM I/O address space
#    addr(5) selects between special registers/word access, so the offset has to be added to 16
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
	addi	r10 = r0, 0;	# r10 == addr_cnt                        	#15
	addi	r11 = r0, 1;	# r11 == test_limit (2^10 blocks)        	#16
	sli	r11 = r11, 10;                                         	#17
	addi	r1= r0, 46; '.'                                       	#18
	swm     [r5 + 1] = r1;                                     	#19

#write:
#	Set the value in the cache line (we don't care if it is loaded as we just use one word)
	addi    r1  = r10, 1;   # val = addr+1 (+1 just for fun, to have slightly different value)	#20
	swm	[r6 + 0] = r10;	# sram.word[0] <= value                	#21

#	Ask sdram I/O to store the line into memory
	swm     [r6 + 16] = r10; # sram.addr<=addr_cnt             	#22
	addi    r1  = r0 , 1;                                      	#23
	swm     [r6 + 17] = r1; # sram.cmd<=cmd_store_line         	#24

#	wait untill I/O ready	
	lwm     r1  = [r6 + 17]; # sdram.status                    	#25
	addi	r0 = r0, 0;                                           	#26
	cmpneq  p1 = r1, r0;     # ?busy                           	#27
	(p1)	bc	25;                                                	#28
                addi    r0  = r0 , 0;                       	#29
                addi    r0  = r0 , 0;                       	#30


#	check the memory range for the test
	addi	r10 = r10, 1;                                         	#31
	cmpneq  p1 = r10, r11;                                     	#32
	(p1)	bc	20; # do next write                                	#33
                addi    r0  = r0 , 0;                       	#34
                addi    r0  = r0 , 0;                       	#35

#read_init:
	addi	r1= r0, 47; '/'                                       	#36
	swm     [r5 + 1] = r1;                                     	#37
	addi	r10 = r0, 0;	# addr_cnt <= 0                          	#38
#	 r10 should have the value from before

#read:
#	 Ask sdram I/O to load the line into memory
	swm     [r6 + 16] = r10; # sram.addr<=addr_cnt             	#39
	swm     [r6 + 17] = r0; # sram.cmd<=cmd_load_line          	#40

#	 wait untill I/O ready	
	lwm     r1  = [r6 + 17]; # sdram.status                    	#41
	addi	r0 = r0, 0;                                           	#42
	cmpneq  p1 = r1, r0;     # ?busy                           	#43
	(p1)	bc	41;                                                	#44
                addi    r0  = r0 , 0;                       	#45
                addi    r0  = r0 , 0;                       	#46

#	 Check the value in the cache line (we only used first word)
	lwm	r2 = [r6 + 0];	# sram.word[0] => value                 	#47
	addi    r1  = r10 , 1;  # val = addr+1 (use the same mod as during write)	#48
	cmpneq  p1 = r1, r2; # should be the same                  	#49
                addi    r0  = r0 , 0;                       	#50
                addi    r1  = r0 , 33;  '!'                 	#51
	(p1)	addi    r12 = r12, 1; #error_cnt++                    	#52
	(p1)	swm     [r5 + 1] = r1; # we write to UART without pooling for ready here,	#53


#	 check the memory range for the test
	addi	r10 = r10, 1;                                         	#54
	cmpneq  p1 = r10, r11;                                     	#55
	(p1)	bc	39; # do next read                                 	#56
                addi    r0  = r0 , 0;                       	#57
                addi    r0  = r0 , 0;                       	#58

# read test done, output result and go back to start
	addi	r1 = r0, 10; # '\n'                                   	#59
	swm	[r5 + 1] = r1;                                         	#60

	cmpneq  p2 = r12, r0; # ?error_cnt != 0                    	#61

# output 'OK' or '##' if err	
	lwm     r2  = [r5 + 0];                                    	#62
                addi    r3  = r0 , 1;                       	#63
	and     r2  = r3 , r2;                                     	#64
	cmpneq  p1 = r3, r2;                                       	#65
	(p1)	bc	62;                                                	#66

#	 79 '0', 35 '#'
		addi	r1 = r0, 79;                                         	#67
	(p2)	addi	r1 = r0, 35;                                     	#68
	swm     [r5 + 1] = r1;                                     	#69

	lwm     r2  = [r5 + 0];                                    	#70
                addi    r3  = r0 , 1;                       	#71
	and     r2  = r3 , r2;                                     	#72
	cmpneq  p1 = r3, r2;                                       	#73
	(p1)	bc	70;                                                	#74

#	 75 'K', 35 '#'
		addi	r1 = r0, 75;                                         	#75
	(p2)	addi	r1 = r0, 35;                                     	#76
	swm     [r5 + 1] = r1;                                     	#77

	b	1;                                                       	#78
	addi    r0 = r0, 0;                                        	#79
	addi    r0 = r0, 0;                                        	#80
halt;                                                       	#81
