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

volatile _IODEV uint32_t * const SDCIO_BASE = (volatile _IODEV uint32_t *)0xf00d0000;

void sdcio_write(const uint32_t address, const uint32_t value);
uint32_t sdcio_read(const uint32_t address);

#endif
