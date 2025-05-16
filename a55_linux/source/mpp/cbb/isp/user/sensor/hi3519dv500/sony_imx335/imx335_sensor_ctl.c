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

#include "imx335_cmos.h"

#define I2C_DEV_FILE_NUM     16
#define I2C_BUF_NUM          8

static int g_fd[OT_ISP_MAX_PIPE_NUM] = {[0 ...(OT_ISP_MAX_PIPE_NUM - 1)] = -1};

int imx335_i2c_init(ot_vi_pipe vi_pipe)
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
    ot_isp_sns_commbus *imx335businfo = TD_NULL;
    imx335businfo = imx335_get_bus_info(vi_pipe);
    dev_num = imx335businfo->i2c_dev;
	printf("dev_num=%d\n",dev_num);
    (td_void)snprintf_s(dev_file, sizeof(dev_file), sizeof(dev_file) - 1, "/dev/i2c-%u", dev_num);

    g_fd[vi_pipe] = open(dev_file, O_RDWR, S_IRUSR | S_IWUSR);
    if (g_fd[vi_pipe] < 0) {
        isp_err_trace("Open /dev/ot_i2c_drv-%u error!\n", dev_num);
        return TD_FAILURE;
    }

    ret = ioctl(g_fd[vi_pipe], OT_I2C_SLAVE_FORCE, (imx335_I2C_ADDR >> 1));
    if (ret < 0) {
        isp_err_trace("I2C_SLAVE_FORCE error!\n");
        close(g_fd[vi_pipe]);
        g_fd[vi_pipe] = -1;
        return ret;
    }
#endif

    return TD_SUCCESS;
}

int imx335_i2c_exit(ot_vi_pipe vi_pipe)
{
    if (g_fd[vi_pipe] >= 0) {
        close(g_fd[vi_pipe]);
        g_fd[vi_pipe] = -1;
        return TD_SUCCESS;
    }
    return TD_FAILURE;
}

td_s32 imx335_read_register(ot_vi_pipe vi_pipe, td_u32 addr)
{
    ot_unused(vi_pipe);
    ot_unused(addr);
    return TD_SUCCESS;
}

