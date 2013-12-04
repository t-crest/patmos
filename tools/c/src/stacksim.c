/*

	A C based simulation of the stack cache.

	Author: Martin Schoeberl (martin@jopdesign.com)

	By LLVM/Patmos convention the stack grows towards lower addresses.
	Therefore the top of stack (cache or memory) is the smallest address.

	TODO: this version might be off by one... see paper and pasim
*/

#include <stdio.h>

#define MEM_SIZE 1024
#define SC_SIZE 64
#define SC_MASK (SC_SIZE-1)

// An arbitrary start of the stack area
#define STACK_START 500

// The main memory
static int mem[1024];
// The stack cache
static int sc[SC_SIZE];

// Pointer the top memory saved stack content
int mem_top;

// Pointer to the top element in the stack cache
int sc_top;

// #elements is mem_top - sc_top and always has to be <= SC_SIZE

// provide n (new) free words in the S$
// may spill to main memory

void reserve(int n) {

	int nspill, i;

	sc_top -= n;
	nspill = mem_top - sc_top - SC_SIZE;
	for (i=0; i<nspill; ++i) {
		--mem_top;
		mem[mem_top] = sc[mem_top & SC_MASK];
	}

}

// drop n elements form the stack cache
// may change the top memory pointer

void free(int n) {

	sc_top += n;
	if (sc_top > mem_top) {
		mem_top = sc_top;
	}
}

// ensure that n elements are valid in the stack cache
// may fill from main memory

void ensure(int n) {

	int nfill, i;

	nfill = n - (mem_top - sc_top);
	for (i=0; i<nfill; ++i) {
		sc[mem_top & SC_MASK] = mem[mem_top];
		++mem_top;
	}
}

// store one word in the stack cache
// addr is a plain main memory address

void store(int addr, int val) {
	sc[addr & SC_MASK] = val;
}

// load one word from the stack cache
// addr is a plain main memory address

int load(int addr) {
	return sc[addr & SC_MASK];
}


void dump() {
	
	int i;
	printf("mem_top %d sc_top %d\n", mem_top, sc_top);
// printf("mem[..] %d\n", mem[STACK_START-5]);
	for (i=0; i<SC_SIZE; ++i) {
		printf("%d ", sc[i]);
	}
	printf("\n");
}

 
int main(int argc, char **argv) {

	// initialize the stack area
	mem_top = STACK_START;
	sc_top = STACK_START;

	// the first stack frame
	reserve(10);
	store(STACK_START-5, 123);
	dump();
	// all shall go to main memory
	reserve(SC_SIZE);
	// reuse the slot
	store(STACK_START-SC_SIZE-5, 456);
	dump();
	free(SC_SIZE);
	dump();
	// reload the first stack frame
	ensure(10);
	dump();
	// some more tests would be good

	return 0;
}
