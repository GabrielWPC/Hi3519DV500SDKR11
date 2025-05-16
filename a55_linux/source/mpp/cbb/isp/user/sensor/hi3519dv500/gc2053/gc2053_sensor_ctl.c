/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef OT_GPIO_I2C
#include "gpioi2c_ex.h"
#else
#include "ot_i2c.h"
#endif
#include "securec.h"

#include "gc2053_cmos.h"

#define I2C_DEV_FILE_NUM     16
#define I2C_BUF_NUM          8

static int g_fd[OT_ISP_MAX_PIPE_NUM] = {[0 ...(OT_ISP_MAX_PIPE_NUM - 1)] = -1};

int gc2053_i2c_init(ot_vi_pipe vi_pipe)
{
    if (g_fd[vi_pipe] >= 0) {
        return TD_SUCCESS;
    }
#ifdef OT_GPIO_I2C
    g_fd[vi_pipe] = open("/dev/gpioi2c_ex", O_RDONLY, S_IRUSR);
    if (g_fd[vi_pipe] < 0) {
        isp_err_trace("Open gpioi2c_ex error!\n");
        return TD_FAILURE;
    }
#else
    int ret;
    char dev_file[I2C_DEV_FILE_NUM] = {0};
    td_u8 dev_num;
    ot_isp_sns_commbus *gc2053businfo = TD_NULL;
    gc2053businfo = gc2053_get_bus_info(vi_pipe);
    dev_num = gc2053businfo->i2c_dev;
	printf("dev_num=%d\n",dev_num);
    (td_void)snprintf_s(dev_file, sizeof(dev_file), sizeof(dev_file) - 1, "/dev/i2c-%u", dev_num);

    g_fd[vi_pipe] = open(dev_file, O_RDWR, S_IRUSR | S_IWUSR);
    if (g_fd[vi_pipe] < 0) {
        isp_err_trace("Open /dev/ot_i2c_drv-%u error!\n", dev_num);
        return TD_FAILURE;
    }

    ret = ioctl(g_fd[vi_pipe], OT_I2C_SLAVE_FORCE, (gc2053_I2C_ADDR >> 1));
    if (ret < 0) {
        isp_err_trace("I2C_SLAVE_FORCE error!\n");
        close(g_fd[vi_pipe]);
        g_fd[vi_pipe] = -1;
        return ret;
    }
#endif

    return TD_SUCCESS;
}

int gc2053_i2c_exit(ot_vi_pipe vi_pipe)
{
    if (g_fd[vi_pipe] >= 0) {
        close(g_fd[vi_pipe]);
        g_fd[vi_pipe] = -1;
        return TD_SUCCESS;
    }
    return TD_FAILURE;
}

td_s32 gc2053_read_register(ot_vi_pipe vi_pipe, td_u32 addr)
{
    ot_unused(vi_pipe);
    ot_unused(addr);
    return TD_SUCCESS;
}

td_s32 gc2053_write_register(ot_vi_pipe vi_pipe, td_u32 addr, td_u32 data)
{
    if (g_fd[vi_pipe] < 0) {
        return TD_SUCCESS;
    }

#ifdef OT_GPIO_I2C
    i2c_data.dev_addr = gc2053_I2C_ADDR;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = gc2053_ADDR_BYTE;
    i2c_data.data = data;
    i2c_data.data_byte_num = gc2053_DATA_BYTE;

    ret = ioctl(g_fd[vi_pipe], GPIO_I2C_WRITE, &i2c_data);
    if (ret) {
        isp_err_trace("GPIO-I2C write failed!\n");
        return ret;
    }
#else
    td_u32 idx = 0;
    td_s32 ret;
    td_u8 buf[I2C_BUF_NUM];

    if (gc2053_ADDR_BYTE == 2) {  /* 2 byte */
        buf[idx] = (addr >> 8) & 0xff;  /* shift 8 */
        idx++;
        buf[idx] = addr & 0xff;
        idx++;
    } else {
		buf[idx] = addr & 0xff;
        idx++;	
    }

    if (gc2053_DATA_BYTE == 2) {  /* 2 byte */
    } else {
        buf[idx] = data & 0xff;
        idx++;
    }

    ret = write(g_fd[vi_pipe], buf, gc2053_ADDR_BYTE + gc2053_DATA_BYTE);
    if (ret < 0) {
        isp_err_trace("I2C_WRITE error!\n");
        return TD_FAILURE;
    }

#endif
    return TD_SUCCESS;
}

static void delay_ms(int ms)
{
    usleep(ms * 1000); /* 1ms: 1000us */
    return;
}

void gc2053_prog(ot_vi_pipe vi_pipe, const td_u32 *rom)
{
    ot_unused(vi_pipe);
    ot_unused(rom);
}