td_s32 imx335_write_register(ot_vi_pipe vi_pipe, td_u32 addr, td_u32 data)
{
    if (g_fd[vi_pipe] < 0) {
        return TD_SUCCESS;
    }

#ifdef OT_GPIO_I2C
    i2c_data.dev_addr = imx335_I2C_ADDR;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = imx335_ADDR_BYTE;
    i2c_data.data = data;
    i2c_data.data_byte_num = imx335_DATA_BYTE;

    ret = ioctl(g_fd[vi_pipe], GPIO_I2C_WRITE, &i2c_data);
    if (ret) {
        isp_err_trace("GPIO-I2C write failed!\n");
        return ret;
    }
#else
    td_u32 idx = 0;
    td_s32 ret;
    td_u8 buf[I2C_BUF_NUM];

    if (imx335_ADDR_BYTE == 2) {  /* 2 byte */
        buf[idx] = (addr >> 8) & 0xff;  /* shift 8 */
        idx++;
        buf[idx] = addr & 0xff;
        idx++;
    } else {
    }

    if (imx335_DATA_BYTE == 2) {  /* 2 byte */
    } else {
        buf[idx] = data & 0xff;
        idx++;
    }

    ret = write(g_fd[vi_pipe], buf, imx335_ADDR_BYTE + imx335_DATA_BYTE);
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

void imx335_prog(ot_vi_pipe vi_pipe, const td_u32 *rom)
{
    ot_unused(vi_pipe);
    ot_unused(rom);
}

void imx335_standby(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;
    ret += imx335_write_register(vi_pipe, 0x3000, 0x01);  /* STANDBY */
    ret += imx335_write_register(vi_pipe, 0x3002, 0x01);  /* XTMSTA */
    if (ret != TD_SUCCESS) {
        isp_err_trace("write register failed!\n");
    }
    return;
}

void imx335_restart(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;
    ret += imx335_write_register(vi_pipe, 0x3000, 0x00);  /* standby */
    delay_ms(20); /* 20ms */
    ret += imx335_write_register(vi_pipe, 0x3002, 0x00);  /* master mode start */
    if (ret != TD_SUCCESS) {
        isp_err_trace("write register failed!\n");
    }
    return;
}

void imx335_mirror_flip(ot_vi_pipe vi_pipe, ot_isp_sns_mirrorflip_type sns_mirror_flip)
{
    switch (sns_mirror_flip) {
        case ISP_SNS_NORMAL:
            imx335_write_register(vi_pipe, 0x3030, 0x00);
            break;
        case ISP_SNS_MIRROR:
            imx335_write_register(vi_pipe, 0x3030, 0x01);
            break;
        case ISP_SNS_FLIP:
            imx335_write_register(vi_pipe, 0x3030, 0x02);
            break;
        case ISP_SNS_MIRROR_FLIP:
            imx335_write_register(vi_pipe, 0x3030, 0x03);
            break;
        default:
            break;
    }
    return;
}

td_void imx335_blc_clamp(ot_vi_pipe vi_pipe, ot_isp_sns_blc_clamp blc_clamp)
{
    td_s32 ret = TD_SUCCESS;

    imx335_set_blc_clamp_value(vi_pipe, blc_clamp.blc_clamp_en);

    if (blc_clamp.blc_clamp_en == TD_TRUE) {
        ret += imx335_write_register(vi_pipe, 0x300e, 0x01);
    } else {
        ret += imx335_write_register(vi_pipe, 0x300e, 0x00);
    }

    if (ret != TD_SUCCESS) {
        isp_err_trace("write register failed!\n");
    }
    return;
}

void imx335_comm_init(ot_vi_pipe vi_pipe);
void imx335_linear_2m_init(ot_vi_pipe vi_pipe);

void imx335_default_reg_init(ot_vi_pipe vi_pipe)
{
    td_u32 i;
    td_s32 ret = TD_SUCCESS;
    ot_isp_sns_state *pastimx335 = TD_NULL;
    pastimx335 = imx335_get_ctx(vi_pipe);
    for (i = 0; i < pastimx335->regs_info[0].reg_num; i++) {
        ret += imx335_write_register(vi_pipe,
                                     pastimx335->regs_info[0].i2c_data[i].reg_addr,
                                     pastimx335->regs_info[0].i2c_data[i].data);
    }
    if (ret != TD_SUCCESS) {
        isp_err_trace("write register failed!\n");
    }
    return;
}

void imx335_init(ot_vi_pipe vi_pipe)
{
    ot_wdr_mode wdr_mode;
    td_bool          init;
    td_s32 ret;
    ot_isp_sns_state *pastimx335 = TD_NULL;
    pastimx335 = imx335_get_ctx(vi_pipe);
    init       = pastimx335->init;
    wdr_mode   = pastimx335->wdr_mode;

    ret = imx335_i2c_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("i2c init failed!\n");
        return;
    }
    /* When sensor first init, config all registers */
    if (init == TD_FALSE) {
        if (OT_WDR_MODE_2To1_LINE == wdr_mode) {
        } else {
            imx335_linear_2m_init(vi_pipe);
        }
    } else {
        /* When sensor switch mode(linear<->WDR or resolution), config different registers(if possible) */
        if (OT_WDR_MODE_2To1_LINE == wdr_mode) {
        } else {
            imx335_linear_2m_init(vi_pipe);
        }
    }

    pastimx335->init = TD_TRUE;
    return;
}

void imx335_exit(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    ret = imx335_i2c_exit(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("imx335 exit failed!\n");
    }
    return;
}


