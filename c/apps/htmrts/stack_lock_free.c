#include "htmrts.h"
#include "stack.h"

void intialize(stack_t * stack_ptr)
{
	do
	{
		_intialize(stack_ptr);
	}
	while(*HTMRTS_COMMIT != 0);
}

void push(stack_t * stack_ptr, element_t * element_ptr)
{
	do
	{
		_push(stack_ptr, element_ptr);
	}
	while(*HTMRTS_COMMIT != 0);
}

element_t * pop(stack_t * stack_ptr)
{
	element_t * element_ptr;
	do
	{
		element_ptr = _pop(stack_ptr);
	}
	while(*HTMRTS_COMMIT != 0);
	return element_ptr;
}