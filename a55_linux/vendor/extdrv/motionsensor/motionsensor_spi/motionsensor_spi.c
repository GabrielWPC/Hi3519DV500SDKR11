/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: motionsensor_spi.c
 * Author: ISP SW
 * Create: 2018-7-30
 * Version: Initial Draft
 */

#include "motionsensor_spi.h"
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include "securec.h"

#ifndef __LITEOS__
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include "ot_common.h"
#include "ot_osal.h"

#define SSP_DBG_ERR                KERN_ALERT
#define SSP_DBG_INFO               KERN_DEBUG
#define ssp_trace(level, fmt, ...) printk(level fmt, ##__VA_ARGS__)

#ifndef IOMEM
#define IOMEM __iomem
#endif

#else

#ifndef IOMEM
#define IOMEM
#endif

#define SSP_DBG_ERR                "err"
#define SSP_DBG_INFO               "inf"
#define ssp_trace(level, fmt, ...) dprintf(fmt, ##__VA_ARGS__)
#endif

#define SPI_LIST_CMDLINE_LEN 64
#define SPI_NAME_MAX 8
#define SPI_NUM 3

#define SPI_DATAWIDTH_8 0
#define DEV_NAME        "ssp"
#define MAX_ISP_DEV_NUM 8
#define SSP_DEV_NUM     5

#define ssp_readw(addr, ret)    ((ret) = (*(volatile td_u32 *)(addr)))
#define ssp_writew(addr, value) ((*(volatile td_u32 *)(addr)) = (value))

#define ot_reg_read(addr, ret)    ((ret) = (*(volatile td_u32 *)(addr)))
#define ot_reg_write(addr, value) ((*(volatile td_u32 *)(addr)) = (value))

#define SSP_BASE 0x11070000
#define SSP_SIZE 0x10000  /* 64KB */

#define CRG_BASE       0x11010000
#define CRG_SIZE       0x10000
#define SSP_CRG_OFFSET 0x4480

#define IO1_BASE       0x0EFF0000
#define IO1_SIZE       0x1000
#define IO3_BASE       0x10260000
#define IO3_SIZE       0x1000

static td_void IOMEM *g_reg_ssp_base_va;
static td_void IOMEM *g_reg_crg_base_va;
static td_void IOMEM *g_reg_io1_base_va;
static td_void IOMEM *g_reg_io3_base_va;

#define io_address_verify(x) ((td_ulong)(td_uintptr_t)g_reg_ssp_base_va + ((x) - (SSP_BASE)))
#define SSP_CRG_ADDRESS      ((td_ulong)(td_uintptr_t)g_reg_crg_base_va + SSP_CRG_OFFSET)
#define io1_addr(offset)     ((td_ulong)(td_uintptr_t)g_reg_io1_base_va + (offset))
#define io3_addr(offset)     ((td_ulong)(td_uintptr_t)g_reg_io3_base_va + (offset))

/* SSP register definition . */
#define ssp_cr0(ssp_no)   io_address_verify(SSP_BASE + 0x00 + ((ssp_no) * 0x1000))
#define ssp_cr1(ssp_no)   io_address_verify(SSP_BASE + 0x04 + ((ssp_no) * 0x1000))
#define ssp_dr(ssp_no)    io_address_verify(SSP_BASE + 0x08 + ((ssp_no) * 0x1000))
#define ssp_sr(ssp_no)    io_address_verify(SSP_BASE + 0x0C + ((ssp_no) * 0x1000))
#define ssp_cpsr(ssp_no)  io_address_verify(SSP_BASE + 0x10 + ((ssp_no) * 0x1000))
#define ssp_imsc(ssp_no)  io_address_verify(SSP_BASE + 0x14 + ((ssp_no) * 0x1000))
#define ssp_ris(ssp_no)   io_address_verify(SSP_BASE + 0x18 + ((ssp_no) * 0x1000))
#define ssp_mis(ssp_no)   io_address_verify(SSP_BASE + 0x1C + ((ssp_no) * 0x1000))
#define ssp_icr(ssp_no)   io_address_verify(SSP_BASE + 0x20 + ((ssp_no) * 0x1000))
#define ssp_dmacr(ssp_no) io_address_verify(SSP_BASE + 0x24 + ((ssp_no) * 0x1000))

