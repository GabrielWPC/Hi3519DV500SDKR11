/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: spi_dev.c
 * Author: ISP SW
 * Create: 2018-7-30
 * Version: Initial Draft
 */

#include "spi_dev.h"
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

#include "ot_common.h"
#include "securec.h"
#include "motionsensor.h"

#define REG_WIDTH 1

static td_u32 g_csn = 0;
static spinlock_t g_lock_rd;

#ifndef __LITEOS__
static td_u32 g_bus_num = 1;

#ifndef MODULE
osal_setup_num_param(g_bus_num, g_bus_num);
osal_setup_num_param(g_csn, g_csn);
#else
module_param(g_bus_num, uint, S_IRUGO);
MODULE_PARM_DESC(g_bus_num, "spi bus number");
module_param(g_csn, uint, S_IRUGO);
MODULE_PARM_DESC(g_csn, "chip select number");
#endif

struct spi_master *g_ot_master;

static td_s32 ssp_func_write(struct spi_device *ot_spi, td_u8 addr, const td_u8 *data, td_u32 cnt)
{
    struct spi_master *master = g_ot_master;
    static struct spi_transfer t;
    static struct spi_message msg;
    static td_u8 buf[4] = { 0 }; /* 4 byte buf */
    td_s16 ret;
    td_ulong flags;

    if (ot_spi == TD_NULL) {
        return -ENODEV;
    }

    ot_unused(cnt);

    /* check spi_message is or no finish */
    spin_lock_irqsave(&master->queue_lock, flags);

    if (msg.state != TD_NULL) {
        msensor_err_trace("msg.state not null!!\n");
        return -EFAULT;
    }

    spin_unlock_irqrestore(&master->queue_lock, flags);

    buf[0] = addr;
    buf[0] &= (~0x80);
    buf[1] = *data;

    t.tx_buf = buf;
    t.len = 2; /* length 2 */

    spi_message_init(&msg);
    spi_message_add_tail(&t, &msg);

    msg.state = &msg;
    ret = spi_sync(ot_spi, &msg);
    if (ret != 0) {
        msensor_err_trace(" spi_async() error(%d)!\n", ret);
        return -EAGAIN;
    }
    return ret;
}

static td_s32 ssp_func_read(struct spi_device *ot_spi, td_u8 addr, td_u8 *data, td_u32 cnt)
{
    struct spi_master *master = g_ot_master;
    static struct spi_transfer t;
    static struct spi_message msg;
    static td_u8 buf[4] = { 0 }; /* 4 byte buf */
    td_s16 ret;
    td_ulong flags;
    static td_u8 body[1024] = { 0 }; /* 1024 byte buf */

    if (ot_spi == TD_NULL) {
        return -ENODEV;
    }

    /* check spi_message is or no finish */
    spin_lock_irqsave(&master->queue_lock, flags);

    if (msg.state != TD_NULL) {
        msensor_err_trace("msg.state not null!!(%x)\n", addr);
        spin_unlock_irqrestore(&master->queue_lock, flags);
        return -EFAULT;
    }

    spin_unlock_irqrestore(&master->queue_lock, flags);

    buf[0] = addr;
    buf[0] |= 0x80;
    buf[1] = 0;

    t.tx_buf = buf;
    t.rx_buf = body;
    t.len = cnt + 1;

    spi_message_init(&msg);
    spi_message_add_tail(&t, &msg);
    msg.state = &msg;
    ret = spi_sync(ot_spi, &msg);
    if (ret != 0) {
        msensor_err_trace(" spi_async() error(%d)!\n", ret);
        return -EAGAIN;
    }

    (td_void)memcpy_s(data, cnt, body + 1, cnt);
    return ret;
}

td_s32 motionsersor_spi_write(struct spi_device *ot_spi, td_u8 addr, const td_u8 *data, td_u32 cnt)
{
    return ssp_func_write(ot_spi, addr, data, cnt);
}

td_s32 motionsersor_spi_read(struct spi_device *ot_spi, td_u8 addr, td_u8 *data, td_u32 cnt)
{
    td_s16 ret;

    ret = ssp_func_read(ot_spi, addr, data, cnt);
    return ret;
}

