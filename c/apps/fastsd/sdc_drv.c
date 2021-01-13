#include <errno.h>
#include <machine/patmos.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "sdc_drv.h"
#include "sdc_io.h"
#include "sdc_mmc.h"

#include "sdc_debug.h"

struct sdcdrv
{
    int clk_freq;
};

static int sdcdrv_mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data);
static void sdcdrv_mmc_set_ios(struct mmc *mmc);
static int sdcdrv_mmc_init(struct mmc *mmc);
static void sdcdrv_set_clock(struct sdcdrv *dev, uint clock);
static void sdcdrv_mmc_set_buswidth(struct sdcdrv *dev, uint width);
static void sdcdrv_mmc_setup_data_xfer(struct sdcdrv *dev, struct mmc_cmd *cmd, struct mmc_data *data);
static int sdcdrv_mmc_finish(struct sdcdrv *dev, struct mmc_cmd *cmd);
static int sdcdrv_mmc_data_finish(struct sdcdrv *dev);

#define DEFAULT_CMD_TIMEOUT (0x7FFF)

#define SDC_CMD_INT_STATUS_CC 0x0001
#define SDC_CMD_INT_STATUS_EI 0x0002
#define SDC_CMD_INT_STATUS_CTE 0x0004
#define SDC_CMD_INT_STATUS_CCRC 0x0008
#define SDC_CMD_INT_STATUS_CIE 0x0010

#define SDC_DAT_INT_STATUS_TRS 0x01
#define SDC_DAT_INT_STATUS_CRC 0x02
#define SDC_DAT_INT_STATUS_OV 0x04

struct mmc *sdcdrv_init(void)
{
    struct mmc *mmc;
    struct sdcdrv *priv;

    unsigned int cpu_freq = get_cpu_freq();

    mmc = (struct mmc *)malloc(sizeof(struct mmc));
    if (!mmc)
        goto MMC_ALLOC;

    priv = (struct sdcdrv *)malloc(sizeof(struct sdcdrv));
    if (!priv)
        goto SDCDRV_ALLOC;

    priv->clk_freq = cpu_freq;

    sprintf(mmc->name, "fastsd");
    mmc->priv = priv;

    mmc->send_cmd = sdcdrv_mmc_send_cmd;
    mmc->set_ios = sdcdrv_mmc_set_ios;
    mmc->init = sdcdrv_mmc_init;
    mmc->getcd = NULL;
    // prescaler min 2 max 64
    mmc->f_max = cpu_freq / 2;
    mmc->f_min = cpu_freq / 6;
    mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
    /* TODO: ANALYZE MMC_MODE_HS | MMC_MODE_HS_52MHz */
    mmc->host_caps = MMC_MODE_4BIT;
    /* TODO: how big is our buffer ? */
    mmc->b_max = 256;
    return mmc;
SDCDRV_ALLOC:
    free(mmc);

MMC_ALLOC:
    return NULL;
}

static int sdcdrv_mmc_init(struct mmc *mmc)
{
    struct sdcdrv *priv = (struct sdcdrv *)mmc->priv;
    sdc_reg_write(R_CMD_TIMEOUT, DEFAULT_CMD_TIMEOUT);
    sdc_reg_write(R_CMD_EV_ENABLE, 0);
    sdc_reg_write(R_DAT_EV_ENABLE, 0);
    sdc_reg_write(R_CMD_EV_STATUS, 0);
    sdc_reg_write(R_DAT_EV_STATUS, 0);
    sdcdrv_set_clock(priv, priv->clk_freq / 2);
    return 0;
}

static void sdcdrv_set_clock(struct sdcdrv *dev, uint clock)
{
    int clk_div = dev->clk_freq / (2.0 * clock) - 1;

    DEBUG_PRINT("sdcdrv_set_clock %d, div %d\n\r", clock, clk_div);
    //software reset
    sdc_reg_write(R_RESET, 1);
    //set clock devider
    sdc_reg_write(R_CLK_DEVIDER, clk_div);
    //clear software reset
    sdc_reg_write(R_RESET, 0);
}

static void sdcdrv_mmc_set_ios(struct mmc *mmc)
{
    sdcdrv_mmc_set_buswidth(mmc->priv, mmc->bus_width);
    if (mmc->clock)
        sdcdrv_set_clock((struct sdcdrv *)mmc->priv, mmc->clock);
}

static void sdcdrv_mmc_set_buswidth(struct sdcdrv *dev, uint width)
{
    if (width == 4)
        sdc_reg_write(R_CONTROL, R_CONTROL_4BIT);
    else if (width == 1)
        sdc_reg_write(R_CONTROL, 0);
}

