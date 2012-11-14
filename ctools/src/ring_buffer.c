#include <stdio.h>
#include <malloc.h>
 
// Opaque buffer element type.  
typedef struct { int value; } ElemType;
 int t;
// Circular buffer object
typedef struct {
    int         sc_size;   // maximum number of sc elements           
    int         mem_size;  // maximum number of mem elements            
    int         head;  // reserve, free            
    int         tail;    // spill, fill 
    int         count; // number of occupied slots
    int         sp; //stack pointer
    ElemType   *sc;  // vector of sc
    ElemType   *mem;  // vector of main memory
} CircularBuffer;
 
void cbInit(CircularBuffer *cb, int sc_size, int mem_size, int sp) {
    cb->sc_size  = sc_size;
    cb->mem_size  = mem_size;	
    cb->sp       = sp;
    cb->head = 0;
    cb->tail = 0;	
    cb->count = 0;
    cb->sc = (ElemType *)calloc(cb->sc_size, sizeof(ElemType));
    cb->mem = (ElemType *)calloc(cb->mem_size, sizeof(ElemType));	
}
 
void cbNull(CircularBuffer *cb) {
	free(cb->sc); // OK if null
	free(cb->mem);   
	}
 
/*int cbIsFull(CircularBuffer *cb) {
    return cb->count == cb->sc_size; }
 
int cbIsEmpty(CircularBuffer *cb) {
    return cb->count == 0; }*/
 
void cbReserve(CircularBuffer *cb, int res_count ) {
	if ((cb->sc_size - cb->count) <  res_count)// check spill	
		for(t = 0; t < (res_count - (cb->sc_size - cb->count)); t++) // res_count - number of free slots
		{
			cb->mem[cb->sp] =  cb->sc[cb->tail];
			if ((cb->tail + 1) > cb->sc_size) 
				cb->tail = cb->tail++ % cb->sc_size;
			else
				cb->tail++;
			cb->sp++; 
		}
	cb->head = (cb->head + res_count) % cb->sc_size; // this will be elimination of higher bits in HW
	if ((cb->count + res_count) <= cb->sc_size)	
		cb->count = cb->count + res_count; //update number of occupied slots
	else    
		cb->count = cb->sc_size;	// stack cache is full
	printf("Reserve: head:%d\n", cb->head);
	printf("Reserve: tail:%d\n", cb->tail);	
	printf("Reserve: count:%d\n", cb->count);
	printf("Reserve: sp:%d\n", cb->sp);	
	printf("\n");	
}

void cbFree(CircularBuffer *cb, int free_count ) {
	if (cb->head < free_count) {	
//		temp = cb->head;
		cb->head = cb->sc_size - (free_count - cb->head);
		}
	else
		cb->head = cb->head - free_count;
	cb->count = cb->count - free_count; //update number of occupied slots
	printf("Free: head:%d\n", cb->head);
	printf("Free: tail:%d\n", cb->tail);	
	printf("Free: count:%d\n", cb->count);
	printf("Free: sp:%d\n", cb->sp);
	printf("\n");				
}

void cbEnsure(CircularBuffer *cb, int ens_count ) {
	if ((cb->count) <  ens_count)// check fill, if there are at least the same number of slots...	
		for(t = 0; t < (ens_count - cb->count); t++) //fill
		{
			cb->sc[cb->tail] =  cb->mem[cb->sp];
			if ((cb->tail - 1) > -1)
				cb->tail--;
			else     
				cb->tail <= cb->sc_size;			
			cb->sp--; 
		}
	cb->count = ens_count; // this is not ok, in case we have more that two functions 
	printf("Ensure: head:%d\n", cb->head);
	printf("Ensure: tail:%d\n", cb->tail);	
	printf("Ensure: count:%d\n", cb->count);
	printf("Ensure: sp:%d\n", cb->sp);		
	printf("\n");		
}

 
int main(int argc, char **argv) {
	CircularBuffer cb;
	ElemType elem = {0};
 
	int scSize = 20; 
	int memSize = 50;
	cbInit(&cb, scSize, memSize, 5);
	
 	cbReserve(&cb, 15); //reserve 10 elements
	cbReserve(&cb, 10); //reserve 10 elements

	cbFree(&cb, 10);
  
	cbEnsure(&cb, 15);

	cbFree(&cb, 15);
/*    for (elem.value = 0; elem.value < 3 * testBufferSize; ++ elem.value)
        cbWrite(&cb, &elem);
 
  
    while (!cbIsEmpty(&cb)) {
        cbRead(&cb, &elem);
        printf("%d\n", elem.value);
    }*/
 
	cbNull(&cb);
	return 0;
}
