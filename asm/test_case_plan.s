# Pipeline stages:
# (1) Instruction fetch			[IF]
# (2) Decode and read register	[DR]
# (3) Execute						[EX, EX_1, ..., EX_n]
# (4) Memory access				[MW]
# (5) Register write back			[WB]
# 
# 
# Patmos test cases:
# 
# ALU instructions
# - ALU Immediate
# - ALU Long Immediate
# - ALU Arithmetic
# 
# SPC instrictions 
# - Wait
# - Move to special
# - Move from special
# 
# LDT instruction
# 
# STT intruction
# 
# STC instruction
# 
# CLF instructions
# - Call/Branch
# - Call/Branch Indirect
# - Return
# 
# Register file
# - Forwarding
# - General-purpose registers
# - Special registers
# - Predicate registers
# 
# Predicated instructions
# 
# Forwarding
# 
# Bundled instructions
# 
# Function calls
# 

		addi	r0 = r0, 0;

add:	addi	r1 = r0, 5; 
		add		r2 = r1, r1;	# r2 = 10

# br exit; # Should jump to correct exit label, using halt or loop

addi: 	addi	r3 = r2, 100; # r3 = 110

# br exit; # Should jump to correct exit label, using halt or loop

# addl: 	addl	r4 = r0, 0xffffffff; # r4 = 4294967295
# addl not yet implemented in assembler

# br exit; # Should jump to correct exit label, using halt or loop

sub: 	sub		r3 = r3, r2; # r3 = 100

# br exit; # Should jump to correct exit label, using halt or loop

rsub: 	rsub	r3 = r3, r2; # r3 = -90
		rsub	r3 = r3, r2; # r3 = 100

# br exit; # Should jump to correct exit label, using halt or loop

subi: 	subi	r4 = r3, 50; # r4 = 50

# br exit; # Should jump to correct exit label, using halt or loop

sl: 	sl		r4 = r4, 1; # r4 = 100

# br exit; # Should jump to correct exit label, using halt or loop

sr: 	sr		r4 = r4, 2; # r4 = 25

# br exit; # Should jump to correct exit label, using halt or loop

sra: 	addi	r5 = r0, 4095;
		sl		r5 = r5, 20;
		sra		r4 = r4, 20; # r4 = 0xffffffff

# br exit; # Should jump to correct exit label, using halt or loop

or: 	or		r4 = r4, 0; # r4 = 0xffffffff

# br exit; # Should jump to correct exit label, using halt or loop

and: 	and		r4 = r4, 0; # r4 = 0x0

# br exit; # Should jump to correct exit label, using halt or loop

rl: 	addi	r6 = r0, 1;
		rl		r6 = r6, r1;

# br exit; # Should jump to correct exit label, using halt or loop

rr: 	rr		r6 = r6, r1; # r6 = 1

# br exit; # Should jump to correct exit label, using halt or loop

xor: 	xor		r6 = r6, r1; # r6 = 2

# br exit; # Should jump to correct exit label, using halt or loop

nor: 	addi	r7 = r0, 0;
		nor		r6 = r6, r7; # r6 = 4294967293

# br exit; # Should jump to correct exit label, using halt or loop

shadd: 	shadd	r8 = r1, 1; # r8 = 11

# br exit; # Should jump to correct exit label, using halt or loop

shadd2: shadd2	r8 = r8, 1; # r8 = 23

# br exit; # Should jump to correct exit label, using halt or loop

#mov: 	mov		r8 = r7;

# br exit; # Should jump to correct exit label, using halt or loop

#neg: 	addi	r7 = r0, 10;
#		neg		r7 = r7; # r7 = -10

# br exit; # Should jump to correct exit label, using halt or loop

#not: 	not		r7 = r7; # r7 = 9

# br exit; # Should jump to correct exit label, using halt or loop

#zext8: 	zext8	r6 = r6;

# br exit; # Should jump to correct exit label, using halt or loop

#li: 	li		

# br exit; # Should jump to correct exit label, using halt or loop

#nop: 	nop		

# br exit; # Should jump to correct exit label, using halt or loop

sext8:	addi	r1 = r0, 255;
		sext8	r2 = r1; # r2 = 4294967295

# br exit; # Should jump to correct exit label, using halt or loop

sext16: addi	r1 = r0, 255;
		sext16	r2 = r1; # r2 = 255

# br exit; # Should jump to correct exit label, using halt or loop

zext16: zext16	r2 = r1; # r2 = 255

# br exit; # Should jump to correct exit label, using halt or loop

abs: 	addi	r3 = r0, 10;
		sub		r3 = r0, r3;
		abs		r4 = r3; # r4 = 10

# br exit; # Should jump to correct exit label, using halt or loop

#mul: 	mul		r3, r2; # r5 = -2550

# br exit; # Should jump to correct exit label, using halt or loop

#mulu: 	mulu	r4, r2; # r5 = 2550

# br exit; # Should jump to correct exit label, using halt or loop

cmpeq: 	cmpeq	p1 = r1, r2;

# br exit; # Should jump to correct exit label, using halt or loop

cmpneq: cmpneq	p2 = r1, r3;

# br exit; # Should jump to correct exit label, using halt or loop

#cmplt: 	cmplt	

# br exit; # Should jump to correct exit label, using halt or loop

#cmple: 	cmple		

