#include "htmrts.h"
#include "queue.h"

void intialize(queue_t * queue_ptr)
{
	do
	{
		_intialize(queue_ptr);
	}
	while(*HTMRTS_COMMIT != 0);
}

void initialize_element(element_t * element_ptr, val_t val)
{
	do
	{
		_initialize_element(element_ptr, val);
	}
	while(*HTMRTS_COMMIT != 0);
}

void enqueue(queue_t * queue_ptr, element_t * element_ptr)
{
	do
	{
		_enqueue(queue_ptr, element_ptr);
	}
	while(*HTMRTS_COMMIT != 0);
}

element_t * dequeue(queue_t * queue_ptr)
{
	element_t * last_ptr;
	do
	{
		last_ptr = _dequeue(queue_ptr);
	}
	while(*HTMRTS_COMMIT != 0);
	return last_ptr;
}

val_t dequeue_val(queue_t * queue_ptr)
{
	val_t val;
	do
	{
		val = _dequeue_val(queue_ptr);
	}
	while(*HTMRTS_COMMIT != 0);
	return val;
}