void gc2053_standby(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;
    ret += gc2053_write_register(vi_pipe, 0x3000, 0x01);  /* STANDBY */
    ret += gc2053_write_register(vi_pipe, 0x3002, 0x01);  /* XTMSTA */
    if (ret != TD_SUCCESS) {
        isp_err_trace("write register failed!\n");
    }
    return;
}

void gc2053_restart(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;
    ret += gc2053_write_register(vi_pipe, 0x3000, 0x00);  /* standby */
    delay_ms(20); /* 20ms */
    ret += gc2053_write_register(vi_pipe, 0x3002, 0x00);  /* master mode start */
    if (ret != TD_SUCCESS) {
        isp_err_trace("write register failed!\n");
    }
    return;
}

void gc2053_mirror_flip(ot_vi_pipe vi_pipe, ot_isp_sns_mirrorflip_type sns_mirror_flip)
{
    switch (sns_mirror_flip) {
        case ISP_SNS_NORMAL:
            gc2053_write_register(vi_pipe, 0x3030, 0x00);
            break;
        case ISP_SNS_MIRROR:
            gc2053_write_register(vi_pipe, 0x3030, 0x01);
            break;
        case ISP_SNS_FLIP:
            gc2053_write_register(vi_pipe, 0x3030, 0x02);
            break;
        case ISP_SNS_MIRROR_FLIP:
            gc2053_write_register(vi_pipe, 0x3030, 0x03);
            break;
        default:
            break;
    }
    return;
}

td_void gc2053_blc_clamp(ot_vi_pipe vi_pipe, ot_isp_sns_blc_clamp blc_clamp)
{
    td_s32 ret = TD_SUCCESS;

    gc2053_set_blc_clamp_value(vi_pipe, blc_clamp.blc_clamp_en);

    if (blc_clamp.blc_clamp_en == TD_TRUE) {
        ret += gc2053_write_register(vi_pipe, 0x300e, 0x01);
    } else {
        ret += gc2053_write_register(vi_pipe, 0x300e, 0x00);
    }

    if (ret != TD_SUCCESS) {
        isp_err_trace("write register failed!\n");
    }
    return;
}

void gc2053_comm_init(ot_vi_pipe vi_pipe);
void gc2053_linear_2m_init(ot_vi_pipe vi_pipe);

void gc2053_default_reg_init(ot_vi_pipe vi_pipe)
{
    td_u32 i;
    td_s32 ret = TD_SUCCESS;
    ot_isp_sns_state *pastgc2053 = TD_NULL;
    pastgc2053 = gc2053_get_ctx(vi_pipe);
    for (i = 0; i < pastgc2053->regs_info[0].reg_num; i++) {
        ret += gc2053_write_register(vi_pipe,
                                     pastgc2053->regs_info[0].i2c_data[i].reg_addr,
                                     pastgc2053->regs_info[0].i2c_data[i].data);
    }
    if (ret != TD_SUCCESS) {
        isp_err_trace("write register failed!\n");
    }
    return;
}

void gc2053_init(ot_vi_pipe vi_pipe)
{
    ot_wdr_mode wdr_mode;
    td_bool          init;
    td_s32 ret,i;
    ot_isp_sns_state *pastgc2053 = TD_NULL;
    pastgc2053 = gc2053_get_ctx(vi_pipe);
    init       = pastgc2053->init;
    wdr_mode   = pastgc2053->wdr_mode;

    ret = gc2053_i2c_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("i2c init failed!\n");
        return;
    }
    /* When sensor first init, config all registers */
    if (init == TD_FALSE) {
        if (OT_WDR_MODE_2To1_LINE == wdr_mode) {
        } else {
            gc2053_linear_2m_init(vi_pipe);
			for (i = 0; i < pastgc2053->regs_info[0].reg_num; i++) {
        ret += gc2053_write_register(vi_pipe,
                                     pastgc2053->regs_info[0].i2c_data[i].reg_addr,
                                     pastgc2053->regs_info[0].i2c_data[i].data);
    }
        }
    } else {
        /* When sensor switch mode(linear<->WDR or resolution), config different registers(if possible) */
        if (OT_WDR_MODE_2To1_LINE == wdr_mode) {
        } else {
            gc2053_linear_2m_init(vi_pipe);
			for (i = 0; i < pastgc2053->regs_info[0].reg_num; i++) {
        ret += gc2053_write_register(vi_pipe,
                                     pastgc2053->regs_info[0].i2c_data[i].reg_addr,
                                     pastgc2053->regs_info[0].i2c_data[i].data);
    }
        }
    }

    pastgc2053->init = TD_TRUE;
    return;
}

void gc2053_exit(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    ret = gc2053_i2c_exit(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("gc2053 exit failed!\n");
    }
    return;
}


