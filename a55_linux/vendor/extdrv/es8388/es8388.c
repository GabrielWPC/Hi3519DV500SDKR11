/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: es8388 driver.
 * Author: Hisilicon multimedia software group
 * Create: 2019-10-10
 */

#include <linux/kernel.h>
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

#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>

#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/irq.h>

#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <linux/notifier.h>
#include <linux/i2c.h>

#ifndef __LITEOS__
#include <linux/i2c-dev.h>
#endif
#ifdef __LITEOS__
#include <linux/cdev.h>
#include <linux/device.h>
#include "i2c_bsp.h"
#endif
#include "ot_osal.h"
#include "securec.h"
#include "es8388.h"

#ifdef OT_FPGA
#define ES8388_I2C_ADAPTER_ID   1
#else
#define ES8388_I2C_ADAPTER_ID   7
#endif

#define CHIP_NUM 1
#define DEV_NAME "es8388"
#define DEBUG_LEVEL 1
#define dprintk(level, fmt, args...)     \
    do {                                 \
        if (level < DEBUG_LEVEL) {       \
            printk(KERN_INFO "%s [%s , %d]: " fmt "\n", DEV_NAME, __FUNCTION__, __LINE__, ##args);   \
        }                                \
    } while (0)

unsigned int g_iic_device_addr[CHIP_NUM] = {0x20};

static struct i2c_board_info g_board_info = {
    I2C_BOARD_INFO("es8388", 0x20),
};

static struct i2c_client* g_es8388_client = NULL;

static unsigned int g_open_cnt = 0;
static unsigned int g_chip_count = 1;

static int es8388_device_init(unsigned int num);

#ifndef __LITEOS__
static unsigned char ot_i2c_read(unsigned char dev_addr, unsigned int reg_addr,
    unsigned int reg_addr_num, unsigned int data_byte_num)
{
    unsigned char tmp_buf0[4]; /* 4: buf len */
    unsigned char tmp_buf1[4]; /* 4: buf len */
    int ret;
    int ret_data = 0xFF;
    int idx = 0;
    static struct i2c_client client;
    static struct i2c_msg msg[2]; /* 2: msg len */

    if (g_es8388_client == NULL) {
        return -1;
    }

    ret = memcpy_s(&client, sizeof(client), g_es8388_client, sizeof(*g_es8388_client));
    if (ret != EOK) {
        return -1;
    }

    msg[0].addr = (dev_addr >> 1);
    msg[0].flags = client.flags & I2C_M_TEN;
    msg[0].len = reg_addr_num;
    msg[0].buf = tmp_buf0;

    /* reg_addr config */
    if (reg_addr_num == 1) {
        tmp_buf0[idx++] = reg_addr & 0xff;
    } else {
        tmp_buf0[idx++] = (reg_addr >> 8) & 0xff; /* 8: offset */
        tmp_buf0[idx++] = reg_addr & 0xff;
    }

    msg[1].addr = (dev_addr >> 1);
    msg[1].flags = client.flags & I2C_M_TEN;
    msg[1].flags |= I2C_M_RD;
    msg[1].len = data_byte_num;
    msg[1].buf = tmp_buf1;

    while (1) {
        ret = i2c_transfer(client.adapter, msg, 2); /* 2: len */
        if (ret == 2) { /* 2: return len */
            if (data_byte_num == 2) {  /* 2: data num */
                ret_data = tmp_buf1[1] | (tmp_buf1[0] << 8); /* 8: offset */
            } else {
                ret_data = tmp_buf1[0];
            }
            break;
        } else if ((ret == -EAGAIN) && (in_atomic() || irqs_disabled())) {
            continue;
        } else {
            dprintk(0, "i2c_transfer error, ret=%d.\n", ret);
            break;
        }
    }

    return ret_data;
}

static int ot_i2c_write(unsigned char dev_addr, unsigned int reg_addr,
    unsigned int reg_addr_num, unsigned int data, unsigned int data_byte_num)
{
    unsigned char tmp_buf[8]; /* 8: buf len */
    int ret;
    int idx = 0;
    static struct i2c_client client;

    if (g_es8388_client == NULL) {
        return -1;
    }

    ret = memcpy_s(&client, sizeof(client), g_es8388_client, sizeof(*g_es8388_client));
    if (ret != EOK) {
        return -1;
    }

    client.addr = (dev_addr >> 1);

    /* reg_addr config */
    if (reg_addr_num == 1) {
        tmp_buf[idx++] = reg_addr & 0xff;
    } else {
        tmp_buf[idx++] = (reg_addr >> 8) & 0xff; /* 8: offset */
        tmp_buf[idx++] = reg_addr & 0xff;
    }

    /* data config */
    if (data_byte_num == 1) {
        tmp_buf[idx++] = data;
    } else {
        tmp_buf[idx++] = (data >> 8) & 0xff; /* 8: offset */
        tmp_buf[idx++] = data & 0xff;
    }

    while (1) {
        ret = i2c_master_send(&client, tmp_buf, idx);
        if (ret == idx) {
            break;
        } else if ((ret == -EAGAIN) && (in_atomic() || irqs_disabled())) {
            continue;
        } else {
            dprintk(0, "i2c_master_send error, ret=%d.\n", ret);
            return ret;
        }
    }

    return 0;
}

unsigned char es8388_read(unsigned char dev_addr, unsigned char reg_addr)
{
    return ot_i2c_read(dev_addr, reg_addr, 1, 1);
}

int es8388_write(unsigned char dev_addr, unsigned char reg_addr, unsigned char data)
{
    return ot_i2c_write(dev_addr, reg_addr, 1, data, 1);
}
#else
int es8388_write(unsigned char dev_addr, unsigned char reg_addr, unsigned char value)
{
    return ot_test_i2c_write(dev_addr, reg_addr, 1, value, 1);
}

int es8388_read(unsigned char dev_addr, unsigned char reg_addr)
{
    return ot_test_i2c_read(dev_addr, reg_addr, 1, 1);
}
#endif

void es8388_reg_dump(unsigned int reg_num)
{
    unsigned int i;
    for (i = 0; i < reg_num; i++) {
        printk("reg%d =%x,", i, es8388_read(g_iic_device_addr[0], i));
        if ((i + 1) % 8 == 0) { /* 8: print num per line */
            printk("\n");
        }
    }
}

void es8388_codec_soft_reset(unsigned int chip_num)
{
    unsigned int i2c_dev;

    if (chip_num >= CHIP_NUM) {
        return;
    }

    i2c_dev = g_iic_device_addr[chip_num];

    es8388_write(i2c_dev, 0x02, 0xF3); /* stop STM and DLL, power down DAC&ADC vref. */
    es8388_write(i2c_dev, 0x08, 0x00); /* I2S salve, BCLKDIV auto. */
    es8388_write(i2c_dev, 0x2B, 0x80); /* ADC and DAC have the same LRCK. */
    es8388_write(i2c_dev, 0x00, 0x16);
    es8388_write(i2c_dev, 0x01, 0x72);
    es8388_write(i2c_dev, 0x03, 0xF0); /* Power off ADC and LIN/RIN input. */
    es8388_write(i2c_dev, 0x04, 0xC0); /* power off DAC and LOUT&ROUT. */
    es8388_write(i2c_dev, 0x07, 0x7C); /* VSEL normal */

    /* ADC SET */
    es8388_write(i2c_dev, 0x09, 0x00); /* Mic Boost PGA = 0dB. */
    es8388_write(i2c_dev, 0x0A, 0x50); /* LINSEL = LINPUT2, RINSEL = RINPUT2. */
    es8388_write(i2c_dev, 0x0B, 0x02); /* select LIN1 and RIN1 as differential pair */
    es8388_write(i2c_dev, 0x0C, 0x0C); /* I2S, 16bit, Ldata = LADC, Rdata= RADC. */
    es8388_write(i2c_dev, 0x0D, 0x02); /* MCLK/LRCK = 256. */
    es8388_write(i2c_dev, 0x10, 0x00); /* LADC VOL = 0dB. */
    es8388_write(i2c_dev, 0x11, 0x00); /* RADC VOL = 0dB. */

    /* ALC SET */
    es8388_write(i2c_dev, 0x12, 0x22); /* ALCSEL = off, PGA MAXGAIN = 17.5dB, MINGAIN = 0dB. */
    es8388_write(i2c_dev, 0x13, 0xA0); /* ALCLVL = -1.5dB, ALCHLD = 0ms. */
    es8388_write(i2c_dev, 0x14, 0x05); /* ALCDCY = 420ms, ALCATK = 3.328ms. */
    es8388_write(i2c_dev, 0x15, 0x06); /* ALCMODE = Normal. */
    es8388_write(i2c_dev, 0x16, 0x90); /* Noise gate = disable. */

    /* DAC SET */
    es8388_write(i2c_dev, 0x17, 0x18); /* I2S, 16bit, Ldata = LDAC, Rdata= RDAC. */
    es8388_write(i2c_dev, 0x18, 0x02); /* MCLK/LRCK = 256. */
    es8388_write(i2c_dev, 0x19, 0x02); /* DACMute = normal. */
    es8388_write(i2c_dev, 0x1A, 0x00); /* LDAC VOL = 0dB. */
    es8388_write(i2c_dev, 0x1B, 0x00); /* RDAC VOL = 0dB. */
    es8388_write(i2c_dev, 0x26, 0x00); /* LMIXSEL = LIN1, RMIXSEL = RIN1. */
    es8388_write(i2c_dev, 0x27, 0xB8); /* LIN signal to left mixer disable. */
    es8388_write(i2c_dev, 0x2A, 0xB8); /* RIN signal to right mixer disable. */
    es8388_write(i2c_dev, 0x2E, 0x1E); /* LOUT1VOL = 0dB. */
    es8388_write(i2c_dev, 0x2F, 0x1E); /* ROUT1VOL = 0dB. */
    es8388_write(i2c_dev, 0x30, 0x1E); /* LOUT2VOL = 0dB */
    es8388_write(i2c_dev, 0x31, 0x1E); /* ROUT2VOL = 0dB */

    es8388_write(i2c_dev, 0x02, 0x00); /* start STM and DLL, power on DAC&ADC vref. */
}

/*
 * device open. set counter
 */
static int es8388_open(struct inode *inode, struct file *file)
{
    (void)inode;
    (void)file;
    if (g_open_cnt == 0) {
        g_open_cnt++;
        return 0;
    }
    return -1;
}

/*
 * Close device, Do nothing!
 */
static int es8388_close(struct inode *inode, struct file *file)
{
    (void)inode;
    (void)file;
    g_open_cnt--;
    return 0;
}

static void es8388_set_adc_mclk_lrclk_ratio(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg13_ctrl reg13_ctrl;
    reg13_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG13_ADC_CONTROL5);
    reg13_ctrl.bit.adc_fs_ratio = audio_ctrl->clk_div;
    es8388_write(i2c_dev, ES8388_REG13_ADC_CONTROL5, reg13_ctrl.b8);
    return;
}

