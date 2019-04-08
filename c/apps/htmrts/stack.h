typedef int val_t;

typedef int top_t;
typedef _iodev_ptr_t top_ptr_t;

typedef _UNCACHED volatile struct element_t {
	volatile val_t val;
	_UNCACHED volatile struct element_t * next;
} element_t;