#include <machine/patmos.h>

void inline write(_iodev_ptr_t uart_base_ptr, char c) {
  while ((*uart_base_ptr & 0x01) == 0);
  *(uart_base_ptr+1) = c;
}

char inline read(_iodev_ptr_t uart_base_ptr) {
  while ((*uart_base_ptr & 0x02) == 0);
  return *(uart_base_ptr+1);
}

char inline has_data(_iodev_ptr_t uart_base_ptr) {
  return ((*uart_base_ptr & 0x02) != 0);
}

int main() {

	_iodev_ptr_t uart1_ptr = (_iodev_ptr_t) PATMOS_IO_UART;
	_iodev_ptr_t uart2_ptr = (_iodev_ptr_t) PATMOS_IO_UART2;
	_iodev_ptr_t uart3_ptr = (_iodev_ptr_t) PATMOS_IO_UART3;

	for (;;) {
    if(has_data(uart1_ptr))
      write(uart1_ptr,read(uart1_ptr));
    if(has_data(uart2_ptr))
      write(uart2_ptr,read(uart2_ptr));
    if(has_data(uart3_ptr))
      write(uart3_ptr,read(uart3_ptr));
	}
}
