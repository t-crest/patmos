#include "hardlock.h"
#define _CASPM_SUPRESS_LOCK_
#include "caspm.h"
#include "stack.h"

typedef _UNCACHED volatile struct node_t {
	volatile val_t val;
	_UNCACHED volatile struct node_t * next;
} node_t;

void push(head_t head, node_t * node)
{
	lock(0);
	int oldhead = caspm_read(head);
	node->next = (node_t *)oldhead;
	cas(head,oldhead,(int)node);
	unlock(0);
}

node_t * pop(head_t head)
{
	lock(0);
	node_t * node = (node_t *)caspm_read(head);
	cas(head,(int)node,(int)node->next);
	unlock(0);
	return node;
}