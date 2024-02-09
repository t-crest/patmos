
/* Patmos API support for the C target of Lingua Franca. */

/*************
Copyright (c) 2024, The University of California at Berkeley.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************/

/**
 * Patmos API support for the C target of Lingua Franca.
 *
 * This is based on lf_nrf_support.h in icyphy/lf-buckler.
 *  
 * @author{Soroush Bateni <soroush@utdallas.edu>}
 * @author{Abhi Gundrala <gundralaa@berkeley.edu>}
 * @author{Shaokai Lin <shaokai@berkeley.edu>} 
 */

#ifndef LF_PATMOS_SUPPORT_H
#define LF_PATMOS_SUPPORT_H

#define LOG_LEVEL 0
// This embedded platform has no TTY suport
#define NO_TTY 

#include <stdint.h> // For fixed-width integral types
#include <time.h>   // For CLOCK_MONOTONIC
#include <stdbool.h>

#include <inttypes.h>  // Needed to define PRId64 and PRIu32
#define PRINTF_TIME "%" PRId64
#define PRINTF_MICROSTEP "%" PRIu32
#define PRINTF_TAG "(%" PRId64 ", %" PRIu32 ")"

// Keep track of physical actions being entered into the system
static volatile bool _lf_async_event = false;

// The underlying physical clock for Linux
#define _LF_CLOCK CLOCK_MONOTONIC

#endif // LF_PATMOS_SUPPORT_H