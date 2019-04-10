#include "../hardlock/hardlock.h"
#include "queue.h"

void intialize(queue_t * queue_ptr)
{
	_intialize(queue_ptr);
}

void initialize_element(element_t * element_ptr, val_t val)
{
	_initialize_element(element_ptr, val);
}

void enqueue(queue_t * queue_ptr, element_t * element_ptr)
{
	lock(0);
	_enqueue(queue_ptr, element_ptr);
	unlock(0);
}

element_t * dequeue(queue_t * queue_ptr)
{
	lock(0);
	element_t * last_ptr = _dequeue(queue_ptr);
	unlock(0);
	return last_ptr;
}

val_t dequeue_val(queue_t * queue_ptr)
{
	lock(0);
	val_t val = _dequeue_val(queue_ptr);
	unlock(0);
	return val;
}
