#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#ifdef DEBUG_SDC
#define DEBUG_PRINT(fmt, args...) printf("%s:%d: " fmt "\n", __FILE__, __LINE__, ##args)
#else
#define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

#endif