# br exit; # Should jump to correct exit label, using halt or loop

#cmpult: cmpult		

# br exit; # Should jump to correct exit label, using halt or loop

#cmpule: cmpule		

# br exit; # Should jump to correct exit label, using halt or loop

#btest: 	btest		

# br exit; # Should jump to correct exit label, using halt or loop

#isodd: 	isodd		

# br exit; # Should jump to correct exit label, using halt or loop

#set: 	set		

# br exit; # Should jump to correct exit label, using halt or loop

#clr: 	clr		

# br exit; # Should jump to correct exit label, using halt or loop

#wait.mem: 	wait.mem		

# br exit; # Should jump to correct exit label, using halt or loop

#mts: 	mts		

# br exit; # Should jump to correct exit label, using halt or loop

#mfs: 	mfs		

# br exit; # Should jump to correct exit label, using halt or loop

#lws: 	lws		

# br exit; # Should jump to correct exit label, using halt or loop

#lwl: 	lwl		

# br exit; # Should jump to correct exit label, using halt or loop

#lwc: 	lwc		

# br exit; # Should jump to correct exit label, using halt or loop

#lwm: 	lwm		

# br exit; # Should jump to correct exit label, using halt or loop

#lhs: 	lhs		

# br exit; # Should jump to correct exit label, using halt or loop

#lhl: 	lhl		

# br exit; # Should jump to correct exit label, using halt or loop

#lhc: 	lhc		

# br exit; # Should jump to correct exit label, using halt or loop

#lhm: 	lhm		

# br exit; # Should jump to correct exit label, using halt or loop

#lbs: 	lbs		

# br exit; # Should jump to correct exit label, using halt or loop

#lbl: 	lbl		

# br exit; # Should jump to correct exit label, using halt or loop

#lbc: 	lbc		

# br exit; # Should jump to correct exit label, using halt or loop

#lbm: 	lbm		

# br exit; # Should jump to correct exit label, using halt or loop

#lhus: 	lhus		

# br exit; # Should jump to correct exit label, using halt or loop

#lhul: 	lhul		

# br exit; # Should jump to correct exit label, using halt or loop

#lhuc: 	lhuc		

# br exit; # Should jump to correct exit label, using halt or loop

#lhum: 	lhum		

# br exit; # Should jump to correct exit label, using halt or loop

#lbus: 	lbus		

# br exit; # Should jump to correct exit label, using halt or loop

#lbul: 	lbul		

# br exit; # Should jump to correct exit label, using halt or loop

#lbuc: 	lbuc		

# br exit; # Should jump to correct exit label, using halt or loop

#lbum: 	lbum		

# br exit; # Should jump to correct exit label, using halt or loop

#dlwc: 	dlwc		

# br exit; # Should jump to correct exit label, using halt or loop

#dlwm: 	dlwm		

# br exit; # Should jump to correct exit label, using halt or loop

#dlhc: 	dlhc		

# br exit; # Should jump to correct exit label, using halt or loop

#dlhm: 	dlhm		

# br exit; # Should jump to correct exit label, using halt or loop

#dlbc: 	dlbc		

# br exit; # Should jump to correct exit label, using halt or loop

#dlbm: 	dlbm		

# br exit; # Should jump to correct exit label, using halt or loop

#dlhuc: 	dlhuc		

# br exit; # Should jump to correct exit label, using halt or loop

#dlhum: 	dlhum		

# br exit; # Should jump to correct exit label, using halt or loop

#dlbuc: 	dlbuc		

# br exit; # Should jump to correct exit label, using halt or loop

#dlbum: 	dlbum		

# br exit; # Should jump to correct exit label, using halt or loop

#sws: 	sws		

# br exit; # Should jump to correct exit label, using halt or loop

#swl: 	swl		

# br exit; # Should jump to correct exit label, using halt or loop

#swc: 	swc		

# br exit; # Should jump to correct exit label, using halt or loop

#swm: 	swm		

# br exit; # Should jump to correct exit label, using halt or loop

#shs: 	shs		

# br exit; # Should jump to correct exit label, using halt or loop

#shl: 	shl		

# br exit; # Should jump to correct exit label, using halt or loop

#shc: 	shc		

# br exit; # Should jump to correct exit label, using halt or loop

#shm: 	shm		

# br exit; # Should jump to correct exit label, using halt or loop

#sbs: 	sbs		

# br exit; # Should jump to correct exit label, using halt or loop

#sbl: 	sbl		

# br exit; # Should jump to correct exit label, using halt or loop

#sbc: 	sbc		

# br exit; # Should jump to correct exit label, using halt or loop

#sbm: 	sbm		

# br exit; # Should jump to correct exit label, using halt or loop

#sres: 	sres		

# br exit; # Should jump to correct exit label, using halt or loop

#sens: 	sens		

# br exit; # Should jump to correct exit label, using halt or loop

#sfree: 	sfree		

# br exit; # Should jump to correct exit label, using halt or loop

#call: 	call		

# br exit; # Should jump to correct exit label, using halt or loop

#br: 	br		

# br exit; # Should jump to correct exit label, using halt or loop

#brcf: 	brcf		

# br exit; # Should jump to correct exit label, using halt or loop

#ret: 	ret		

# br exit; # Should jump to correct exit label, using halt or loop

exit: 	halt;