static spinlock_t g_ssp_lock[SSP_DEV_NUM];
#define ssp_spin_lock_init(ssp_no)     spin_lock_init(&g_ssp_lock[ssp_no])
#define ssp_spin_lock(ssp_no, flags)   spin_lock_irqsave(&g_ssp_lock[ssp_no], flags)
#define ssp_spin_unlock(ssp_no, flags) spin_unlock_irqrestore(&g_ssp_lock[ssp_no], flags)

static spinlock_t g_ssp_crg_lock;
#define ssp_crg_spin_lock_init()   spin_lock_init(&g_ssp_crg_lock)
#define ssp_crg_spin_lock(flags)   spin_lock_irqsave(&g_ssp_crg_lock, flags)
#define ssp_crg_spin_unlock(flags) spin_unlock_irqrestore(&g_ssp_crg_lock, flags)

// default: no spi mux pin
static td_char g_spi_list[SPI_LIST_CMDLINE_LEN] = "";
#ifndef MODULE
osal_setup_str_param(spi, g_spi_list);
#else
module_param_string(spi, g_spi_list, SPI_LIST_CMDLINE_LEN, 0600);
MODULE_PARM_DESC(spi, "spi=");
#endif

static td_u32 g_motionsensor_spi_num = 1;

static td_s32 g_spi_index[SPI_NUM] = {-1, -1, -1};

typedef enum {
    PIN_MUX_SPI_1,
    PIN_MUX_SPI_2,
    PIN_MUX_SPI_3,
    PIN_MUX_SPI_BUTT
} pin_mux_spi;

static td_void ot_ssp_clock_enable(td_u32 ssp_no)
{
    td_u32 ret = 0;
    td_ulong flags;

    ssp_crg_spin_lock(flags);

    ot_reg_read(SSP_CRG_ADDRESS, ret);
    ret |= 0x1 << (4 + ssp_no); /* ssp clock enable bit: 4 */
    ot_reg_write(SSP_CRG_ADDRESS, ret);

    ssp_crg_spin_unlock(flags);
    return;
}

static td_void ot_ssp_clock_disable(td_u32 ssp_no)
{
    td_u32 ret = 0;
    td_ulong flags;

    ssp_crg_spin_lock(flags);

    ot_reg_read(SSP_CRG_ADDRESS, ret);
    ret = ret & (~(0x1 << (4 + ssp_no))); /* ssp clock enable bit: 4 */
    ot_reg_write(SSP_CRG_ADDRESS, ret);

    ssp_crg_spin_unlock(flags);
    return;
}

static td_void ot_ssp_clock_reset(td_u32 ssp_no)
{
#ifdef MOTIONSENSOR_SPI_DEBUG
    td_u32 ret = 0;
    td_ulong flags;

    ssp_crg_spin_lock(flags);

    ot_reg_read(SSP_CRG_ADDRESS, ret);
    ret |= 0x1 << (0 + ssp_no); /* ssp clock reset bit: 0 */
    ot_reg_write(SSP_CRG_ADDRESS, ret);

    ssp_crg_spin_unlock(flags);
#endif
}

static td_void ot_ssp_clock_unreset(td_u32 ssp_no)
{
#ifdef MOTIONSENSOR_SPI_DEBUG
    td_u32 ret = 0;
    td_ulong flags;

    ssp_crg_spin_lock(flags);

    ot_reg_read(SSP_CRG_ADDRESS, ret);
    ret = ret & (~(0x1 << (0 + ssp_no))); /* ssp clock reset bit: 0 */
    ot_reg_write(SSP_CRG_ADDRESS, ret);

    ssp_crg_spin_unlock(flags);
#endif
}

/*
 * set SSP frame form routine.
 *
 * @param framemode: frame form
 * 00: Motorola SPI frame form.
 * when set the mode,need set SSPCLKOUT phase and SSPCLKOUT voltage level.
 * 01: TI synchronous serial frame form
 * 10: National Microwire frame form
 * 11: reserved
 * @param sphvalue: SSPCLKOUT phase (0/1)
 * @param sp0: SSPCLKOUT voltage level (0/1)
 * @param datavalue: data bit
 * 0000: reserved    0001: reserved    0010: reserved    0011: 4bit data
 * 0100: 5bit data   0101: 6bit data   0110:7bit data    0111: 8bit data
 * 1000: 9bit data   1001: 10bit data  1010:11bit data   1011: 12bit data
 * 1100: 13bit data  1101: 14bit data  1110:15bit data   1111: 16bit data
 *
 * @return value: 0--success; -1--error.
 */
