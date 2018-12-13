#include "hardlock.h"
#define _CASPM_SUPRESS_LOCK_
#include "caspm.h"
#include "stack.h"

typedef _UNCACHED volatile struct element_t {
	volatile val_t val;
	_UNCACHED volatile struct element_t * next;
} element_t;

void push(top_t top, element_t * element)
{
	lock(0);
	int oldtop = caspm_read(top);
	element->next = (element_t *)oldtop;
	cas(top,oldtop,(int)element);
	unlock(0);
}

element_t * pop(top_t top)
{
	lock(0);
	element_t * element = (element_t *)caspm_read(top);
	cas(top,(int)element,(int)element->next);
	unlock(0);
	return element;
}