void gc2053_linear_1080p30_init_write_register(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;
    ret +=  gc2053_write_register(vi_pipe, 0x3000, 0x01);    /* Standby mode */
    ret +=  gc2053_write_register(vi_pipe, 0x3002, 0x01);    /* Master mode stop */

    /* Mode register setting */
    ret +=  gc2053_write_register(vi_pipe, 0x3005, 0x01);
    ret +=  gc2053_write_register(vi_pipe, 0x3007, 0x00);
    ret +=  gc2053_write_register(vi_pipe, 0x3009, 0x02);    /* 60fps;0x00->120fps */
    ret +=  gc2053_write_register(vi_pipe, 0x300c, 0x00);
    ret +=  gc2053_write_register(vi_pipe, 0x3010, 0x21);
    ret +=  gc2053_write_register(vi_pipe, 0x3011, 0x0a);
    ret +=  gc2053_write_register(vi_pipe, 0x3014, 0x00);    /* gain */
    ret +=  gc2053_write_register(vi_pipe, 0x3018, 0x65);    /* VMAX */
    ret +=  gc2053_write_register(vi_pipe, 0x3019, 0x04);
    ret +=  gc2053_write_register(vi_pipe, 0x301c, 0x30);    /* HMAX; */
    ret +=  gc2053_write_register(vi_pipe, 0x301d, 0x11);    /* HMAX; */
    ret +=  gc2053_write_register(vi_pipe, 0x3020, 0x01);    /* SHS1 */
    ret +=  gc2053_write_register(vi_pipe, 0x3021, 0x00);    /* SHS1 */
    ret +=  gc2053_write_register(vi_pipe, 0x3022, 0x00);    /* SHS1 */
    ret +=  gc2053_write_register(vi_pipe, 0x3030, 0x0B);    /* RHS1 */
    ret +=  gc2053_write_register(vi_pipe, 0x3031, 0x00);    /* RHS1 */
    ret +=  gc2053_write_register(vi_pipe, 0x3032, 0x00);    /* RHS1 */
    ret +=  gc2053_write_register(vi_pipe, 0x3024, 0x00);    /* SHS2 */
    ret +=  gc2053_write_register(vi_pipe, 0x3025, 0x00);    /* SHS2 */
    ret +=  gc2053_write_register(vi_pipe, 0x3026, 0x00);    /* SHS2 */
    ret +=  gc2053_write_register(vi_pipe, 0x3045, 0x01);
    ret +=  gc2053_write_register(vi_pipe, 0x3046, 0x01);    /* MIPI */
    ret +=  gc2053_write_register(vi_pipe, 0x305c, 0x18);    /* 37.125MHz INCK Setting */
    ret +=  gc2053_write_register(vi_pipe, 0x305d, 0x03);
    ret +=  gc2053_write_register(vi_pipe, 0x305e, 0x20);
    ret +=  gc2053_write_register(vi_pipe, 0x305f, 0x01);
    ret +=  gc2053_write_register(vi_pipe, 0x309e, 0x4a);
    ret +=  gc2053_write_register(vi_pipe, 0x309f, 0x4a);
    ret +=  gc2053_write_register(vi_pipe, 0x3106, 0x00);
    ret +=  gc2053_write_register(vi_pipe, 0x311c, 0x0e);
    if (ret != TD_SUCCESS) {
        printf("write register failed!\n");
    }
    return;
}