static void es8388_set_dac_mclk_lrclk_ratio(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg24_ctrl reg24_ctrl;
    reg24_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG24_DAC_CONTROL2);
    reg24_ctrl.bit.dac_fs_ratio = audio_ctrl->clk_div;
    es8388_write(i2c_dev, ES8388_REG24_DAC_CONTROL2, reg24_ctrl.b8);
    return;
}

static void es8388_set_mclk_bclk_ratio(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg8_ctrl reg8_ctrl;
    reg8_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG8_MASTER_MODE_CONTROL);
    reg8_ctrl.bit.bclk_div = audio_ctrl->clk_div;
    es8388_write(i2c_dev, ES8388_REG8_MASTER_MODE_CONTROL, reg8_ctrl.b8);
    return;
}

static void es8388_set_adc_polarity_and_offset(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg12_ctrl reg12_ctrl;
    reg12_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG12_ADC_CONTROL4);
    reg12_ctrl.bit.adc_lr_p = audio_ctrl->clk_polarity;
    es8388_write(i2c_dev, ES8388_REG12_ADC_CONTROL4, reg12_ctrl.b8);
    return;
}

static void es8388_set_dac_polarity_and_offset(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg23_ctrl reg23_ctrl;
    reg23_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG23_DAC_CONTROL1);
    reg23_ctrl.bit.dac_lr_p = audio_ctrl->clk_polarity;
    es8388_write(i2c_dev, ES8388_REG23_DAC_CONTROL1, reg23_ctrl.b8);
    return;
}

