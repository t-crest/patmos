/**
 * @file sdcdrv.h
 * @author Philipp Birkl
 * @date 2021-01-03
 * @brief Driver interface for the fast SD controller
 */
#ifndef SDCDRV_H
#define SDCDRV_H

struct mmc *sdcdrv_init(void);

#endif
