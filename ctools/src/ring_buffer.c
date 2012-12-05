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
    cb->spill_fill = spill_fill;
    cb->head = cb->sc_size - 1;
    cb->tail = spill_fill & (cb->sc_size - 1);	
    cb->count = 0;
    cb->sc = (ElemType *)calloc(cb->sc_size, sizeof(ElemType));
    cb->mem = (ElemType *)calloc(cb->mem_size, sizeof(ElemType));	
}
 
void cbNull(CircularBuffer *cb) {
	free(cb->sc); // OK if null
	free(cb->mem);   
	}
 
 
void cbReserve(CircularBuffer *cb, int res_count ) {
	if ((abs(cb->head -(cb->spill_fill - 79 +15))) + res_count > cb->sc_size)
	{
		for(t = 0; t < (res_count - (cb->sc_size - (abs(cb->head -(cb->spill_fill - 79 +15))))); t++) // res_count - number of free slots
		{
			cb->mem[cb->spill_fill - t] =  cb->sc[(cb->spill_fill - t) & (cb->sc_size - 1)];
		}
		cb->spill_fill = cb->spill_fill - (res_count - (cb->sc_size - (abs(cb->head -(cb->spill_fill - 79 +15)))));
		cb->tail = cb->spill_fill & (cb->sc_size - 1);	//  this is just to print and check
	}
	cb->head =( cb->head - res_count) ;

	printf("Reserve: head:%d\n", cb->head & (cb->sc_size - 1));
	printf("Reserve: tail:%d\n", cb->tail);	
//	printf("Reserve: count:%d\n", cb->count);
	printf("Reserve: spill_fill:%d\n", cb->spill_fill);	
	printf("\n");	
}

void cbFree(CircularBuffer *cb, int free_count ) {
	if (abs(cb->head -(test - 79 +15)) < free_count)
	{
		cb->spill_fill = cb->spill_fill + (free_count - (abs(cb->head -(cb->spill_fill - 79 +15)))); // obviously...!
	
	}
	cb->head = (cb->head + free_count);
	printf("Free: head:%d\n", cb->head & (cb->sc_size - 1));
	printf("Free: tail:%d\n", cb->tail);	
	//printf("Free: count:%d\n", cb->count);
	printf("Free: spill_fill:%d\n", cb->spill_fill);
	printf("\n");				
}

void cbEnsure(CircularBuffer *cb, int ens_count ) {
	int k = 0;
	int test = cb->spill_fill;
	if (((abs(cb->head -(test - 79 +15)))) <  ens_count)
	 {// check fill, if there are at least the same number of slots...	
		for(t = (abs(cb->head -(test - 79 +15))); t < ens_count; t++) //fill
		{
			cb->sc[cb->spill_fill & (cb->sc_size-1)] =  cb->mem[cb->spill_fill];
			cb->tail = cb->tail++ ;			
			cb->spill_fill++; 
		}
		cb->tail = cb->spill_fill & (cb->sc_size-1); // this is just for printing the value, we point to sc using spill_fill...
		k = cb->tail % cb->sc_size; //& (cb->sc_size - 1); // this is where it assumes whatever so we need to find a limit on this... //or we just use % for c...
	}
	
	printf("Ensure: head:%d\n", cb->head & (cb->sc_size - 1));
	printf("Ensure: tail:%d\n", cb->tail);	
	//printf("Ensure: count:%d\n", cb->count);
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
	cbInit(&cb, addrSize, memSize, 79);
	
	printf("Test1:\n");
 	cbReserve(&cb, 10); //reserve 10 elements
	cbReserve(&cb, 10); //reserve 10 elements
	cbReserve(&cb, 8); //reserve 8 elements
	//cbReserve(&cb, 8);


	cbFree(&cb, 8);
	cbEnsure(&cb, 10);

	cbFree(&cb, 10);
  
	cbEnsure(&cb, 10);

	cbFree(&cb, 10);

	printf("Test2:\n");
	cbReserve(&cb, 10); //reserve 10 elements
	cbReserve(&cb, 16); 

	cbFree(&cb, 16);
	cbEnsure(&cb, 10);

	cbFree(&cb, 10);
 
	printf("Test3:\n");
	cbReserve(&cb, 10); //reserve 10 elements
	cbReserve(&cb, 10); //reserve 10 elements
	cbReserve(&cb, 16); //reserve 8 elements
	
	cbFree(&cb, 16);
	cbFree(&cb, 10);
	
	cbEnsure(&cb, 10);
	cbFree(&cb, 10);
	

	cbNull(&cb);
	return 0;
}
