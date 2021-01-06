#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "sdc_drv.h"
#include "sdc_mmc.h"

int main()
{
    printf("SD CARD TEST !!!\n");
    struct mmc *drv = sdcdrv_init();
    if (!drv)
    {
        printf("sdcdrv_init failed\n\r");
        return -1;
    }
    printf("sdcdrv_init success\n");

    drv->has_init = 0;
    int err = mmc_init(drv);
    if (err != 0 || drv->has_init == 0)
    {
        printf("mmc_init failed\n");
        return -1;
    }
    printf("mmc_init success\n\r");

    print_mmcinfo(drv);

    return 0;
}
