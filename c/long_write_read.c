/*
    This is bootable program for testing cache and external memory by writing a long array and reading it back
    Author: Anthon V. Riber
    Copyright: DTU, BSD License
*/

#include "include/bootable.h"
#include "include/patio.h"
#include <machine/patmos.h>
#include <machine/spm.h>
#include <stdio.h>
//1048576
//524288

int send(char c)
{
    while ((UART_STATUS & 0x01) == 0)
        ;
    UART_DATA = c;
    return 0;
}

#define MEMO ((volatile _UNCACHED int *)0x00020000)
int main()
{
    global_mode();

    for (int i = 0; i < 16; i += 1)
    {
        *(MEMO + i) = i % 10;
    }

    for (int i = 0; i < 16; i += 1)
    {
        char c = '0' + *(MEMO + i);
        WRITECHAR(c);
        // if (*(MEMO + i) != i % 10)
        // {
        //     char c = '0' + *(MEMO + i);
        //     WRITECHAR(c);
        // }
    }
}