void imx335_linear_1080p30_init_write_register(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;
    ret +=  imx335_write_register(vi_pipe, 0x3000, 0x01);    /* Standby mode */
    ret +=  imx335_write_register(vi_pipe, 0x3002, 0x01);    /* Master mode stop */

    /* Mode register setting */
    ret +=  imx335_write_register(vi_pipe, 0x3005, 0x01);
    ret +=  imx335_write_register(vi_pipe, 0x3007, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x3009, 0x02);    /* 60fps;0x00->120fps */
    ret +=  imx335_write_register(vi_pipe, 0x300c, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x3010, 0x21);
    ret +=  imx335_write_register(vi_pipe, 0x3011, 0x0a);
    ret +=  imx335_write_register(vi_pipe, 0x3014, 0x00);    /* gain */
    ret +=  imx335_write_register(vi_pipe, 0x3018, 0x65);    /* VMAX */
    ret +=  imx335_write_register(vi_pipe, 0x3019, 0x04);
    ret +=  imx335_write_register(vi_pipe, 0x301c, 0x30);    /* HMAX; */
    ret +=  imx335_write_register(vi_pipe, 0x301d, 0x11);    /* HMAX; */
    ret +=  imx335_write_register(vi_pipe, 0x3020, 0x01);    /* SHS1 */
    ret +=  imx335_write_register(vi_pipe, 0x3021, 0x00);    /* SHS1 */
    ret +=  imx335_write_register(vi_pipe, 0x3022, 0x00);    /* SHS1 */
    ret +=  imx335_write_register(vi_pipe, 0x3030, 0x0B);    /* RHS1 */
    ret +=  imx335_write_register(vi_pipe, 0x3031, 0x00);    /* RHS1 */
    ret +=  imx335_write_register(vi_pipe, 0x3032, 0x00);    /* RHS1 */
    ret +=  imx335_write_register(vi_pipe, 0x3024, 0x00);    /* SHS2 */
    ret +=  imx335_write_register(vi_pipe, 0x3025, 0x00);    /* SHS2 */
    ret +=  imx335_write_register(vi_pipe, 0x3026, 0x00);    /* SHS2 */
    ret +=  imx335_write_register(vi_pipe, 0x3045, 0x01);
    ret +=  imx335_write_register(vi_pipe, 0x3046, 0x01);    /* MIPI */
    ret +=  imx335_write_register(vi_pipe, 0x305c, 0x18);    /* 37.125MHz INCK Setting */
    ret +=  imx335_write_register(vi_pipe, 0x305d, 0x03);
    ret +=  imx335_write_register(vi_pipe, 0x305e, 0x20);
    ret +=  imx335_write_register(vi_pipe, 0x305f, 0x01);
    ret +=  imx335_write_register(vi_pipe, 0x309e, 0x4a);
    ret +=  imx335_write_register(vi_pipe, 0x309f, 0x4a);
    ret +=  imx335_write_register(vi_pipe, 0x3106, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x311c, 0x0e);
    if (ret != TD_SUCCESS) {
        printf("write register failed!\n");
    }
    return;
}






