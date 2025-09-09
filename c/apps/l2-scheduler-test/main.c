#include <machine/patmos.h>
#include <stdio.h>

int main() {
    volatile _IODEV int *sch_io_ptr = (volatile _IODEV int *) 0xf00b0000;
    int val1;
    int val2;

    val1 = *sch_io_ptr;

    printf("%d\n", val1);

    *sch_io_ptr = 60;

    val2 = *sch_io_ptr;

    printf("%d\n", val2);
}