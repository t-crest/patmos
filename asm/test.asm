label:
	add r1


// That's not Patmos, but Leros assembler....
// Register definitions

R0 = ?

// first instruction is not executed
	nop
start:
	load 0
	loadh 2
	store r0
