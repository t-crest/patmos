#include "caspm.h"
#include "stack.h"

#include <machine/patmos.h>

const int cnt_msk = 0xFF000000;
const int ref_msk = 0x00FFFFFF;

typedef _UNCACHED volatile struct element_t {
	volatile val_t val;
	_UNCACHED volatile struct element_t * next;
	volatile char cnt;
} element_t;

void push(top_t top, element_t * element)
{
	// Increment element cnt
	int newcnt = (++(element->cnt)) << 24;
	int oldtop;
	int newtop;
	do
	{
		oldtop = caspm_read(top);
		element->next = (element_t *)(oldtop & ref_msk);
		// Combine the element cnt and pointer to create the new top		
		newtop = newcnt | (int)element;
	}
	while(cas(top, oldtop, newtop) != oldtop);
}

element_t * pop(top_t top)
{
	int newtop;
	int oldtop;
	element_t * element;
	do
	{
		oldtop = caspm_read(top);
		if (!oldtop)
			return 0;
		// Get the old element reference from the top
		element = (element_t *)(oldtop & ref_msk);
		element_t * nextelement = element->next;
		if(!nextelement)
			newtop = 0;
		else
			newtop = (nextelement->cnt & cnt_msk) | ((int)nextelement & ref_msk);
	}
	while(cas(top, oldtop, newtop) != oldtop);
	return element;
}