void imx335_linear_2m_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;
    ret +=  imx335_write_register(vi_pipe, 0x3000, 0x01); /* standby */
    ret +=  imx335_write_register(vi_pipe, 0x3001, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x3002, 0x01);
    ret +=  imx335_write_register(vi_pipe, 0x3003, 0x00);

    ret +=  imx335_write_register(vi_pipe, 0x300C, 0x5B);
    ret +=  imx335_write_register(vi_pipe, 0x300D, 0x40);
    ret +=  imx335_write_register(vi_pipe, 0x3030, 0x94); /* VMAX_LOW */
    ret +=  imx335_write_register(vi_pipe, 0x3031, 0x11); /* VMAX_MIDDLE */
    ret +=  imx335_write_register(vi_pipe, 0x3032, 0x00); /* VMAX_HIGH */

    ret +=  imx335_write_register(vi_pipe, 0x3034, 0x26); /* HMAX */
    ret +=  imx335_write_register(vi_pipe, 0x3035, 0x02);

    ret +=  imx335_write_register(vi_pipe, 0x3058, 0x09); /* SHR0 */
    ret +=  imx335_write_register(vi_pipe, 0x3059, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x305A, 0x00);

    ret +=  imx335_write_register(vi_pipe, 0x30E8, 0x00); /* Gain */
    ret +=  imx335_write_register(vi_pipe, 0x30E9, 0x00);

    ret +=  imx335_write_register(vi_pipe, 0x315A, 0x02);
    ret +=  imx335_write_register(vi_pipe, 0x316A, 0x7E);

    ret +=  imx335_write_register(vi_pipe, 0x319D, 0x01);
    ret +=  imx335_write_register(vi_pipe, 0x319E, 0x01);

    ret +=  imx335_write_register(vi_pipe, 0x3288, 0x21);
    ret +=  imx335_write_register(vi_pipe, 0x328A, 0x02);

    ret +=  imx335_write_register(vi_pipe, 0x3414, 0x05);
    ret +=  imx335_write_register(vi_pipe, 0x3416, 0x18);

    ret +=  imx335_write_register(vi_pipe, 0x341C, 0x47);
    ret +=  imx335_write_register(vi_pipe, 0x341D, 0x00);

    ret +=  imx335_write_register(vi_pipe, 0x3648, 0x01);
    ret +=  imx335_write_register(vi_pipe, 0x364A, 0x04);
    ret +=  imx335_write_register(vi_pipe, 0x364C, 0x04);

    ret +=  imx335_write_register(vi_pipe, 0x3678, 0x01);
    ret +=  imx335_write_register(vi_pipe, 0x367C, 0x31);
    ret +=  imx335_write_register(vi_pipe, 0x367E, 0x31);

    ret +=  imx335_write_register(vi_pipe, 0x3706, 0x10);
    ret +=  imx335_write_register(vi_pipe, 0x3708, 0x03);

    ret +=  imx335_write_register(vi_pipe, 0x3714, 0x02);
    ret +=  imx335_write_register(vi_pipe, 0x3715, 0x02);
    ret +=  imx335_write_register(vi_pipe, 0x3716, 0x01);
    ret +=  imx335_write_register(vi_pipe, 0x3717, 0x03);
    ret +=  imx335_write_register(vi_pipe, 0x371C, 0x3D);
    ret +=  imx335_write_register(vi_pipe, 0x371D, 0x3F);

    ret +=  imx335_write_register(vi_pipe, 0x372C, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x372D, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x372E, 0x46);
    ret +=  imx335_write_register(vi_pipe, 0x372F, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x3730, 0x89);
    ret +=  imx335_write_register(vi_pipe, 0x3731, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x3732, 0x08);
    ret +=  imx335_write_register(vi_pipe, 0x3733, 0x01);
    ret +=  imx335_write_register(vi_pipe, 0x3734, 0xFE);
    ret +=  imx335_write_register(vi_pipe, 0x3735, 0x05);

    ret +=  imx335_write_register(vi_pipe, 0x3740, 0x02);

    ret +=  imx335_write_register(vi_pipe, 0x375D, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x375E, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x375F, 0x11);
    ret +=  imx335_write_register(vi_pipe, 0x3760, 0x01);

    ret +=  imx335_write_register(vi_pipe, 0x3768, 0x1B);
    ret +=  imx335_write_register(vi_pipe, 0x3769, 0x1B);
    ret +=  imx335_write_register(vi_pipe, 0x376A, 0x1B);
    ret +=  imx335_write_register(vi_pipe, 0x376B, 0x1B);
    ret +=  imx335_write_register(vi_pipe, 0x376C, 0x1A);
    ret +=  imx335_write_register(vi_pipe, 0x376D, 0x17);
    ret +=  imx335_write_register(vi_pipe, 0x376E, 0x0F);

    ret +=  imx335_write_register(vi_pipe, 0x3776, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x3777, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x3778, 0x46);
    ret +=  imx335_write_register(vi_pipe, 0x3779, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x377A, 0x89);
    ret +=  imx335_write_register(vi_pipe, 0x377B, 0x00);
    ret +=  imx335_write_register(vi_pipe, 0x377C, 0x08);
    ret +=  imx335_write_register(vi_pipe, 0x377D, 0x01);
    ret +=  imx335_write_register(vi_pipe, 0x377E, 0x23);
    ret +=  imx335_write_register(vi_pipe, 0x377F, 0x02);
    ret +=  imx335_write_register(vi_pipe, 0x3780, 0xD9);
    ret +=  imx335_write_register(vi_pipe, 0x3781, 0x03);
    ret +=  imx335_write_register(vi_pipe, 0x3782, 0xF5);
    ret +=  imx335_write_register(vi_pipe, 0x3783, 0x06);
    ret +=  imx335_write_register(vi_pipe, 0x3784, 0xA5);
    ret +=  imx335_write_register(vi_pipe, 0x3788, 0x0F);
    ret +=  imx335_write_register(vi_pipe, 0x378A, 0xD9);
    ret +=  imx335_write_register(vi_pipe, 0x378B, 0x03);
    ret +=  imx335_write_register(vi_pipe, 0x378C, 0xEB);
    ret +=  imx335_write_register(vi_pipe, 0x378D, 0x05);
    ret +=  imx335_write_register(vi_pipe, 0x378E, 0x87);
    ret +=  imx335_write_register(vi_pipe, 0x378F, 0x06);
    ret +=  imx335_write_register(vi_pipe, 0x3790, 0xF5);
    ret +=  imx335_write_register(vi_pipe, 0x3792, 0x43);
    ret +=  imx335_write_register(vi_pipe, 0x3794, 0x7A);
    ret +=  imx335_write_register(vi_pipe, 0x3796, 0xA1);

    imx335_default_reg_init(vi_pipe);

    ret +=  imx335_write_register(vi_pipe, 0x3000, 0x00); /* Standby Cancel */
    delay_ms(18);  /* delay 18 ms */
    ret +=  imx335_write_register(vi_pipe, 0x3002, 0x00);
    if (ret != TD_SUCCESS) {
        printf("write register failed!\n");
        return;
    }
    printf("-------Sony IMX335_init_5M_2592x1944_12bit_linear30 Initial OK!-------\n");
    return;
}