static int sdcdrv_mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
    struct sdcdrv *dev = (struct sdcdrv *)mmc->priv;

    int command = (cmd->cmdidx << 8);
    if (cmd->resp_type & MMC_RSP_PRESENT)
    {
        if (cmd->resp_type & MMC_RSP_136)
            command |= 2;
        else
        {
            command |= 1;
        }
    }
    if (cmd->resp_type & MMC_RSP_BUSY)
        command |= (1 << 2);
    if (cmd->resp_type & MMC_RSP_CRC)
        command |= (1 << 3);
    if (cmd->resp_type & MMC_RSP_OPCODE)
        command |= (1 << 4);

    if (data && ((data->flags & MMC_DATA_READ) || ((data->flags & MMC_DATA_WRITE))) && data->blocks)
    {
        if (data->flags & MMC_DATA_READ)
            command |= (1 << 5);
        if (data->flags & MMC_DATA_WRITE)
            command |= (1 << 6);
        sdcdrv_mmc_setup_data_xfer(dev, cmd, data);
    }

    DEBUG_PRINT("sdcdrv_mmc_send_cmd %04x\n\r", command);

    sdc_reg_write(R_COMMAND, command);
    sdc_reg_write(R_ARGUMENT, cmd->cmdarg);

    if (sdcdrv_mmc_finish(dev, cmd) < 0)
        return -1;
    if (data && data->blocks)
    {
        if (data->flags & MMC_DATA_READ)
        {
            int retcode = sdcdrv_mmc_data_finish(dev);
            // data should be in buffer
            for (int cur_block = 0; cur_block < data->blocks; cur_block++)
            {
                for (int cur_data = 0; cur_data < data->blocksize; cur_data += 1)
                {
                    uint32_t buffer = sdc_buffer_read(cur_block * data->blocksize + cur_data);
                    uint32_t *tmp = (uint32_t*)data->dest;
                    tmp[cur_block * data->blocksize + cur_data] = buffer;
                }
            }

            return retcode;
        }
        else
        {
            return sdcdrv_mmc_data_finish(dev);
        }
    }
    else
        return 0;
}

static void sdcdrv_mmc_setup_data_xfer(struct sdcdrv *dev, struct mmc_cmd *cmd, struct mmc_data *data)
{
    sdc_reg_write(R_BUFF_ADDR, 0);
    sdc_reg_write(R_BLOCK_SIZE, data->blocksize - 1);
    sdc_reg_write(R_BLOCK_COUNT, data->blocks - 1);
    if (data->flags & MMC_DATA_WRITE)
    {
        // prepare the buffer for writing
        for (int cur_block = 0; cur_block < data->blocks; cur_block++)
        {
            for (int cur_data = 0; cur_data < data->blocksize; cur_data += 1)
            {
                // XXX: Does this work?
                uint32_t aligned_addr = cur_block * data->blocksize + cur_data;
                uint32_t buff = (data->src[aligned_addr + 0]) << 24 | (data->src[aligned_addr + 1] << 16) | (data->src[aligned_addr + 2] << 8) | (data->src[aligned_addr + 3] << 0);
                sdc_buffer_write(aligned_addr, buff);
            }
        }
    }
}

static int sdcdrv_mmc_finish(struct sdcdrv *dev, struct mmc_cmd *cmd)
{
    int retval = 0;
    while (1)
    {
        int r2 = sdc_reg_read(R_CMD_EV_STATUS);
        DEBUG_PRINT("sdcdrv_mmc_finish: cmd %d, status %x\n", cmd->cmdidx, r2);
        if (r2 & SDC_CMD_INT_STATUS_EI)
        {
            //clear interrupts
            sdc_reg_write(R_CMD_EV_STATUS, 0);
            DEBUG_PRINT("sdcdrv_mmc_finish: cmd %d, status %x\n\r", cmd->cmdidx, r2);
            retval = -1;
            break;
        }
        else if (r2 & SDC_CMD_INT_STATUS_CC)
        {
            //clear interrupts
            sdc_reg_write(R_CMD_EV_STATUS, 0);
            //get response
            cmd->response[0] = sdc_reg_read(R_RESP1);
            DEBUG_PRINT("sdcdrv_mmc_finish:  %d response %x ", cmd->cmdidx, cmd->response[0]);
            if (cmd->resp_type & MMC_RSP_136)
            {
                cmd->response[1] = sdc_reg_read(R_RESP2);
                cmd->response[2] = sdc_reg_read(R_RESP3);
                cmd->response[3] = sdc_reg_read(R_RESP4);
                DEBUG_PRINT("%x %x %x", cmd->response[1], cmd->response[2], cmd->response[3]);
            }
            DEBUG_PRINT("sdcdrv_mmc_finish:  %d ok\n\r", cmd->cmdidx);
            retval = 0;

            break;
        }
        //else if (!(r2 & SDC_CMD_INT_STATUS_CIE)) {
        //	printf("sdcdrv_mmc_finish: cmd %d no exec %x\n", cmd->cmdidx, r2);
        //}
    }
    return retval;
}

static int sdcdrv_mmc_data_finish(struct sdcdrv *dev)
{
    int status;

    while ((status = sdc_reg_read(R_DAT_EV_STATUS)) == 0)
        ;
    sdc_reg_write(R_DAT_EV_STATUS, 0);

    if (status & SDC_DAT_INT_STATUS_TRS)
    {
        DEBUG_PRINT("sdcdrv_mmc_data_finish: ok\n\r");
        return 0;
    }
    else
    {
        DEBUG_PRINT("sdcdrv_mmc_data_finish: status %x\n\r", status);
        return -1;
    }
}