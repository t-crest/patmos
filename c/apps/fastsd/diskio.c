#include <stdlib.h>

#include "diskio.h"
#include "sdc_mmc.h"
#include "sdc_drv.h"
#include "sdc_debug.h"

static struct mmc *drv = NULL;

DSTATUS disk_status(BYTE pdrv)
{
    if (drv == NULL)
    {
        return STA_NOINIT;
    }

    if (!drv->has_init)
    {
        return STA_NOINIT;
    }

    return 0;
}

DSTATUS disk_initialize(BYTE pdrv)
{
    drv = sdcdrv_init();
    if (!drv)
    {
        DEBUG_PRINT("sdcdrv_init failed");
        return STA_NOINIT;
    }
    DEBUG_PRINT("sdcdrv_init success");

    drv->has_init = 0;
    int err = mmc_init(drv);

    if (err != 0 || drv->has_init == 0)
    {
        DEBUG_PRINT("mmc_init failed");
        free(drv->priv);
        free(drv);
        drv = NULL;
        return STA_NOINIT;
    }
    DEBUG_PRINT("mmc_init success");

    return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    mmc_bread(drv, sector, count, buff);
    // TODO: error checking
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
    mmc_bwrite(drv, sector, count, buff);
    // TODO: error checking
    return RES_OK;
}

DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff) {
    // TODO: wait until writing is finished
    return RES_OK;
}
