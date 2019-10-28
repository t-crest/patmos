#include "../hardlock/hardlock.h"
#include "stack.h"

void intialize(stack_t * stack_ptr)
{
	_intialize(stack_ptr);
}

void push(stack_t * stack_ptr, element_t * element_ptr)
{
	lock(0);
	_push(stack_ptr, element_ptr);
	unlock(0);
}

element_t * pop(stack_t * stack_ptr)
{
	lock(0);
	element_t * element_ptr = _pop(stack_ptr);
	unlock(0);
	return element_ptr;
}