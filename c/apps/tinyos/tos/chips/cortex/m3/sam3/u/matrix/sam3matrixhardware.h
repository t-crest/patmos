/**
 * Copyright (c) 2009 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the copyright holders nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Bus Matrix register definitions.
 *
 * @author Thomas Schmid
 */

#ifndef _SAM3UMATRIXHARDWARE_H
#define _SAM3UMATRIXHARDWARE_H

#include "matrixhardware.h"

/**
 * Bus Matrix Register definitions,  AT91 ARM Cortex-M3 based Microcontrollers
 * SAM3U Series, Preliminary, p. 339
 */
typedef struct matrix
{
    volatile matrix_mcfg_t mcfg0; // master configuration register 0
    volatile matrix_mcfg_t mcfg1; // master configuration register 1
    volatile matrix_mcfg_t mcfg2; // master configuration register 2
    volatile matrix_mcfg_t mcfg3; // master configuration register 3
    volatile matrix_mcfg_t mcfg4; // master configuration register 4
    uint32_t reserved0[10];
    volatile matrix_scfg_t scfg0; // slave confgiruation register 0
    volatile matrix_scfg_t scfg1; // slave confgiruation register 1
    volatile matrix_scfg_t scfg2; // slave confgiruation register 2
    volatile matrix_scfg_t scfg3; // slave confgiruation register 3
    volatile matrix_scfg_t scfg4; // slave confgiruation register 4
    volatile matrix_scfg_t scfg5; // slave confgiruation register 5
    volatile matrix_scfg_t scfg6; // slave confgiruation register 6
    volatile matrix_scfg_t scfg7; // slave confgiruation register 7
    volatile matrix_scfg_t scfg8; // slave confgiruation register 8
    volatile matrix_scfg_t scfg9; // slave confgiruation register 9
    uint32_t reserved1[5];
    volatile matrix_pras_t pras0; // priority register A for slave 0
    uint32_t reserved2;
    volatile matrix_pras_t pras1; // priority register A for slave 0
    uint32_t reserved3;
    volatile matrix_pras_t pras2; // priority register A for slave 0
    uint32_t reserved4;
    volatile matrix_pras_t pras3; // priority register A for slave 0
    uint32_t reserved5;
    volatile matrix_pras_t pras4; // priority register A for slave 0
    uint32_t reserved6;
    volatile matrix_pras_t pras5; // priority register A for slave 0
    uint32_t reserved7;
    volatile matrix_pras_t pras6; // priority register A for slave 0
    uint32_t reserved8;
    volatile matrix_pras_t pras7; // priority register A for slave 0
    uint32_t reserved9;
    volatile matrix_pras_t pras8; // priority register A for slave 0
    uint32_t reserved10;
    volatile matrix_pras_t pras9; // priority register A for slave 0
    uint32_t reserved11[12];
    volatile matrix_mrcr_t mrcr;  // master remap control register
} matrix_t;

/**
 * Memory mapping for the MATRIX
 */
volatile matrix_t* MATRIX = (volatile matrix_t*) 0x400E0200; // MATRIX Base Address

#endif // _SAM3UMATRIXHARDWARE_H