static void es8388_set_master_mode(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg8_ctrl reg8_ctrl;
    reg8_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG8_MASTER_MODE_CONTROL);
    reg8_ctrl.bit.msc = audio_ctrl->ctrl_mode;
    es8388_write(i2c_dev, ES8388_REG8_MASTER_MODE_CONTROL, reg8_ctrl.b8);
    return;
}

static void es8388_set_bclk_dir(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg8_ctrl reg8_ctrl;
    reg8_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG8_MASTER_MODE_CONTROL);
    reg8_ctrl.bit.bclk_inv = audio_ctrl->clk_polarity;
    es8388_write(i2c_dev, ES8388_REG8_MASTER_MODE_CONTROL, reg8_ctrl.b8);
    return;
}

static void es8388_set_adc_data_width(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg12_ctrl reg12_ctrl;
    reg12_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG12_ADC_CONTROL4);
    reg12_ctrl.bit.adc_wl = audio_ctrl->data_length;
    es8388_write(i2c_dev, ES8388_REG12_ADC_CONTROL4, reg12_ctrl.b8);
    return;
}

static void es8388_set_dac_data_width(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg23_ctrl reg23_ctrl;
    reg23_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG23_DAC_CONTROL1);
    reg23_ctrl.bit.dac_wl = audio_ctrl->data_length;
    es8388_write(i2c_dev, ES8388_REG23_DAC_CONTROL1, reg23_ctrl.b8);
    return;
}

static void es8388_set_adc_data_format(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg12_ctrl reg12_ctrl;
    reg12_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG12_ADC_CONTROL4);
    reg12_ctrl.bit.adc_format = audio_ctrl->data_format;
    es8388_write(i2c_dev, ES8388_REG12_ADC_CONTROL4, reg12_ctrl.b8);
    return;
}

static void es8388_set_dac_data_format(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg23_ctrl reg23_ctrl;
    reg23_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG23_DAC_CONTROL1);
    reg23_ctrl.bit.dac_format = audio_ctrl->data_format;
    es8388_write(i2c_dev, ES8388_REG23_DAC_CONTROL1, reg23_ctrl.b8);
    return;
}