td_s32 motionsersor_spi_init(struct spi_device **ot_spi)
{
    td_s32 status = TD_SUCCESS;
    struct device *d = TD_NULL;
    td_char *spi_name = TD_NULL;
    td_s32 spi_name_len;

    g_ot_master = spi_busnum_to_master(g_bus_num);
    if (g_ot_master == TD_NULL) {
        status = -ENXIO;
        goto err0;
    }

    spi_name_len = strlen(dev_name(&g_ot_master->dev)) + 10; /* extend 10 */

    spi_name = osal_kmalloc(spi_name_len, OSAL_GFP_KERNEL);
    if (spi_name == TD_NULL) {
        status = -ENOMEM;
        goto err0;
    }

    (td_void)memset_s(spi_name, spi_name_len, 0, spi_name_len);
    if (sprintf_s(spi_name, spi_name_len, "%s.%u", dev_name(&g_ot_master->dev), g_csn) == TD_FAILURE) {
        status = TD_FAILURE;
        goto err1;
    }

    d = bus_find_device_by_name(&spi_bus_type, TD_NULL, spi_name);
    if (d == TD_NULL) {
        status = -ENXIO;
        goto err1;
    }
    *ot_spi = to_spi_device(d);

    if (*ot_spi == TD_NULL) {
        status = -ENXIO;
        goto err2;
    }

    spin_lock_init(&g_lock_rd);
err2:
    put_device(d);
err1:
    if (spi_name != TD_NULL) {
        osal_kfree(spi_name);
        spi_name = TD_NULL;
    }
err0:
    return status;
}

td_s32 motionsersor_spi_deinit(const struct spi_device *spi_device)
{
    ot_unused(spi_device);
    return TD_SUCCESS;
}
#else

static td_s32 ssp_func_read(td_u8 addr, td_u8 *data, td_u32 cnt, td_u32 spi_num)
{
    td_u8 buf[0x4] = { 0 };
    static td_u8 body[1024] = { 0 }; /* 1024 byte buf */
    static struct spi_ioc_transfer transfer[1];
    td_s32 retval;

    transfer[0].tx_buf = buf;
    transfer[0].rx_buf = body;
    transfer[0].len = REG_WIDTH + cnt;
    transfer[0].cs_change = 1;
    transfer[0].speed = 10000000; /* speed 10000000 */
    (td_void)memset_s(buf, sizeof(buf), 0, sizeof(buf));

    buf[0] = (addr & 0xff) | 0x80;
    buf[1] = 0x0;

    retval = spi_dev_set(spi_num, g_csn, &transfer[0]);

    if (memcpy_s(data, cnt, body + 1, cnt) != EOK) {
        return TD_FAILURE;
    }

    if (retval == transfer[0].len) {
        return TD_SUCCESS;
    }
    return TD_FAILURE;
}

static td_s32 ssp_func_write(td_u8 addr, td_u8 *data, td_u32 cnt, td_u32 spi_num)
{
    td_u8 buf[0x10];
    struct spi_ioc_transfer transfer[1];
    td_s32 retval;
    transfer[0].tx_buf = buf;
    transfer[0].rx_buf = buf;
    transfer[0].len = REG_WIDTH + cnt;
    transfer[0].cs_change = 1;
    transfer[0].speed = 10000000; /* speed 10000000 */
    (td_void)memset_s(buf, sizeof(buf), 0, sizeof(buf));

    buf[0] = (addr & 0xff) & (~0x80);

    buf[1] = *data;

    retval = spi_dev_set(spi_num, g_csn, &transfer[0]);
    if (retval == transfer[0].len) {
        return TD_SUCCESS;
    }
    return TD_FAILURE;
}

td_s32 motionsersor_spi_write(td_u8 addr, const td_u8 *data, td_u32 cnt, td_u32 spi_num)
{
    return ssp_func_write(addr, data, cnt, spi_num);
}

td_s32 motionsersor_spi_read(td_u8 addr, td_u8 *data, td_u32 cnt, td_u32 spi_num)
{
    td_s32 ret;
    td_ulong flags;

    spin_lock_irqsave(&g_lock_rd, flags);
    ret = ssp_func_read(addr, data, cnt, spi_num);
    spin_unlock_irqrestore(&g_lock_rd, flags);
    return ret;
}

td_s32 motionsersor_spi_init(td_void)
{
    spin_lock_init(&g_lock_rd);
    return TD_SUCCESS;
}

td_s32 motionsersor_spi_deinit(td_void)
{
    return TD_SUCCESS;
}

#endif
