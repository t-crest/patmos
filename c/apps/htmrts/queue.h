typedef int val_t;

typedef _IODEV volatile struct element_t {
	_IODEV volatile struct element_t * next_ptr;
	val_t val;
} element_t;

typedef _IODEV volatile struct queue_t {
	element_t * first_ptr;
	element_t * last_ptr;
} queue_t;

inline void _intialize(queue_t * queue_ptr)
{
	queue_ptr->first_ptr = NULL;
	queue_ptr->last_ptr = NULL;
}

inline void _initialize_element(element_t * element_ptr, val_t val)
{
	element_ptr->next_ptr = NULL;
	element_ptr->val = val;
}

inline void _enqueue(queue_t * queue_ptr, element_t * element_ptr)
{
	element_ptr->next_ptr = NULL;
	element_t * first_ptr = queue_ptr->first_ptr;
	if(first_ptr)
		first_ptr->next_ptr = element_ptr;
	else
		queue_ptr->last_ptr = element_ptr;
	queue_ptr->first_ptr = element_ptr;
}

inline element_t * _dequeue(queue_t * queue_ptr)
{
	element_t * last_ptr = queue_ptr->last_ptr;
	if(last_ptr) {
		element_t * next_ptr = last_ptr->next_ptr;
		if(!next_ptr)
			queue_ptr->first_ptr = NULL;
		queue_ptr->last_ptr = next_ptr;
	}
	return last_ptr;
}