static void es8388_set_left_input_select(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg10_ctrl reg10_ctrl;
    reg10_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG10_ADC_CONTROL2);
    reg10_ctrl.bit.lin_sel = audio_ctrl->audio_in_select;
    es8388_write(i2c_dev, ES8388_REG10_ADC_CONTROL2, reg10_ctrl.b8);
    return;
}

static void es8388_set_right_input_select(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg10_ctrl reg10_ctrl;
    reg10_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG10_ADC_CONTROL2);
    reg10_ctrl.bit.rin_sel = audio_ctrl->audio_in_select;
    es8388_write(i2c_dev, ES8388_REG10_ADC_CONTROL2, reg10_ctrl.b8);
    return;
}

static void es8388_set_left_input_gain(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg9_ctrl reg9_ctrl;
    reg9_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG9_ADC_CONTROL1);
    reg9_ctrl.bit.mic_amp_l = audio_ctrl->volume;
    es8388_write(i2c_dev, ES8388_REG9_ADC_CONTROL1, reg9_ctrl.b8);
    return;
}

static void es8388_set_right_input_gain(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg9_ctrl reg9_ctrl;
    reg9_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG9_ADC_CONTROL1);
    reg9_ctrl.bit.mic_amp_r = audio_ctrl->volume;
    es8388_write(i2c_dev, ES8388_REG9_ADC_CONTROL1, reg9_ctrl.b8);
    return;
}

static void es8388_set_left_adc_volume(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg16_ctrl reg16_ctrl;
    reg16_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG16_ADC_CONTROL8);
    reg16_ctrl.bit.ladc_vol = audio_ctrl->volume;
    es8388_write(i2c_dev, ES8388_REG16_ADC_CONTROL8, reg16_ctrl.b8);
    return;
}

static void es8388_set_right_adc_volume(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg17_ctrl reg17_ctrl;
    reg17_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG17_ADC_CONTROL9);
    reg17_ctrl.bit.radc_vol = audio_ctrl->volume;
    es8388_write(i2c_dev, ES8388_REG17_ADC_CONTROL9, reg17_ctrl.b8);
    return;
}

static void es8388_set_adc_mute(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg15_ctrl reg15_ctrl;
    reg15_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG15_ADC_CONTROL7);
    reg15_ctrl.bit.adc_mute = audio_ctrl->if_mute;
    es8388_write(i2c_dev, ES8388_REG15_ADC_CONTROL7, reg15_ctrl.b8);
    return;
}

static void es8388_set_left_dac_volume(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg26_ctrl reg26_ctrl;
    reg26_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG26_DAC_CONTROL4);
    reg26_ctrl.bit.ldac_vol = audio_ctrl->volume;
    es8388_write(i2c_dev, ES8388_REG26_DAC_CONTROL4, reg26_ctrl.b8);
    return;
}

static void es8388_set_right_dac_volume(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg27_ctrl reg27_ctrl;
    reg27_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG27_DAC_CONTROL5);
    reg27_ctrl.bit.rdac_vol = audio_ctrl->volume;
    es8388_write(i2c_dev, ES8388_REG27_DAC_CONTROL5, reg27_ctrl.b8);
    return;
}

static void es8388_set_left_output1_volume(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg46_ctrl reg46_ctrl;
    reg46_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG46_DAC_CONTROL24);
    reg46_ctrl.bit.lout1_vol = audio_ctrl->volume;
    es8388_write(i2c_dev, ES8388_REG46_DAC_CONTROL24, reg46_ctrl.b8);
    return;
}

static void es8388_set_left_output2_volume(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg48_ctrl reg48_ctrl;
    reg48_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG48_DAC_CONTROL26);
    reg48_ctrl.bit.lout2_vol = audio_ctrl->volume;
    es8388_write(i2c_dev, ES8388_REG48_DAC_CONTROL26, reg48_ctrl.b8);
    return;
}

static void es8388_set_right_output1_volume(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg47_ctrl reg47_ctrl;
    reg47_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG47_DAC_CONTROL25);
    reg47_ctrl.bit.rout1_vol = audio_ctrl->volume;
    es8388_write(i2c_dev, ES8388_REG47_DAC_CONTROL25, reg47_ctrl.b8);
    return;
}

static void es8388_set_right_output2_volume(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg49_ctrl reg49_ctrl;
    reg49_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG49_DAC_CONTROL27);
    reg49_ctrl.bit.rout2_vol = audio_ctrl->volume;
    es8388_write(i2c_dev, ES8388_REG49_DAC_CONTROL27, reg49_ctrl.b8);
    return;
}

static void es8388_set_dac_mute(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg25_ctrl reg25_ctrl;
    reg25_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG25_DAC_CONTROL3);
    reg25_ctrl.bit.dac_mute = audio_ctrl->if_mute;
    es8388_write(i2c_dev, ES8388_REG25_DAC_CONTROL3, reg25_ctrl.b8);
    return;
}

static void es8388_set_left_input_power(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg3_ctrl reg3_ctrl;
    reg3_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG3_ADC_POWER_MANAGEMENT);
    reg3_ctrl.bit.pdn_ainl = audio_ctrl->if_powerup;
    es8388_write(i2c_dev, ES8388_REG3_ADC_POWER_MANAGEMENT, reg3_ctrl.b8);
    return;
}