static td_s32 ot_ssp_set_frameform(td_u32 ssp_no, td_u8 framemode, td_u8 spo, td_u8 sph, td_u8 datawidth)
{
    td_u32 ret = 0;

    ssp_readw(ssp_cr0(ssp_no), ret);
    if (framemode > 3) { /* frame form 3 */
        ssp_trace(SSP_DBG_ERR, "set frame parameter err.\n");
        return -1;
    }
    ret = (ret & 0xFFCF) | (framemode << 4); /* 4 bits */
    if ((ret & 0x30) == 0) {
        if (spo > 1) {
            ssp_trace(SSP_DBG_ERR, "set spo parameter err.\n");
            return -1;
        }
        if (sph > 1) {
            ssp_trace(SSP_DBG_ERR, "set sph parameter err.\n");
            return -1;
        }
        ret = (ret & 0xFF3F) | (sph << 7) | (spo << 6); /* sph 7 bits, spo 6 bits */
    }
    if ((datawidth > 16) || (datawidth < 4)) { /* datawidth between 4 bits and 16 bits */
        ssp_trace(SSP_DBG_ERR, "set datawidth parameter err.\n");
        return -1;
    }
    ret = (ret & 0xFFF0) | (datawidth - 1);
    ssp_writew(ssp_cr0(ssp_no), ret);
    return 0;
}

/*
 * set SSP serial clock rate routine.
 *
 * @param scr: scr value.(0-255,usually it is 0)
 * @param cpsdvsr: clock prescale divisor.(2-254 even)
 *
 * @return value: 0--success; -1--error.
 */
static td_s32 ot_ssp_set_serialclock(td_u32 ssp_no, td_u8 scr, td_u8 cpsdvsr)
{
    td_u32 ret = 0;

    ssp_readw(ssp_cr0(ssp_no), ret);
    ret = (ret & 0xFF) | (scr << 8); /* 8 bits */
    ssp_writew(ssp_cr0(ssp_no), ret);
    if ((cpsdvsr & 0x1)) {
        ssp_trace(SSP_DBG_ERR, "set cpsdvsr parameter err.\n");
        return -1;
    }
    ssp_writew(ssp_cpsr(ssp_no), cpsdvsr);
    return 0;
}

static td_s32 ot_ssp_big_end_set(td_u32 ssp_no)
{
    td_u32 ret = 0;

    ssp_readw(ssp_cr1(ssp_no), ret);
    ret = (ret & 0xFF) & 0xEF;
    ssp_writew(ssp_cr1(ssp_no), ret);
    return 0;
}

static td_s32 ot_ssp_alt_mode_set(td_u32 ssp_no, td_s32 enable)
{
    td_u32 ret = 0;

    ssp_readw(ssp_cr1(ssp_no), ret);
    if (enable == 0) {
        ret = ret & (~0x40);
    } else {
        ret = (ret & 0xFF) | 0x40;
    }
    ssp_writew(ssp_cr1(ssp_no), ret);
    return 0;
}

static td_u32 ot_ssp_is_fifo_busy(td_u32 ssp_no)
{
    td_u32 ret = 0;

    ssp_readw(ssp_sr(ssp_no), ret);
    return (ret & 0x10);
}

static td_u32 ot_ssp_is_fifo_empty(td_u32 ssp_no, td_s32 send)
{
    td_u32 ret = 0;

    ssp_readw(ssp_sr(ssp_no), ret);
    if (send != 0) {
        if ((ret & 0x1) == 0x1) { /* send fifo */
            return 0;
        } else {
            return 1;
        }
    } else {
        if ((ret & 0x4) == 0x4) { /* receive fifo */
            return 0;
        } else {
            return 1;
        }
    }
}

