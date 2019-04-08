#include "../hardlock/hardlock.h"
#include "stack.h"

void push(top_ptr_t top, element_t * element)
{
	lock(0);
	top_t oldtop = *top;
	element->next = (element_t *)oldtop;
	*top = (top_t)element;
	unlock(0);
}

element_t * pop(top_ptr_t top)
{
	lock(0);
	element_t * element = (element_t *)*top;
	if(element)
		*top = (top_t)element->next;
	unlock(0);
	return element;
}