void gc2053_linear_2m_init(ot_vi_pipe vi_pipe)
{
    gc2053_write_register(vi_pipe, 0xfe, 0x80);
    gc2053_write_register(vi_pipe, 0xfe, 0x80);
    gc2053_write_register(vi_pipe, 0xfe, 0x00);
    gc2053_write_register(vi_pipe, 0xf2, 0x00);
    gc2053_write_register(vi_pipe, 0xf3, 0x00);
    gc2053_write_register(vi_pipe, 0xf4, 0x36);
    gc2053_write_register(vi_pipe, 0xf5, 0xc0);
    gc2053_write_register(vi_pipe, 0xf6, 0x44);
    gc2053_write_register(vi_pipe, 0xf7, 0x01);
    gc2053_write_register(vi_pipe, 0xf8, 0x2c);
    gc2053_write_register(vi_pipe, 0xf9, 0x42);
    gc2053_write_register(vi_pipe, 0xfc, 0x8e);

    gc2053_write_register(vi_pipe, 0xfe, 0x00);
    gc2053_write_register(vi_pipe, 0x87, 0x18);
    gc2053_write_register(vi_pipe, 0xee, 0x30);
    gc2053_write_register(vi_pipe, 0xd0, 0xb7);
    gc2053_write_register(vi_pipe, 0x03, 0x04);
    gc2053_write_register(vi_pipe, 0x04, 0x60);
    gc2053_write_register(vi_pipe, 0x05, 0x04);
    gc2053_write_register(vi_pipe, 0x06, 0x4c);
    gc2053_write_register(vi_pipe, 0x07, 0x00);
    gc2053_write_register(vi_pipe, 0x08, 0x11);
    gc2053_write_register(vi_pipe, 0x0a, 0x02);
    gc2053_write_register(vi_pipe, 0x0c, 0x02);
    gc2053_write_register(vi_pipe, 0x0d, 0x04);
    gc2053_write_register(vi_pipe, 0x0e, 0x40);
    gc2053_write_register(vi_pipe,0xfe, 0x01);
    gc2053_write_register(vi_pipe,0x83, 0x01);
    gc2053_write_register(vi_pipe,0x87, 0x51);
    gc2053_write_register(vi_pipe,0xfe, 0x03);
    gc2053_write_register(vi_pipe,0x01, 0x27);
    gc2053_write_register(vi_pipe,0xfe, 0x00);
    gc2053_write_register(vi_pipe,0x3e, 0x91);
    gc2053_write_register(vi_pipe, 0x12, 0xe2);
    gc2053_write_register(vi_pipe, 0x13, 0x16);
    gc2053_write_register(vi_pipe, 0x19, 0x0a);
    gc2053_write_register(vi_pipe, 0x28, 0x0a);
    gc2053_write_register(vi_pipe, 0x2b, 0x04);
    gc2053_write_register(vi_pipe, 0x37, 0x03);
    gc2053_write_register(vi_pipe, 0x43, 0x07);
    gc2053_write_register(vi_pipe, 0x44, 0x40);
    gc2053_write_register(vi_pipe, 0x46, 0x0b);
    gc2053_write_register(vi_pipe, 0x4b, 0x20);
    gc2053_write_register(vi_pipe, 0x4e, 0x08);
    gc2053_write_register(vi_pipe, 0x55, 0x20);
    gc2053_write_register(vi_pipe, 0x77, 0x01);
    gc2053_write_register(vi_pipe, 0x78, 0x00);
    gc2053_write_register(vi_pipe, 0x7c, 0x93);
    gc2053_write_register(vi_pipe, 0x8d, 0x92);
    gc2053_write_register(vi_pipe, 0x90, 0x00);
    gc2053_write_register(vi_pipe, 0x41, 0x04);
    gc2053_write_register(vi_pipe, 0x42, 0x65);
    gc2053_write_register(vi_pipe, 0xce, 0x7c);
    gc2053_write_register(vi_pipe, 0xd2, 0x41);
    gc2053_write_register(vi_pipe, 0xd3, 0xdc);
    gc2053_write_register(vi_pipe, 0xe6, 0x50);
    gc2053_write_register(vi_pipe, 0xb6, 0xc0);
    gc2053_write_register(vi_pipe, 0xb0, 0x70);
    gc2053_write_register(vi_pipe, 0x26, 0x30);
    gc2053_write_register(vi_pipe, 0xfe, 0x01);
    gc2053_write_register(vi_pipe, 0x55, 0x07);
    //dither
    gc2053_write_register(vi_pipe, 0x58, 0x00); //default 0x80
    gc2053_write_register(vi_pipe,0x04, 0x00);
    gc2053_write_register(vi_pipe,0x94, 0x03);
    gc2053_write_register(vi_pipe,0x97, 0x07);
    gc2053_write_register(vi_pipe,0x98, 0x80);
    gc2053_write_register(vi_pipe, 0xfe, 0x04);
    gc2053_write_register(vi_pipe, 0x14, 0x78);
    gc2053_write_register(vi_pipe, 0x15, 0x78);
    gc2053_write_register(vi_pipe, 0x16, 0x78);
    gc2053_write_register(vi_pipe, 0x17, 0x78);
    gc2053_write_register(vi_pipe, 0xfe, 0x01);
    gc2053_write_register(vi_pipe, 0x9a, 0x06);
    gc2053_write_register(vi_pipe, 0xfe, 0x00);
    gc2053_write_register(vi_pipe, 0x7b, 0x2a);
    gc2053_write_register(vi_pipe, 0x23, 0x2d);
    gc2053_write_register(vi_pipe, 0xfe, 0x03);
    gc2053_write_register(vi_pipe, 0x02, 0x56);
    gc2053_write_register(vi_pipe, 0x03, 0xb6);
    gc2053_write_register(vi_pipe, 0x12, 0x80);
    gc2053_write_register(vi_pipe, 0x13, 0x07);
    gc2053_write_register(vi_pipe, 0x15, 0x12);
    gc2053_write_register(vi_pipe, 0xfe, 0x00);

    printf("===Galaxycore GC2053_1080P_30FPS_10BIT_LINEAR_Init_OK!===\n");
    return;
}









