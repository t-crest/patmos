/**
 * Simple ELF UART echo test.
 * 
 * Compile with:
 * 
 * patmos-clang -o test24.elf test24.c
 *
 */

#include <machine/uart.h>

int main(int argc, char** argv) {
    char c[8];
    int len;
    while ((len = uart_read(c, 8))) {
	uart_write(c, len);
    }
    return 0;
}
