#
#  Test of call and return instructions
#
	.word   32
	addi	r1 = r0, 0
	addi	r20 = r0, 20
	addi	r16 = r0, start
	callr	r16        # we need an initial call that does not return for a start
	addi	r1 = r0, 1
	addi	r1 = r0, 2
	addi	r1 = r0, 3

	.word 100 # This looks like not working at all....
start:	addi	r1 = r1, 1
	addi	r16 = r0, foo		
	callr	r16
# we should check (and define) delay slots for call/ret - probably 2
	addi	r2 = r0, 2
	addi	r3 = r0, 3
	addi	r4 = r0, 4
# return shall come in here
	addi	r5 = r0, 5
	addi	r6 = r0, 6
	br	end
	addi	r0 = r0, 0
	addi	r0 = r0, 0
	.word 24
foo:	addi	r6 = r0, 6
	addi	r7 = r0, 7
	ret # base/offset in srb/sro
	addi	r10 = r0, 10 # how many return delay slots? Maybe 2 as all others
	addi	r11 = r0, 11
	addi	r12 = r0, 12
end:	addi	r8 = r0, 8
	addi 	r9 = r0, 9
	halt
	nop
	nop
	nop