static void es8388_set_right_input_power(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg3_ctrl reg3_ctrl;
    reg3_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG3_ADC_POWER_MANAGEMENT);
    reg3_ctrl.bit.pdn_ainr = audio_ctrl->if_powerup;
    es8388_write(i2c_dev, ES8388_REG3_ADC_POWER_MANAGEMENT, reg3_ctrl.b8);
    return;
}

static void es8388_set_left_adc_power(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg3_ctrl reg3_ctrl;
    reg3_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG3_ADC_POWER_MANAGEMENT);
    reg3_ctrl.bit.pdn_adcl = audio_ctrl->if_powerup;
    es8388_write(i2c_dev, ES8388_REG3_ADC_POWER_MANAGEMENT, reg3_ctrl.b8);
    return;
}

static void es8388_set_right_adc_power(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg3_ctrl reg3_ctrl;
    reg3_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG3_ADC_POWER_MANAGEMENT);
    reg3_ctrl.bit.pdn_adcr = audio_ctrl->if_powerup;
    es8388_write(i2c_dev, ES8388_REG3_ADC_POWER_MANAGEMENT, reg3_ctrl.b8);
    return;
}

static void es8388_set_left_dac_power(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg4_ctrl reg4_ctrl;
    reg4_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG4_DAC_POWER_MANAGEMENT);
    reg4_ctrl.bit.pdn_dacl = audio_ctrl->if_powerup;
    es8388_write(i2c_dev, ES8388_REG4_DAC_POWER_MANAGEMENT, reg4_ctrl.b8);
    return;
}

static void es8388_set_right_dac_power(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg4_ctrl reg4_ctrl;
    reg4_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG4_DAC_POWER_MANAGEMENT);
    reg4_ctrl.bit.pdn_dacr = audio_ctrl->if_powerup;
    es8388_write(i2c_dev, ES8388_REG4_DAC_POWER_MANAGEMENT, reg4_ctrl.b8);
    return;
}

static void es8388_set_left_output1_enable(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg4_ctrl reg4_ctrl;
    reg4_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG4_DAC_POWER_MANAGEMENT);
    reg4_ctrl.bit.lout1 = audio_ctrl->if_powerup;
    es8388_write(i2c_dev, ES8388_REG4_DAC_POWER_MANAGEMENT, reg4_ctrl.b8);
    return;
}

static void es8388_set_right_output1_enable(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg4_ctrl reg4_ctrl;
    reg4_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG4_DAC_POWER_MANAGEMENT);
    reg4_ctrl.bit.rout1 = audio_ctrl->if_powerup;
    es8388_write(i2c_dev, ES8388_REG4_DAC_POWER_MANAGEMENT, reg4_ctrl.b8);
    return;
}

static void es8388_set_left_output2_enable(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg4_ctrl reg4_ctrl;
    reg4_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG4_DAC_POWER_MANAGEMENT);
    reg4_ctrl.bit.lout2 = audio_ctrl->if_powerup;
    es8388_write(i2c_dev, ES8388_REG4_DAC_POWER_MANAGEMENT, reg4_ctrl.b8);
    return;
}

static void es8388_set_right_output2_enable(unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    es8388_reg4_ctrl reg4_ctrl;
    reg4_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG4_DAC_POWER_MANAGEMENT);
    reg4_ctrl.bit.rout2 = audio_ctrl->if_powerup;
    es8388_write(i2c_dev, ES8388_REG4_DAC_POWER_MANAGEMENT, reg4_ctrl.b8);
    return;
}

