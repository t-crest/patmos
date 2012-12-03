#include <stdio.h>
// Mac does not like malloc.h
#include <malloc.h> // SA: why commenting this, I don't get it...
#include <math.h>
// MS: read that stdlib.h shall be used, but that did not work either.
//#include <stdlib.h>
 
// Opaque buffer element type.  
typedef struct { int value; } ElemType;
 int t;
// Circular buffer object
typedef struct {
    int         sc_size;   // maximum number of sc elements           
    int 	address_size;
    int         mem_size;  // maximum number of mem elements            
    int         head;  // reserve, free            
    int         tail;    // spill, fill 
    int         count; // number of occupied slots
    int         spill_fill; //stack pointer
    ElemType   *sc;  // vector of sc
    ElemType   *mem;  // vector of main memory
} CircularBuffer;
 
void cbInit(CircularBuffer *cb, int address_size, int mem_size, int spill_fill) {
    cb->sc_size  = pow(2, address_size);//sc_size;
    cb->mem_size  = mem_size;	
    cb->spill_fill       = spill_fill;
    cb->head = 15;
    cb->tail = 15;	
    cb->count = 0;
    cb->sc = (ElemType *)calloc(cb->sc_size, sizeof(ElemType));
    cb->mem = (ElemType *)calloc(cb->mem_size, sizeof(ElemType));	
}
 
void cbNull(CircularBuffer *cb) {
	free(cb->sc); // OK if null
	free(cb->mem);   
	}
 
 
void cbReserve(CircularBuffer *cb, int res_count ) {
	if ((cb->count + res_count) <= cb->sc_size)	
		cb->count = cb->count + res_count; //update number of occupied slots
	else    //check spill
	{
		
		for(t = 0; t < (res_count - (cb->sc_size - cb->count)); t++) // res_count - number of free slots
		{
			cb->mem[cb->spill_fill] =  cb->sc[(cb->tail & (cb->sc_size - 1)) - 1];
			cb->tail = cb->tail--;
			cb->spill_fill--; 
		}
		cb->tail = cb->tail & (cb->sc_size - 1);
		cb->count = cb->sc_size;	// stack cache is full
	}
		cb->head = ((cb->head - (res_count)) & (cb->sc_size - 1));

	printf("Reserve: head:%d\n", cb->head);
	printf("Reserve: tail:%d\n", cb->tail);	
	printf("Reserve: count:%d\n", cb->count);
	printf("Reserve: spill_fill:%d\n", cb->spill_fill);	
	printf("\n");	
}

void cbFree(CircularBuffer *cb, int free_count ) {
	

	if (cb->count >= free_count)
	{
		cb->count = cb->count - free_count; //update number of occupied slots
		cb->head = (cb->head + free_count) &  (cb->sc_size - 1);
	}
	else
	{
		cb->spill_fill = cb->spill_fill + (free_count - cb->count); // obviously...!
		cb->count = 0;
		cb->head = cb->tail = 16;
	}
	printf("Free: head:%d\n", cb->head);
	printf("Free: tail:%d\n", cb->tail);	
	printf("Free: count:%d\n", cb->count);
	printf("Free: spill_fill:%d\n", cb->spill_fill);
	printf("\n");				
}

void cbEnsure(CircularBuffer *cb, int ens_count ) {
	if ((cb->count) <  ens_count) {// check fill, if there are at least the same number of slots...	
		for(t = cb->count; t < ens_count; t++) //fill
		{
			cb->sc[cb->tail - 1] =  cb->mem[cb->spill_fill];
			cb->tail <= cb->tail++ ;			
			cb->spill_fill++; 
		}
		cb->count = ens_count; 
		cb->tail <= cb->tail & (cb->sc_size-1);
	}
	printf("Ensure: head:%d\n", cb->head);
	printf("Ensure: tail:%d\n", cb->tail);	
	printf("Ensure: count:%d\n", cb->count);
	printf("Ensure: spill_fill:%d\n", cb->spill_fill);		
	printf("\n");		
}

 
int main(int argc, char **argv) {
	CircularBuffer cb;
	ElemType elem = {0};
 

// MS: MASK = SIZE-1 works ONLY when SIZE is a power of 2. // SA: Yes, I forgot to change that...
// Define the number of address bits n and size as 2 ** n.
	int addrSize = 4; 
	int memSize = 50;
	cbInit(&cb, addrSize, memSize, 35);
	
	printf("Test1:\n");
 	cbReserve(&cb, 10); //reserve 10 elements
	cbReserve(&cb, 10); //reserve 10 elements
	cbReserve(&cb, 8); //reserve 8 elements
	cbReserve(&cb, 8);


	/*cbFree(&cb, 8);
	cbEnsure(&cb, 10);

	cbFree(&cb, 10);
  
	cbEnsure(&cb, 10);

	cbFree(&cb, 10);*/

	/*printf("Test2:\n");
	cbReserve(&cb, 10); //reserve 10 elements
	cbReserve(&cb, 16); 

	cbFree(&cb, 16);
	cbEnsure(&cb, 10);

	cbFree(&cb, 10);
 
	printf("Test3:\n");
	cbReserve(&cb, 10); //reserve 10 elements
	cbReserve(&cb, 10); //reserve 10 elements
	cbReserve(&cb, 16); //reserve 8 elements*/
	/*
	cbFree(&cb, 16);
	cbFree(&cb, 10);
	
	cbEnsure(&cb, 10);
	cbFree(&cb, 10);*/
	

	cbNull(&cb);
	return 0;
}
