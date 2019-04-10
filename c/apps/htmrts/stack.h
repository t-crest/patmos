typedef int val_t;

typedef _UNCACHED volatile struct element_t {
	_UNCACHED volatile struct element_t * next_ptr;
	val_t val;
} element_t;

typedef _IODEV volatile struct stack_t {
	element_t * top_ptr;
} stack_t;

void intialize_element(element_t * element_ptr, val_t val)
{
	element_ptr->next_ptr = NULL;
	element_ptr->val = val;
}

inline void _intialize(stack_t * stack_ptr)
{
	stack_ptr->top_ptr = NULL;
}

inline void _push(stack_t * stack_ptr, element_t * element_ptr)
{
	element_ptr->next_ptr = stack_ptr->top_ptr;		
	stack_ptr->top_ptr = element_ptr;
}

inline element_t * _pop(stack_t * stack_ptr)
{
	element_t * element_ptr = stack_ptr->top_ptr;
	if(element_ptr)
		stack_ptr->top_ptr = element_ptr->next_ptr;
	return element_ptr;
}
