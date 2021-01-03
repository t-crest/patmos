/**
 * @file sdcio.h
 * @author Philipp Birkl
 * @date 2021-01-03
 * @brief Lowlevel interface for the fast SD controller
 */
#ifndef SDCIO_H
#define SDCIO_H

#include <machine/patmos.h>
#include <stdint.h>

#if !defined PATMOS_IO_FASTSD || PATMOS_IO_FASTSD == 0
#error "Martin, du musst die Base Addresse `PATMOS_IO_FASTSD` im Makefile definieren :)"
#endif

#define SDCIO_BASE ((volatile _IODEV unsigned *) (PATMOS_IO_FASTSD + 0x0))

void sdcio_write(const uint32_t address, const uint32_t value);
uint32_t sdcio_read(const uint32_t address);

#endif