/* fsspclkout is fsspclk / (cpsdvsr * (1 + scr)) */
static td_s32 ot_ssp_init_cfg(td_u32 ssp_no)
{
    const td_u8 framemode = 0;
    const td_u8 spo = 1;
    const td_u8 sph = 1;

#if SPI_DATAWIDTH_8
    const td_u8 datawidth = 8;
#else
    const td_u8 datawidth = 16;
#endif

#ifdef OT_FPGA
    const td_u8 scr = 1;
    const td_u8 cpsdvsr = 2;
#else
    const td_u8 scr = 4; /* scr 4 */
    const td_u8 cpsdvsr = 2; /* cpsdvsr 2 */
#endif

    ot_ssp_set_frameform(ssp_no, framemode, spo, sph, datawidth);

    ot_ssp_set_serialclock(ssp_no, scr, cpsdvsr);
    ot_ssp_big_end_set(ssp_no);
    /* altasens mode, which CS won't be pull high between 16bit data transfer */
    ot_ssp_alt_mode_set(ssp_no, 0);
    return 0;
}

static td_void spi_enable(td_u32 ssp_no)
{
    td_u32 ret = 0;

    ot_ssp_clock_enable(ssp_no);
    ot_ssp_clock_unreset(ssp_no);
    /* little endian */
    ot_reg_read(ssp_cr1(ssp_no), ret);
    ret = (ret & 0xff) | 0x2;
    ot_reg_write(ssp_cr1(ssp_no), ret); /* 0x2 */
    ot_ssp_init_cfg(ssp_no);
    return;
}

static td_void spi_disable(td_u32 ssp_no)
{
    td_u32 ret = 0;

    ot_reg_read(ssp_cr1(ssp_no), ret);
    ret = (ret & 0xff) & (~0x2);
    ot_reg_write(ssp_cr1(ssp_no), ret); /* 0x0 */
    ot_ssp_clock_reset(ssp_no);
    ot_ssp_clock_disable(ssp_no);
    return;
}

td_u16 ot_motionsensor_ssp_read_alt(td_u32 ssp_no, td_u8 reg_addr, td_u8 *reg_data,
    td_u32 cnt, td_bool fifo_mode)
{
    td_u32 ret = 0;
    td_ulong flags;
#if SPI_DATAWIDTH_8
    td_u8 buf[2] = { 0 }; /* 2 buf */
#else
    td_u16 buf_16;
#endif
    td_u32 i;

    ssp_spin_lock(ssp_no, flags);

    spi_enable(ssp_no);

    for (i = 0; i < cnt; i++) {
#if SPI_DATAWIDTH_8
        if (fifo_mode != TD_FALSE) {
            buf[0] = reg_addr | 0x80;
            buf[1] = 0x0;
        } else {
            buf[0] = (reg_addr + i) | 0x80;
            buf[1] = 0x0;
        }
        ssp_writew(ssp_dr(ssp_no), buf[0]);
        ssp_writew(ssp_dr(ssp_no), buf[1]);
#else
        if (fifo_mode != TD_FALSE) {
            buf_16 = (reg_addr | 0x80) << 8;    /* 8 bit */
        } else {
            buf_16 = ((reg_addr + i) | 0x80) << 8; /* 8 bit */
        }
        ssp_writew(ssp_dr(ssp_no), buf_16);
#endif
        while (ot_ssp_is_fifo_busy(ssp_no) != 0) {}
        while (ot_ssp_is_fifo_empty(ssp_no, 1) != 0) {}
        while (ot_ssp_is_fifo_empty(ssp_no, 0) != 0) {}
        while (ot_ssp_is_fifo_empty(ssp_no, 0) == 0) {
            ssp_readw(ssp_dr(ssp_no), ret);
        }
        reg_data[i] = ret & 0xff;
    }

    spi_disable(ssp_no);
    ssp_spin_unlock(ssp_no, flags);
    return 0;
}

