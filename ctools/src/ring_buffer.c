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
    int         head_full;
    int         tail_full;	
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
    cb->head_full = cb->sc_size - 1;	
    cb->tail = spill_fill & (cb->sc_size - 1);	
    cb->tail_full = spill_fill & (cb->sc_size - 1);	
    cb->count = 0;
    cb->sc = (ElemType *)calloc(cb->sc_size, sizeof(ElemType));
    cb->mem = (ElemType *)calloc(cb->mem_size, sizeof(ElemType));	
}
 
void cbNull(CircularBuffer *cb) {
	free(cb->sc); // OK if null
	free(cb->mem);   
	}
 

void store(CircularBuffer *cb, int address, ElemType data){
	cb->sc[(cb->head + address) & (cb->sc_size - 1)] = data;
}

ElemType load(CircularBuffer *cb, int address){
	return cb->sc[cb->head + address];
}

int num_valid_bytes(int head, int tail){
	//cb->count = abs(head - tail);
	return abs(head - tail);
}
 
void cbReserve(CircularBuffer *cb, int res_count ) {
	printf("count_new:%d\n", num_valid_bytes(cb->head_full, cb->tail_full));
	int count;	
	count =  num_valid_bytes(cb->head_full, cb->tail_full);
	if ((count + res_count) > cb->sc_size)
	{
		for(t = 0; t < (res_count - (cb->sc_size - count)); t++) // res_count - number of free slots
		{
			cb->mem[cb->spill_fill] =  cb->sc[cb->spill_fill & (cb->sc_size - 1)];
			printf("spill%d %d ", t, cb->mem[cb->spill_fill] );
			cb->spill_fill--; 
			cb->tail_full--; 
		}
		printf("\n");
		cb->tail = cb->spill_fill & (cb->sc_size - 1);	//  this is just to print and check
		
	}
		cb->head = ((cb->head - (res_count)) & (cb->sc_size - 1));
		cb->head_full = cb->head_full - res_count;
	printf("Reserve: head_full:%d\n", cb->head_full);
	printf("Reserve: tail_full:%d\n", cb->tail_full);	
	printf("Reserve: head:%d\n", cb->head);
	printf("Reserve: tail:%d\n", cb->tail);	
	printf("Reserve: spill_fill:%d\n", cb->spill_fill);	
	printf("\n");	
}

void cbFree(CircularBuffer *cb, int free_count ) {
	printf("count_new:%d\n", num_valid_bytes(cb->head_full, cb->tail_full));
	int count;	
	count =  num_valid_bytes(cb->head_full, cb->tail_full);
	if (count < free_count)
	{
		cb->spill_fill = cb->spill_fill + (free_count - count); // obviously...!
		cb->tail_full = cb->tail_full + free_count;
		//cb->count = 0;

	}
	cb->head = (cb->head + free_count) & (cb->sc_size - 1);
	cb->head_full = cb->head_full + free_count;
	printf("Free: head_full:%d\n", cb->head_full);
	printf("Free: tail_full:%d\n", cb->tail_full);
	printf("Free: head:%d\n", cb->head);
	printf("Free: tail:%d\n", cb->tail);	
	printf("Free: spill_fill:%d\n", cb->spill_fill);
	printf("\n");				
}

void cbEnsure(CircularBuffer *cb, int ens_count ) {
	printf("count_new:%d\n", num_valid_bytes(cb->head_full, cb->tail_full));
	int count;	
	count =  num_valid_bytes(cb->head_full, cb->tail_full);
	if (count <  ens_count)
	 {// check fill, if there are at least the same number of slots...	
		for(t = count; t < ens_count; t++) //fill
		{
			cb->sc[cb->tail & (cb->sc_size-1)] =  cb->mem[cb->spill_fill];
			cb->tail_full = cb->tail_full++ ;			
			cb->spill_fill++; 
		}
		cb->tail = cb->spill_fill & (cb->sc_size-1); // this is just for printing the value, we point to sc using spill_fill...
	}
	printf("Ensure: head_full:%d\n", cb->head_full);
	printf("Ensure: tail_full:%d\n", cb->tail_full);
	printf("Ensure: head:%d\n", cb->head);
	printf("Ensure: tail:%d\n", cb->tail);	
	printf("Ensure: spill_fill:%d\n", cb->spill_fill);		
	printf("\n");		
}

 
int main(int argc, char **argv) {
	CircularBuffer cb;
	ElemType elem = {10};
 	int c = 0;

// MS: MASK = SIZE-1 works ONLY when SIZE is a power of 2. // SA: Yes, I forgot to change that...
// Define the number of address bits n and size as 2 ** n.
	int addrSize = 4; 
	int memSize = 50;

	cbInit(&cb, addrSize, memSize, 79);
	
	printf("Test1:\n");
 	cbReserve(&cb, 10); //reserve 10 elements

	for (c = 10; c > 0; c--)
	{
		store(&cb, c, elem);
		(&elem)->value = c;
	}
	printf("store_1:\n"); 
	for (c = 0; c < (&cb)->sc_size; c++)
		printf("%d,%d ", c, (&cb)->sc[c] );
	printf("\n" );
//********************************************************
	cbReserve(&cb, 10); //reserve 10 elements
	(&elem)->value = 55;

	for (c = 10; c > 0; c--)
	{
		store(&cb, c, elem);
		(&elem)->value += 10;
	}
	printf("store_2:\n");
	for (c = 0; c < (&cb)->sc_size; c++)
		printf("%d,%d ", c, (&cb)->sc[c] );
	printf("\n");
//********************************************************
	cbReserve(&cb, 8); //reserve 8 elements
	//cbReserve(&cb, 8);
	(&elem)->value = 33;
	for (c = 8; c > 0; c--)
	{
		store(&cb, c, elem);
		(&elem)->value += 10;
	}
	printf("store_3:\n");
	for (c = 0; c < (&cb)->sc_size; c++)
		printf("%d,%d ", c, (&cb)->sc[c] );
	printf("\n");
//*******************************************************

//**********************check main memory****************
	printf("mem\n");
	for (c = 79; c > 66; c--)
		printf("%d,%d ", c, (&cb)->mem[c] );
	printf("\n");
//*******************************************************


	
	elem = load(&cb, 3);
	printf("load %d \n", elem );

	cbFree(&cb, 8);
	cbEnsure(&cb, 10);
//**********************check stack cache****************
	printf("stack_after_fill\n");
	for (c = 15; c >= 0; c--)
		printf("%d,%d ", c, (&cb)->sc[c] );
//*******************************************************


	cbFree(&cb, 10);
  
	cbEnsure(&cb, 10);
//	for (c = 0; c < (&cb)->sc_size; c++)
//		printf("%d,%d ", c, (&cb)->sc[c] );
	printf("\n");
	
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

