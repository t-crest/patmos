#include "caspm.h"
#include "stack.h"

#include <machine/patmos.h>

const int cnt_msk = 0xFF000000;
const int ref_msk = 0x00FFFFFF;

typedef _UNCACHED volatile struct node_t {
	volatile val_t val;
	_UNCACHED volatile struct node_t * next;
	volatile char cnt;
} node_t;

void push(head_t head, node_t * node)
{
	// Increment node cnt
	int newcnt = (++(node->cnt)) << 24;
	int oldhead;
	int newhead;
	do
	{
		oldhead = caspm_read(head);
		node->next = (node_t *)(oldhead & ref_msk);
		// Combine the node cnt and pointer to create the new head		
		newhead = newcnt | (int)node;
	}
	while(cas(head, oldhead, newhead) != oldhead);
}

node_t * pop(head_t head)
{
	int newhead;
	int oldhead;
	node_t * node;
	do
	{
		oldhead = caspm_read(head);
		if (!oldhead)
			return 0;
		// Get the old node reference from the head
		node = (node_t *)(oldhead & ref_msk);
		node_t * nextnode = node->next;
		if(!nextnode)
			newhead = 0;
		else
			newhead = (nextnode->cnt & cnt_msk) | ((int)nextnode & ref_msk);
	}
	while(cas(head, oldhead, newhead) != oldhead);
	return node;
}