td_s32 ot_motionsensor_ssp_write_alt(td_u32 ssp_no, td_u8 reg_addr, const td_u8 *data)
{
    td_u32 ret;
    td_ulong flags;
#if SPI_DATAWIDTH_8
    td_u8 buf[2] = { 0 }; /* 2 buf */
#else
    td_u16 buf_16;
#endif
    ssp_spin_lock(ssp_no, flags);

    spi_enable(ssp_no);

#if SPI_DATAWIDTH_8
    buf[0] = reg_addr & (~0x80);
    buf[1] = *data;
    ssp_writew(ssp_dr(ssp_no), buf[0]);
    ssp_writew(ssp_dr(ssp_no), buf[1]);
#else
    buf_16 = (reg_addr & (~0x80)) << 8; /* 8 bit */
    buf_16 = buf_16 | *data;
    ssp_writew(ssp_dr(ssp_no), buf_16);
#endif

    while (ot_ssp_is_fifo_busy(ssp_no) != 0) {}
    while (ot_ssp_is_fifo_empty(ssp_no, 1) != 0) {}
    while (ot_ssp_is_fifo_empty(ssp_no, 0) != 0) {}
    while (ot_ssp_is_fifo_empty(ssp_no, 0) == 0) {
        ssp_readw(ssp_dr(ssp_no), ret);
    }
    ot_unused(ret);
    spi_disable(ssp_no);
    ssp_spin_unlock(ssp_no, flags);
    return 0;
}

td_s32 ot_motionsensor_ssp_get_num(td_void)
{
    return g_motionsensor_spi_num;
}

/* PIN_MUX_SPI_1_1V8 */
static td_void spi1_pin_muxing(td_void)
{
#ifndef OT_FPGA
    ot_reg_write(io1_addr(0x24), 0x1331); /* SPI1_SCLK */
    ot_reg_write(io1_addr(0x30), 0x1331); /* SPI1_CSN */
    ot_reg_write(io1_addr(0x2C), 0x1331); /* SPI1_SDI */
    ot_reg_write(io1_addr(0x28), 0x1331); /* SPI1_SDO */
#endif
}

/* PIN_MUX_SPI_3_GROUP3 */
static td_void spi3_pin_muxing(td_void)
{
#ifndef OT_FPGA
    ot_reg_write(io3_addr(0x88), 0x1203); /* SPI3_SCLK */
    ot_reg_write(io3_addr(0x78), 0x1203); /* SPI3_CSN */
    ot_reg_write(io3_addr(0x7C), 0x1203); /* SPI3_SDI */
    ot_reg_write(io3_addr(0x98), 0x1203); /* SPI3_SDO */
#endif
}

static td_void spi1_pin_muxing_reset(td_void)
{
#ifndef OT_FPGA
    ot_reg_write(io1_addr(0x24), 0x1300); /* SPI1_SCLK */
    ot_reg_write(io1_addr(0x30), 0x1300); /* SPI1_CSN */
    ot_reg_write(io1_addr(0x2C), 0x1300); /* SPI1_SDI */
    ot_reg_write(io1_addr(0x28), 0x1300); /* SPI1_SDO */
#endif
}

static td_void spi3_pin_muxing_reset(td_void)
{
#ifndef OT_FPGA
    ot_reg_write(io3_addr(0x88), 0x1202); /* SPI3_SCLK */
    ot_reg_write(io3_addr(0x78), 0x1204); /* SPI3_CSN */
    ot_reg_write(io3_addr(0x7C), 0x1202); /* SPI3_SDI */
    ot_reg_write(io3_addr(0x98), 0x1202); /* SPI3_SDO */
#endif
}

#ifdef __LITEOS__
static td_s32 ssp_open(struct file *file)
{
    return 0;
}
static td_s32 ssp_close(struct file *file)
{
    return 0;
}
const static struct file_operations_vfs g_ssp_fops = {
    .open = ssp_open,
    .close = ssp_close,
    .ioctl = ssp_ioctl
};

