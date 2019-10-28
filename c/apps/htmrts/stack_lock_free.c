#include "htmrts.h"
#include "stack.h"

void intialize(stack_t * stack_ptr)
{
	asm volatile ("" : : : "memory");
	do
	{
		_intialize(stack_ptr);
	}
	while(*HTMRTS_COMMIT != 0);
	asm volatile ("" : : : "memory");
}

void push(stack_t * stack_ptr, element_t * element_ptr)
{
	asm volatile ("" : : : "memory");
	do
	{
		_push(stack_ptr, element_ptr);
	}
	while(*HTMRTS_COMMIT != 0);
	asm volatile ("" : : : "memory");
}

element_t * pop(stack_t * stack_ptr)
{
	asm volatile ("" : : : "memory");
	element_t * element_ptr;
	do
	{
		element_ptr = _pop(stack_ptr);
	}
	while(*HTMRTS_COMMIT != 0);
	asm volatile ("" : : : "memory");
	return element_ptr;
}