static int ioctl_clk(unsigned int cmd, unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    switch (cmd) {
        case OT_ES8388_SET_ADC_MCLK_LRCLK_RATIO:
            es8388_set_adc_mclk_lrclk_ratio(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_DAC_MCLK_LRCLK_RATIO:
            es8388_set_dac_mclk_lrclk_ratio(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_MCLK_BCLK_RATIO:
            es8388_set_mclk_bclk_ratio(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_ADC_POLARITY_AND_OFFSET:
            es8388_set_adc_polarity_and_offset(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_DAC_POLARITY_AND_OFFSET:
            es8388_set_dac_polarity_and_offset(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_MASTER_MODE:
            es8388_set_master_mode(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_BCLK_DIR:
            es8388_set_bclk_dir(i2c_dev, audio_ctrl);
            break;

        default:
            return -1;
    }
    return 0;
}

static int ioctl_data(unsigned int cmd, unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    switch (cmd) {
        case OT_ES8388_SET_ADC_DATA_WIDTH:
            es8388_set_adc_data_width(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_DAC_DATA_WIDTH:
            es8388_set_dac_data_width(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_ADC_DATA_FORMAT:
            es8388_set_adc_data_format(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_DAC_DATA_FORMAT:
            es8388_set_dac_data_format(i2c_dev, audio_ctrl);
            break;

        default:
            return -1;
    }
    return 0;
}

static int ioctl_input_select(unsigned int cmd, unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    switch (cmd) {
        case OT_ES8388_SET_LEFT_INPUT_SELECT:
            es8388_set_left_input_select(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_RIGHT_INPUT_SELECT:
            es8388_set_right_input_select(i2c_dev, audio_ctrl);
            break;

        default:
            return -1;
    }
    return 0;
}

static int ioctl_input_volume(unsigned int cmd, unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    switch (cmd) {
        /* input volume control */
        case OT_ES8388_SET_LFET_INPUT_GAIN:
            es8388_set_left_input_gain(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_RIGHT_INPUT_GAIN:
            es8388_set_right_input_gain(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_LFET_ADC_VOLUME:
            es8388_set_left_adc_volume(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_RIGHT_ADC_VOLUME:
            es8388_set_right_adc_volume(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_ADC_MUTE:
            es8388_set_adc_mute(i2c_dev, audio_ctrl);
            break;

        default:
            return -1;
    }
    return 0;
}

static int ioctl_output_volume(unsigned int cmd, unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    switch (cmd) {
        case OT_ES8388_SET_LFET_DAC_VOLUME:
            es8388_set_left_dac_volume(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_RIGHT_DAC_VOLUME:
            es8388_set_right_dac_volume(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_LFET_OUTPUT1_VOLUME:
            es8388_set_left_output1_volume(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_LFET_OUTPUT2_VOLUME:
            es8388_set_left_output2_volume(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_RIGHT_OUTPUT1_VOLUME:
            es8388_set_right_output1_volume(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_RIGHT_OUTPUT2_VOLUME:
            es8388_set_right_output2_volume(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_DAC_MUTE:
            es8388_set_dac_mute(i2c_dev, audio_ctrl);
            break;

        default:
            return -1;
    }
    return 0;
}

static int ioctl_input_power(unsigned int cmd, unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    switch (cmd) {
        case OT_ES8388_SET_LFET_INPUT_POWER:
            es8388_set_left_input_power(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_RIGHT_INPUT_POWER:
            es8388_set_right_input_power(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_LFET_ADC_POWER:
            es8388_set_left_adc_power(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_RIGHT_ADC_POWER:
            es8388_set_right_adc_power(i2c_dev, audio_ctrl);
            break;

        default:
            return -1;
    }
    return 0;
}

static int ioctl_output_power(unsigned int cmd, unsigned int i2c_dev, const ot_es8388_audio_ctrl *audio_ctrl)
{
    switch (cmd) {
        case OT_ES8388_SET_LFET_DAC_POWER:
            es8388_set_left_dac_power(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_RIGHT_DAC_POWER:
            es8388_set_right_dac_power(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_LFET_OUTPUT1_ENABLE:
            es8388_set_left_output1_enable(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_RIGHT_OUTPUT1_ENABLE:
            es8388_set_right_output1_enable(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_LFET_OUTPUT2_ENABLE:
            es8388_set_left_output2_enable(i2c_dev, audio_ctrl);
            break;

        case OT_ES8388_SET_RIGHT_OUTPUT2_ENABLE:
            es8388_set_right_output2_enable(i2c_dev, audio_ctrl);
            break;

        default:
            return -1;
    }
    return 0;
}

static int ioctl_control(unsigned int cmd, unsigned int chip_num)
{
    switch (cmd) {
        case OT_ES8388_SOFT_RESET:
            es8388_codec_soft_reset(chip_num);
            break;

        case OT_ES8388_REG_DUMP:
            es8388_reg_dump(ES8388_REG_NUM);
            break;

        default:
            return -1;
    }
    return 0;
}

static int es8388_ioctl_adp_other(unsigned int cmd, unsigned int i2c_dev,
    const ot_es8388_audio_ctrl *audio_ctrl, unsigned int chip_num)
{
    switch (cmd) {
        /* output volume */
        case OT_ES8388_SET_LFET_DAC_VOLUME:
        case OT_ES8388_SET_RIGHT_DAC_VOLUME:
        case OT_ES8388_SET_LFET_OUTPUT1_VOLUME:
        case OT_ES8388_SET_LFET_OUTPUT2_VOLUME:
        case OT_ES8388_SET_RIGHT_OUTPUT1_VOLUME:
        case OT_ES8388_SET_RIGHT_OUTPUT2_VOLUME:
        case OT_ES8388_SET_DAC_MUTE:
            return ioctl_output_volume(cmd, i2c_dev, audio_ctrl);

        /* input power */
        case OT_ES8388_SET_LFET_INPUT_POWER:
        case OT_ES8388_SET_RIGHT_INPUT_POWER:
        case OT_ES8388_SET_LFET_ADC_POWER:
        case OT_ES8388_SET_RIGHT_ADC_POWER:
            return ioctl_input_power(cmd, i2c_dev, audio_ctrl);

        /* output power */
        case OT_ES8388_SET_LFET_DAC_POWER:
        case OT_ES8388_SET_RIGHT_DAC_POWER:
        case OT_ES8388_SET_LFET_OUTPUT1_ENABLE:
        case OT_ES8388_SET_RIGHT_OUTPUT1_ENABLE:
        case OT_ES8388_SET_LFET_OUTPUT2_ENABLE:
        case OT_ES8388_SET_RIGHT_OUTPUT2_ENABLE:
            return ioctl_output_power(cmd, i2c_dev, audio_ctrl);

        /* control */
        case OT_ES8388_SOFT_RESET:
        case OT_ES8388_REG_DUMP:
            return ioctl_control(cmd, chip_num);

        default:
            return -1;
    }
}

static int es8388_ioctl_adp(unsigned int cmd, unsigned int i2c_dev,
    const ot_es8388_audio_ctrl *audio_ctrl, unsigned int chip_num)
{
    switch (cmd) {
        /* clk */
        case OT_ES8388_SET_ADC_MCLK_LRCLK_RATIO:
        case OT_ES8388_SET_DAC_MCLK_LRCLK_RATIO:
        case OT_ES8388_SET_MCLK_BCLK_RATIO:
        case OT_ES8388_SET_ADC_POLARITY_AND_OFFSET:
        case OT_ES8388_SET_DAC_POLARITY_AND_OFFSET:
        case OT_ES8388_SET_MASTER_MODE:
        case OT_ES8388_SET_BCLK_DIR:
            return ioctl_clk(cmd, i2c_dev, audio_ctrl);

        /* data */
        case OT_ES8388_SET_ADC_DATA_WIDTH:
        case OT_ES8388_SET_DAC_DATA_WIDTH:
        case OT_ES8388_SET_ADC_DATA_FORMAT:
        case OT_ES8388_SET_DAC_DATA_FORMAT:
            return ioctl_data(cmd, i2c_dev, audio_ctrl);
        /* input select */
        case OT_ES8388_SET_LEFT_INPUT_SELECT:
        case OT_ES8388_SET_RIGHT_INPUT_SELECT:
            return ioctl_input_select(cmd, i2c_dev, audio_ctrl);

        /* input volume */
        case OT_ES8388_SET_LFET_INPUT_GAIN:
        case OT_ES8388_SET_RIGHT_INPUT_GAIN:
        case OT_ES8388_SET_LFET_ADC_VOLUME:
        case OT_ES8388_SET_RIGHT_ADC_VOLUME:
        case OT_ES8388_SET_ADC_MUTE:
            return ioctl_input_volume(cmd, i2c_dev, audio_ctrl);

        default:
            return es8388_ioctl_adp_other(cmd, i2c_dev, audio_ctrl, chip_num);
    }
}

#ifndef __LITEOS__
static long es8388_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static long es8388_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
    unsigned int __user *argp = (unsigned int __user *)(uintptr_t)arg;
    unsigned int chip_num;
    unsigned int i2c_dev;
    int ret;
    ot_es8388_audio_ctrl temp = {0};
    ot_es8388_audio_ctrl *audio_ctrl = NULL;

    (void)file;

    if (argp != NULL) {
        if (copy_from_user(&temp, argp, sizeof(ot_es8388_audio_ctrl))) {
            return -EFAULT;
        }
    }

    audio_ctrl = (ot_es8388_audio_ctrl*)(&temp);
    chip_num = audio_ctrl->chip_num;
    if (chip_num >= CHIP_NUM) {
        return -1;
    }
    i2c_dev = g_iic_device_addr[chip_num];

    ret = es8388_ioctl_adp(cmd, i2c_dev, audio_ctrl, chip_num);

    return ret;
}

#ifndef __LITEOS__
/*
 *  The various file operations we support.
 */
static struct file_operations g_es8388_fops = {
    .owner              = THIS_MODULE,
    .unlocked_ioctl     = es8388_ioctl,
    .open               = es8388_open,
    .release            = es8388_close,
#ifdef CONFIG_COMPAT
    .compat_ioctl       = es8388_ioctl
#endif
};
#else
static struct file_operations_vfs g_es8388_fops = {
    .ioctl  = es8388_ioctl,
    .open   = es8388_open,
    .close  = es8388_close,
};
#endif

#ifndef __LITEOS__
static struct miscdevice g_es8388_dev = {
    MISC_DYNAMIC_MINOR,
    DEV_NAME,
    &g_es8388_fops,
};

#ifndef MODULE
osal_setup_num_param(g_chip_count, g_chip_count);
#else
static int set_chip_count(const char *val, const struct kernel_param *kp)
{
    int ret, chip_cnt;

    (void)kp;

    ret = kstrtoint(val, 10, &chip_cnt); /* 10: number base */
    if (ret < 0) {
        return -EINVAL;
    }

    if (chip_cnt < 0 || chip_cnt > CHIP_NUM) {
        printk("chip_count = %d err.\n", chip_cnt);
        return -EINVAL;
    }

    g_chip_count = chip_cnt;

    return 0;
}

static struct kernel_param_ops g_es8388_para_ops = {
    .set = set_chip_count,
};
module_param_cb(g_chip_count, &g_es8388_para_ops, &g_chip_count, 0644);
#endif

MODULE_PARM_DESC(g_chip_count, "the es8388 num of our device using, default 1");

static int es8388_reboot(struct notifier_block *self, unsigned long data, void *pdata)
{
    unsigned int i;
    unsigned int i2c_dev;
    es8388_reg25_ctrl reg25_ctrl;

    (void)self;
    (void)data;
    (void)pdata;

    for (i = 0; i < g_chip_count; i++) {
        if (i >= CHIP_NUM) {
            break;
        }

        i2c_dev = g_iic_device_addr[i];

        /* mute output */
        reg25_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG25_DAC_CONTROL3);
        reg25_ctrl.bit.dac_mute = ES8388_MUTE_ENABLE;
        es8388_write(i2c_dev, ES8388_REG25_DAC_CONTROL3, reg25_ctrl.b8);
    }

    return 0;
}

static struct notifier_block g_es8388_reboot_notifier = {
    .notifier_call = es8388_reboot,
};
#endif

static int es8388_device_init(unsigned int num)
{
    /* init codec configs. */
    unsigned char temp;
    unsigned int i2c_dev;

    if (num >= CHIP_NUM) {
        return -1;
    }

    i2c_dev = g_iic_device_addr[num];

    /* test i2c if ok */
    temp = es8388_read(i2c_dev, 0x2);
    es8388_write(i2c_dev, 0x2, 0xaa);
    if (es8388_read(i2c_dev, 0x2) != 0xaa) {
        dprintk(0, "init es8388(%d) error", num);
        return -1;
    }
    es8388_write(i2c_dev, 0x2, temp);

    es8388_codec_soft_reset(num);

#ifndef __LITEOS__
    register_reboot_notifier(&g_es8388_reboot_notifier);
#endif
    return 0;
}

static int es8388_device_exit(unsigned int num)
{
    es8388_reg25_ctrl reg25_ctrl;
    unsigned int i2c_dev;

    if (num >= CHIP_NUM) {
        return -1;
    }

    i2c_dev = g_iic_device_addr[num];

    /* mute output */
    reg25_ctrl.b8 = es8388_read(i2c_dev, ES8388_REG25_DAC_CONTROL3);
    reg25_ctrl.bit.dac_mute = ES8388_MUTE_ENABLE;
    es8388_write(i2c_dev, ES8388_REG25_DAC_CONTROL3, reg25_ctrl.b8);

    return 0;
}

#ifdef __LITEOS__
struct i2c_client g_es8388_client_obj;
struct i2c_client *ot_es8388_i2c_client_init(void)
{
    g_es8388_client_obj.addr = g_iic_device_addr[0];
    g_es8388_client_obj.flags = 0x00;

    client_attach(&g_es8388_client_obj, ES8388_I2C_ADAPTER_ID);

    return &g_es8388_client_obj;
}
#endif

static int i2c_client_init(void)
{
#ifdef __LITEOS__
    g_es8388_client = ot_es8388_i2c_client_init();
#else
    struct i2c_adapter *i2c_adap = i2c_get_adapter(ES8388_I2C_ADAPTER_ID);
    g_es8388_client = i2c_new_client_device(i2c_adap, &g_board_info);
    i2c_put_adapter(i2c_adap);
#endif
    return 0;
}

static void i2c_client_exit(void)
{
    if (g_es8388_client != NULL) {
        i2c_unregister_device(g_es8388_client);
        g_es8388_client = NULL;
    }
}

static int __init es8388_init(void)
{
    unsigned int i, ret;

#ifndef __LITEOS__
    ret = misc_register(&g_es8388_dev);
#else
    ret = register_driver("/dev/es8388", &g_es8388_fops, 0666, 0);
#endif
    if (ret) {
        dprintk(0, "could not register es8388 device");
        return -1;
    }

    i2c_client_init();
    for (i = 0; i < g_chip_count; i++) {
        if (es8388_device_init(i) < 0) {
            goto init_fail;
        }
    }

    printk("load es8388.ko ....OK!\n");
    return 0;

init_fail:
    i2c_client_exit();
#ifndef __LITEOS__
    misc_deregister(&g_es8388_dev);
#else
    unregister_driver("/dev/es8388");
#endif
    printk("load es8388.ko ....failed!\n");
    return -1;
}

static void __exit es8388_exit(void)
{
    unsigned int i;

    for (i = 0; i < g_chip_count; i++) {
        es8388_device_exit(i);
    }
    i2c_client_exit();

#ifndef __LITEOS__
    unregister_reboot_notifier(&g_es8388_reboot_notifier);

    misc_deregister(&g_es8388_dev);
#else
    unregister_driver("/dev/es8388");
#endif

    printk("unload es8388.ko ....OK!\n");
}

module_init(es8388_init);
module_exit(es8388_exit);
MODULE_LICENSE("GPL");