td_s32 ot_motionsensor_ssp_init(td_void)
{
    td_s32 ret;
    td_u32 ssp_no;

    g_reg_ssp_base_va = (td_void IOMEM *)IO_ADDRESS((unsigned long)SSP_BASE);
    if (g_reg_ssp_base_va == TD_NULL) {
        ssp_trace(SSP_DBG_ERR, "kernel: ioremap ssp base failed!\n");
        return -1;
    }

    g_reg_crg_base_va = (td_void IOMEM *)IO_ADDRESS((unsigned long)CRG_BASE);
    if (g_reg_crg_base_va == TD_NULL) {
        g_reg_ssp_base_va = TD_NULL;
        ssp_trace(SSP_DBG_ERR, "kernel: ioremap ssp crg failed!\n");
        return -1;
    }

    ret = register_driver("/dev/ssp", &g_ssp_fops, 0666, 0); /* 0666 permission */
    if (ret != TD_SUCCESS) {
        g_reg_ssp_base_va = TD_NULL;
        g_reg_crg_base_va = TD_NULL;
        ssp_trace(SSP_DBG_ERR, "register ssp device failed with %#x!\n", ret);
        return -1;
    }

    for (ssp_no = 0; ssp_no < SSP_DEV_NUM; ssp_no++) {
        ssp_spin_lock_init(ssp_no);
    }

    ssp_crg_spin_lock_init();

    ssp_trace(SSP_DBG_INFO, "kernel: ssp initial ok!\n");

    return 0;
}

td_void ot_motionsensor_ssp_exit(td_void)
{
    g_reg_ssp_base_va = TD_NULL;
    g_reg_crg_base_va = TD_NULL;
    unregister_driver("/dev/ssp");
}

#else

static struct file_operations g_ssp_fops = {
    .owner = THIS_MODULE,
};

static struct miscdevice g_ssp_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME,
    .fops = &g_ssp_fops,
};

static td_void spi_pin_mux(td_s32 spi_index)
{
    switch (spi_index) {
        case PIN_MUX_SPI_1:
            spi1_pin_muxing();
            g_motionsensor_spi_num = 1; // spi1
            break;
        case PIN_MUX_SPI_2:
            break;
        case PIN_MUX_SPI_3:
            spi3_pin_muxing();
            g_motionsensor_spi_num = 3; // spi3
            break;
        default:
            break;
    }
}

static td_void spi_pin_mux_reset(td_s32 spi_index)
{
    switch (spi_index) {
        case PIN_MUX_SPI_1:
            spi1_pin_muxing_reset();
            break;
        case PIN_MUX_SPI_2:
            break;
        case PIN_MUX_SPI_3:
            spi3_pin_muxing_reset();
            break;
        default:
            break;
    }
}

static td_s32 parse_spi_index(td_char *s, td_s32 s_len)
{
    td_char tmp[SPI_NAME_MAX] = {0};
    td_s32 i;
    td_char *line = NULL;
    td_s32 index = -1;

    if (s_len == 0) {
        ssp_trace(SSP_DBG_ERR, "no spi pin mux!\n");
        return index;
    }

    for (i = 0; i < SPI_NUM; i++) { /* max spi num 4 */
        (td_void)snprintf_s(tmp, sizeof(tmp), sizeof(tmp) - 1, "%d", i + 1);

        if (strncmp(tmp, s, SPI_NAME_MAX) == 0) {
            index = i;
            return index;
        }
    }

    printk("FUNC:%s line:%d  SPI prefix:[%s] is not supported !\n", __FUNCTION__, __LINE__, line);
    return index;
}

static td_void prase_spi(td_char *s)
{
    td_s32 index = parse_spi_index(s, strlen(s));
    if (index >= 0 && index < SPI_NUM) {
        spi_pin_mux(index);
        g_spi_index[index] = index;
    }
}

static td_void spi_pin_parse_mod(td_char *s, td_u32 str_len)
{
    td_char *line = NULL;
    td_char *argv[SPI_NAME_MAX] = {NULL};
    td_s32 i;

    if (str_len == 0) {
        ssp_trace(SSP_DBG_ERR, "name len is 0!\n");
        return;
    }

    line = strsep(&s, ":");
    while (line != NULL) {
        for (i = 0; (argv[i] = strsep(&line, ",")) != NULL; i++) {
            prase_spi(argv[i]);

            if (i == (ARRAY_SIZE(argv) - 1)) {
                break;
            }
        }
        line = strsep(&s, ":");
    }
}

