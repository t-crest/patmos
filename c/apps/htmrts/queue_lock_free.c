#include "htmrts.h"
#include "queue.h"

void intialize(queue_t * queue_ptr)
{
	asm volatile ("" : : : "memory");
	do
	{
		_intialize(queue_ptr);
	}
	while(*HTMRTS_COMMIT != 0);
	asm volatile ("" : : : "memory");
}

void initialize_element(element_t * element_ptr, val_t val)
{
	asm volatile ("" : : : "memory");
	do
	{
		_initialize_element(element_ptr, val);
	}
	while(*HTMRTS_COMMIT != 0);
	asm volatile ("" : : : "memory");
}

void enqueue(queue_t * queue_ptr, element_t * element_ptr) __attribute__((noinline));
void enqueue(queue_t * queue_ptr, element_t * element_ptr)
{
	asm volatile ("" : : : "memory");
	#pragma loopbound min 1 max 1
	do
	{
		_enqueue(queue_ptr, element_ptr);
	}
	while(*HTMRTS_COMMIT != 0);
	asm volatile ("" : : : "memory");
}

element_t * dequeue(queue_t * queue_ptr) __attribute__((noinline));
element_t * dequeue(queue_t * queue_ptr)
{
	asm volatile ("" : : : "memory");
	element_t * last_ptr;
	#pragma loopbound min 1 max 1
	do
	{
		last_ptr = _dequeue(queue_ptr);
	}
	while(*HTMRTS_COMMIT != 0);
	asm volatile ("" : : : "memory");
	return last_ptr;
}
