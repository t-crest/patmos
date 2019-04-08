#include "htmrts.h"
#include "stack.h"

void push(top_ptr_t top, element_t * element)
{
	do
	{
		element->next = (element_t *)*top;		
		*top = (top_t)element;
	}
	while(*HTMRTS_COMMIT != 0);
}

element_t * pop(top_ptr_t top)
{
	element_t * element;
	do
	{
		element = (element_t *)*top;
		if(element)
			*top = (top_t)element->next;
	}
	while(*HTMRTS_COMMIT != 0);
	return element;
}