static td_s32 ssp_ioremap_reg(td_void)
{
    td_s32 ret = TD_SUCCESS;

    g_reg_io1_base_va = ioremap((unsigned long)IO1_BASE, (unsigned long)(IO1_SIZE));
    if (g_reg_io1_base_va == TD_NULL) {
        ssp_trace(SSP_DBG_ERR, "kernel: ioremap ssp crg failed!\n");
        ret = -ENOMEM;
        goto err0;
    }

    g_reg_io3_base_va = ioremap((unsigned long)IO3_BASE, (unsigned long)(IO3_SIZE));
    if (g_reg_io3_base_va == TD_NULL) {
        ssp_trace(SSP_DBG_ERR, "kernel: ioremap ssp crg failed!\n");
        ret = -ENOMEM;
        goto err1;
    }
    return ret;

err1:
    iounmap((td_void *)g_reg_io1_base_va);
    g_reg_io1_base_va = TD_NULL;
err0:
    return ret;
}

/*
 * initializes SSP interface routine.
 * @return value:0--success.
 */
static td_s32 __init ot_motionsensor_ssp_init(td_void)
{
    td_s32 ret;
    td_u32 ssp_no;

    g_reg_ssp_base_va = ioremap((unsigned long)SSP_BASE, (unsigned long)(SSP_SIZE *SSP_DEV_NUM));
    if (g_reg_ssp_base_va == TD_NULL) {
        ssp_trace(SSP_DBG_ERR, "kernel: ioremap ssp base failed!\n");
        return -ENOMEM;
    }

    g_reg_crg_base_va = ioremap((unsigned long)CRG_BASE, (unsigned long)(CRG_SIZE));
    if (g_reg_crg_base_va == TD_NULL) {
        ssp_trace(SSP_DBG_ERR, "kernel: ioremap ssp crg failed!\n");
        ret = -ENOMEM;
        goto err0;
    }

    ret = ssp_ioremap_reg();
    if (ret != TD_SUCCESS) {
        goto err1;
    }

    spi_pin_parse_mod(g_spi_list, SPI_LIST_CMDLINE_LEN);

    ret = misc_register(&g_ssp_dev);
    if (ret != TD_SUCCESS) {
        ssp_trace(SSP_DBG_ERR, "kernel: register ssp_0 device failed!");
        ret = TD_FAILURE;
        goto err2;
    }

    for (ssp_no = 0; ssp_no < SSP_DEV_NUM; ssp_no++) {
        ssp_spin_lock_init(ssp_no);
    }

    ssp_crg_spin_lock_init();

    ssp_trace(SSP_DBG_INFO, "kernel: ssp initial ok!\n");
    printk(KERN_INFO "load ot_spi.ko ....OK!\n");
    return 0;

err2:
    iounmap((td_void *)g_reg_io3_base_va);
    g_reg_io3_base_va = TD_NULL;
    iounmap((td_void *)g_reg_io1_base_va);
    g_reg_io1_base_va = TD_NULL;
err1:
    iounmap((td_void *)g_reg_crg_base_va);
    g_reg_crg_base_va = TD_NULL;
err0:
    iounmap((td_void *)g_reg_ssp_base_va);
    g_reg_ssp_base_va = TD_NULL;
    return ret;
}

static td_void __exit ot_motionsensor_ssp_exit(td_void)
{
    td_s32 index;

    misc_deregister(&g_ssp_dev);

    for (index = 0; index < SPI_NUM; ++index) {
        if (g_spi_index[index] != -1) {
            spi_pin_mux_reset(index);
            g_spi_index[index] = -1;
        }
    }

    iounmap((td_void *)g_reg_io3_base_va);
    g_reg_io3_base_va = TD_NULL;

    iounmap((td_void *)g_reg_io1_base_va);
    g_reg_io1_base_va = TD_NULL;

    iounmap((td_void *)g_reg_crg_base_va);
    g_reg_crg_base_va = TD_NULL;

    iounmap((td_void *)g_reg_ssp_base_va);
    g_reg_ssp_base_va = TD_NULL;

    printk(KERN_INFO "unload ot_spi.ko ....OK!\n");
}

EXPORT_SYMBOL_GPL(ot_motionsensor_ssp_get_num);
EXPORT_SYMBOL_GPL(ot_motionsensor_ssp_read_alt);
EXPORT_SYMBOL_GPL(ot_motionsensor_ssp_write_alt);

module_init(ot_motionsensor_ssp_init);
module_exit(ot_motionsensor_ssp_exit);
MODULE_DESCRIPTION("ssp driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("otlicon");

#endif
