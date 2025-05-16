/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef OT_GPIO_I2C
#include "gpioi2c_ex.h"
#else
#include "ot_i2c.h"
#endif

#include "securec.h"
#include "ss_mpi_isp.h"

#define I2C_DEV_FILE_NUM     16
#define I2C_BUF_NUM          8
#define I2C_DEV_SENSOR       4

#define OS04A10_I2C_ADDR    0x6c
#define OS04A10_ADDR_BYTE   2
#define OS04A10_DATA_BYTE   1

#define I2C_DEV_FILE_NUM   16
#define I2C_BUF_NUM         8
#define I2C_MSG_CNT         2
#define I2C_READ_BUF_LEN    4
#define I2C_RDWR       0x0707
#define I2C_READ_STATUS_OK  2

#define high_8bits(x) (((x) & 0xff00) >> 8) /* shift 8 bit */
#define low_8bits(x)  ((x) & 0x00ff)

#ifndef clip3
#define clip3(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#endif

/* os04a10 Register Address */
#define OS04A10_EXPO_H_ADDR          0x3501
#define OS04A10_EXPO_L_ADDR          0x3502
#define OS04A10_AGAIN_H_ADDR         0x3508
#define OS04A10_AGAIN_L_ADDR         0x3509
#define OS04A10_DGAIN_H_ADDR         0x350a
#define OS04A10_DGAIN_M_ADDR         0x350b
#define OS04A10_DGAIN_L_ADDR         0x350c
#define OS04A10_HCG_1_ADDR           0x376C
#define OS04A10_GROUP_1_ADDR         0x320d
#define OS04A10_GROUP_2_ADDR         0x3208
#define OS04A10_GROUP_3_ADDR         0x3208
#define OS04A10_GROUP_4_ADDR         0x320d
#define OS04A10_GROUP_5_ADDR         0x320a
#define OS04A10_GROUP_6_ADDR         0x320e

static int g_fd = -1;

struct i2c_rdwr_ioctl_data {
    struct i2c_msg  *msgs;  /* pointers to i2c_msgs */
    __u32 nmsgs;            /* number of i2c_msgs */
};

struct i2c_rdwr_args {
    unsigned int i2c_num;
    unsigned int dev_addr;
    unsigned int reg_addr;
    unsigned int reg_addr_end;
    unsigned int reg_width;
    unsigned int data_width;
    unsigned int reg_step;
};

static td_u32 i2c_ioc_init(struct i2c_rdwr_ioctl_data *rdwr, unsigned char *buf,
    size_t buf_size, struct i2c_rdwr_args args)
{
    if (memset_s(buf, buf_size, 0, I2C_READ_BUF_LEN) != EOK) {
        printf("memset_s fail!\n");
        return -1;
    }
    rdwr->msgs[0].addr = args.dev_addr;
    rdwr->msgs[0].flags = 0;
    rdwr->msgs[0].len = args.reg_width;
    rdwr->msgs[0].buf = buf;

    rdwr->msgs[1].addr = args.dev_addr;
    rdwr->msgs[1].flags = 0;
    rdwr->msgs[1].flags |= I2C_M_RD;
    rdwr->msgs[1].len = args.data_width;
    rdwr->msgs[1].buf = buf;

    return 0;
}

static int sensor_os04a10_i2c_init()
{
    int ret;
    char dev_file[I2C_DEV_FILE_NUM] = {0};
    if (g_fd >= 0) {
        printf("sensor i2c fd already be created\n");
        return TD_SUCCESS;
    }
#ifdef OT_GPIO_I2C
    ot_unused(ret);
    g_fd = open("/dev/gpioi2c_ex", O_RDONLY, S_IRUSR);
    if (g_fd < 0) {
        printf("Open gpioi2c_ex error!\n");
        return TD_FAILURE;
    }
#else
    (td_void)snprintf_s(dev_file, sizeof(dev_file), sizeof(dev_file) - 1, "/dev/i2c-%d", I2C_DEV_SENSOR);

    g_fd = open(dev_file, O_RDWR, S_IRUSR | S_IWUSR);
    if (g_fd < 0) {
        printf("Open /dev/ot_i2c_drv-%d error!\n", I2C_DEV_SENSOR);
        return TD_FAILURE;
    }

    ret = ioctl(g_fd, OT_I2C_SLAVE_FORCE, (OS04A10_I2C_ADDR >> 1));
    if (ret < 0) {
        printf("I2C_SLAVE_FORCE error!\n");
        close(g_fd);
        g_fd = -1;
        return ret;
    }
#endif

    return TD_SUCCESS;
}

int sensor_os04a10_i2c_exit()
{
    if (g_fd >= 0) {
        close(g_fd);
        g_fd = -1;
        return TD_SUCCESS;
    }
    return TD_FAILURE;
}

static td_u32 sensor_os04a10_read_register(td_u32 addr)
{
    if (g_fd < 0) {
        printf("os04a10_read_register fd not opened!\n");
        return TD_FAILURE;
    }

    struct i2c_rdwr_args args;
    td_u32 cur_addr;
    td_u32 data;
    unsigned char buf[I2C_READ_BUF_LEN];
    static struct i2c_rdwr_ioctl_data rdwr;
    static struct i2c_msg msg[I2C_MSG_CNT];

    rdwr.msgs = &msg[0];
    rdwr.nmsgs = (__u32)I2C_MSG_CNT;
    args.i2c_num = I2C_DEV_SENSOR;
    args.dev_addr = (OS04A10_I2C_ADDR >> 1);
    args.reg_addr = addr;
    args.reg_addr_end = addr;
    args.reg_width = OS04A10_ADDR_BYTE;
    args.data_width = OS04A10_DATA_BYTE;
    args.reg_step = 1;
    if (i2c_ioc_init(&rdwr, buf, sizeof(buf), args) != 0) {
        return -1;
    }

    for (cur_addr = args.reg_addr; cur_addr <= args.reg_addr_end; cur_addr += args.reg_step) {
        if (args.reg_width == 2) {  /* 2 byte */
            buf[0] = (cur_addr >> 8) & 0xff;  /* shift 8 */
            buf[1] = cur_addr & 0xff;
        } else {
            buf[0] = cur_addr & 0xff;
        }

        if (ioctl(g_fd, I2C_RDWR, &rdwr) != I2C_READ_STATUS_OK) {
            printf("CMD_I2C_READ error!\n");
            return -1;
        }

        if (args.data_width == 2) {  /* 2 byte */
            data = buf[1] | (buf[0] << 8);  /* shift 8 */
        } else {
            data = buf[0];
        }
    }

    return data;
}

static td_s32 sensor_os04a10_write_register(td_u32 addr, td_u32 data)
{
    td_s32 ret;
    if (g_fd < 0) {
        return TD_SUCCESS;
    }

#ifdef OT_GPIO_I2C
    i2c_data.dev_addr = OS04A10_I2C_ADDR;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = OS04A10_ADDR_BYTE;
    i2c_data.data = data;
    i2c_data.data_byte_num = OS04A10_DATA_BYTE;

    ret = ioctl(g_fd, GPIO_I2C_WRITE, &i2c_data);
    if (ret) {
        isp_err_trace("GPIO-I2C write failed!\n");
        return ret;
    }
#else
    td_u32 idx = 0;
    td_u8 buf[I2C_BUF_NUM];

    buf[idx] = (addr >> 8) & 0xff;  /* shift 8 */
    idx++;
    buf[idx] = addr & 0xff;
    idx++;

    buf[idx] = data & 0xff;

    ret = write(g_fd, buf, OS04A10_ADDR_BYTE + OS04A10_DATA_BYTE);
    if (ret < 0) {
        isp_err_trace("I2C_WRITE error! ret = 0x%x\n", ret);
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

static td_void sensor_os04a10_write_exp(td_u32 sns_exp)
{
    td_s32 ret = TD_SUCCESS;
    td_u32 exp_reg;

    exp_reg = clip3(sns_exp, 2, 1614); /* Range:[2, 1614] */
    ret += sensor_os04a10_write_register(OS04A10_EXPO_L_ADDR, low_8bits(exp_reg));
    ret += sensor_os04a10_write_register(OS04A10_EXPO_H_ADDR, high_8bits(exp_reg));
    if (ret < 0) {
        printf("os04a10_write_sns_exp error!\n");
        return;
    }
    return;
}

static td_void sensor_os04a10_write_gain(td_u32 sns_again, td_u32 sns_dgain)
{
    td_s32 ret = TD_SUCCESS;
    td_u32 again_reg, dgain_reg, hcg_1_reg;

    again_reg = sns_again;
    dgain_reg = sns_dgain;

    if (sns_again < 0x44) {
        hcg_1_reg = 0x12;
    } else {
        hcg_1_reg = 0x02;
        again_reg = (td_u32)((td_float)again_reg / 4.25 + 0.5); /* HCG gain equal 4.25, 0.5 */
    }

    again_reg = clip3(again_reg, 0x10, 0xF8);
    dgain_reg = clip3(dgain_reg, 0x400, 0x3FFF);

    ret += sensor_os04a10_write_register(OS04A10_AGAIN_L_ADDR, ((again_reg & 0xf) << 4)); /* shift 4 */
    ret += sensor_os04a10_write_register(OS04A10_AGAIN_H_ADDR, ((again_reg & 0xf0) >> 4)); /* shift 4 */

    ret += sensor_os04a10_write_register(OS04A10_DGAIN_L_ADDR, ((dgain_reg & 0x03) << 6)); /* shift 6 */
    ret += sensor_os04a10_write_register(OS04A10_DGAIN_M_ADDR, ((dgain_reg & 0x03FC) >> 2)); /* shift 2 */
    ret += sensor_os04a10_write_register(OS04A10_DGAIN_H_ADDR, ((dgain_reg & 0x3C00) >> 10)); /* shift 10 */

    ret += sensor_os04a10_write_register(OS04A10_GROUP_1_ADDR, 0x00);
    ret += sensor_os04a10_write_register(OS04A10_GROUP_2_ADDR, 0x01);
    ret += sensor_os04a10_write_register(OS04A10_HCG_1_ADDR, (hcg_1_reg & 0xff));
    ret += sensor_os04a10_write_register(OS04A10_GROUP_3_ADDR, 0x11);
    ret += sensor_os04a10_write_register(OS04A10_GROUP_4_ADDR, 0x05);
    ret += sensor_os04a10_write_register(OS04A10_GROUP_5_ADDR, 0x01);
    ret += sensor_os04a10_write_register(OS04A10_GROUP_6_ADDR, 0xa0);
    if (ret < 0) {
        printf("os04a10_write_sns_gain error!\n");
        return;
    }
    return;
}

static td_void sensor_os04a10_linear_4m30_12bit_init();

td_void sensor_os04a10_init(td_u32 sns_exp, td_u32 sns_again, td_u32 sns_dgain)
{
    td_s32 ret;
    delay_ms(1);
    ret = sensor_os04a10_i2c_init();
    if (ret != TD_SUCCESS) {
        isp_err_trace("i2c init failed!\n");
        return;
    }

    sensor_os04a10_linear_4m30_12bit_init();
    sensor_os04a10_write_exp(sns_exp);
    sensor_os04a10_write_gain(sns_again, sns_dgain);

    delay_ms(5); /* delay 5 ms */
    sensor_os04a10_write_register(0x0100, 0x01);

    return;
}

td_void sensor_os04a10_read_exp(td_u32 *sns_exp)
{
    td_u32 exp_reg_low, exp_reg_high;

    exp_reg_low = sensor_os04a10_read_register(OS04A10_EXPO_L_ADDR);
    exp_reg_high = sensor_os04a10_read_register(OS04A10_EXPO_H_ADDR);

    *sns_exp = ((exp_reg_high & 0xff) << 8) | (exp_reg_low & 0xff); /* shift 8 */
    return;
}

td_void sensor_os04a10_read_gain(td_u32 *sns_again, td_u32 *sns_dgain)
{
    td_u32 again_reg_low, again_reg_high, hcg_reg, dgain_reg_low, dgain_reg_mid, dgain_reg_high;

    again_reg_low = sensor_os04a10_read_register(OS04A10_AGAIN_L_ADDR);
    again_reg_high = sensor_os04a10_read_register(OS04A10_AGAIN_H_ADDR);
    hcg_reg = sensor_os04a10_read_register(OS04A10_HCG_1_ADDR);

    dgain_reg_low = sensor_os04a10_read_register(OS04A10_DGAIN_L_ADDR);
    dgain_reg_mid = sensor_os04a10_read_register(OS04A10_DGAIN_M_ADDR);
    dgain_reg_high = sensor_os04a10_read_register(OS04A10_DGAIN_H_ADDR);

    *sns_again = ((again_reg_low & 0xf0) >> 4) | ((again_reg_high & 0xf) << 4); /* shift 4 */
    if (hcg_reg == 0x02) {
        *sns_again = (td_u32)(((td_float)(*sns_again) - 0.5) * 4.25); /* HCG gain equal 4.25, 0.5 */
    }

    *sns_dgain = ((dgain_reg_high & 0xf) << 10) | ((dgain_reg_mid & 0xff) << 2) /* shift 10, shift 2 */
        | ((dgain_reg_low & 0xc0) >> 6); /* shift 6 */
    return;
}

void sensor_os04a10_exit()
{
    td_s32 ret;

    ret = sensor_os04a10_i2c_exit();
    if (ret != TD_SUCCESS) {
        isp_err_trace("OS04A10 exit failed!\n");
    }

    return;
}

static td_s32 sensor_os04a10_linear_4m30_12bit_init_part1()
{
    td_s32 ret = TD_SUCCESS;
    ret += sensor_os04a10_write_register(0x0102, 0x00);
    ret += sensor_os04a10_write_register(0x0305, 0x42);
    ret += sensor_os04a10_write_register(0x0306, 0x00);
    ret += sensor_os04a10_write_register(0x0307, 0x00);
    ret += sensor_os04a10_write_register(0x0308, 0x05);
    ret += sensor_os04a10_write_register(0x030a, 0x01);
    ret += sensor_os04a10_write_register(0x0317, 0x0a);
    ret += sensor_os04a10_write_register(0x0322, 0x01);
    ret += sensor_os04a10_write_register(0x0323, 0x02);
    ret += sensor_os04a10_write_register(0x0324, 0x00);
    ret += sensor_os04a10_write_register(0x0325, 0x90);
    ret += sensor_os04a10_write_register(0x0327, 0x05);
    ret += sensor_os04a10_write_register(0x0329, 0x02);
    ret += sensor_os04a10_write_register(0x032c, 0x02);
    ret += sensor_os04a10_write_register(0x032d, 0x02);
    ret += sensor_os04a10_write_register(0x032e, 0x02);
    ret += sensor_os04a10_write_register(0x300f, 0x11);
    ret += sensor_os04a10_write_register(0x3012, 0x41);
    ret += sensor_os04a10_write_register(0x3026, 0x10);
    ret += sensor_os04a10_write_register(0x3027, 0x08);
    ret += sensor_os04a10_write_register(0x302d, 0x24);
    ret += sensor_os04a10_write_register(0x3104, 0x01);
    ret += sensor_os04a10_write_register(0x3106, 0x11);
    ret += sensor_os04a10_write_register(0x3400, 0x00);
    ret += sensor_os04a10_write_register(0x3408, 0x05);
    ret += sensor_os04a10_write_register(0x340c, 0x0c);
    ret += sensor_os04a10_write_register(0x340d, 0xb0);
    ret += sensor_os04a10_write_register(0x3425, 0x51);
    ret += sensor_os04a10_write_register(0x3426, 0x10);
    ret += sensor_os04a10_write_register(0x3427, 0x14);
    ret += sensor_os04a10_write_register(0x3428, 0x10);
    ret += sensor_os04a10_write_register(0x3429, 0x10);
    ret += sensor_os04a10_write_register(0x342a, 0x10);
    ret += sensor_os04a10_write_register(0x342b, 0x04);
    ret += sensor_os04a10_write_register(0x3501, 0x02);
    ret += sensor_os04a10_write_register(0x3504, 0x08);
    ret += sensor_os04a10_write_register(0x3508, 0x01);
    ret += sensor_os04a10_write_register(0x3509, 0x00);
    ret += sensor_os04a10_write_register(0x350a, 0x01);
    ret += sensor_os04a10_write_register(0x3544, 0x08);
    ret += sensor_os04a10_write_register(0x3548, 0x01);
    ret += sensor_os04a10_write_register(0x3549, 0x00);
    ret += sensor_os04a10_write_register(0x3584, 0x08);
    ret += sensor_os04a10_write_register(0x3588, 0x01);
    ret += sensor_os04a10_write_register(0x3589, 0x00);

    return ret;
}

static td_s32 sensor_os04a10_linear_4m30_12bit_init_part2()
{
    td_s32 ret = TD_SUCCESS;
    ret += sensor_os04a10_write_register(0x3601, 0x70);
    ret += sensor_os04a10_write_register(0x3604, 0xe3);
    ret += sensor_os04a10_write_register(0x3605, 0xff);
    ret += sensor_os04a10_write_register(0x3606, 0x01);
    ret += sensor_os04a10_write_register(0x3608, 0xa8);
    ret += sensor_os04a10_write_register(0x360a, 0xd0);
    ret += sensor_os04a10_write_register(0x360b, 0x08);
    ret += sensor_os04a10_write_register(0x360e, 0xc8);
    ret += sensor_os04a10_write_register(0x360f, 0x66);
    ret += sensor_os04a10_write_register(0x3610, 0x89);
    ret += sensor_os04a10_write_register(0x3611, 0x8a);
    ret += sensor_os04a10_write_register(0x3612, 0x4e);
    ret += sensor_os04a10_write_register(0x3613, 0xbd);
    ret += sensor_os04a10_write_register(0x3614, 0x9b);
    ret += sensor_os04a10_write_register(0x362a, 0x0e);
    ret += sensor_os04a10_write_register(0x362b, 0x0e);
    ret += sensor_os04a10_write_register(0x362c, 0x0e);
    ret += sensor_os04a10_write_register(0x362d, 0x09);
    ret += sensor_os04a10_write_register(0x362e, 0x1a);
    ret += sensor_os04a10_write_register(0x362f, 0x34);
    ret += sensor_os04a10_write_register(0x3630, 0x67);
    ret += sensor_os04a10_write_register(0x3631, 0x7f);
    ret += sensor_os04a10_write_register(0x3638, 0x00);
    ret += sensor_os04a10_write_register(0x3643, 0x00);
    ret += sensor_os04a10_write_register(0x3644, 0x00);
    ret += sensor_os04a10_write_register(0x3645, 0x00);
    ret += sensor_os04a10_write_register(0x3646, 0x00);
    ret += sensor_os04a10_write_register(0x3647, 0x00);
    ret += sensor_os04a10_write_register(0x3648, 0x00);
    ret += sensor_os04a10_write_register(0x3649, 0x00);
    ret += sensor_os04a10_write_register(0x364a, 0x04);
    ret += sensor_os04a10_write_register(0x364c, 0x0e);
    ret += sensor_os04a10_write_register(0x364d, 0x0e);
    ret += sensor_os04a10_write_register(0x364e, 0x0e);
    ret += sensor_os04a10_write_register(0x364f, 0x0e);
    ret += sensor_os04a10_write_register(0x3650, 0xff);
    ret += sensor_os04a10_write_register(0x3651, 0xff);
    ret += sensor_os04a10_write_register(0x365a, 0x00);
    ret += sensor_os04a10_write_register(0x365b, 0x00);
    ret += sensor_os04a10_write_register(0x365c, 0x00);
    ret += sensor_os04a10_write_register(0x365d, 0x00);
    ret += sensor_os04a10_write_register(0x3661, 0x07);
    ret += sensor_os04a10_write_register(0x3662, 0x00);
    ret += sensor_os04a10_write_register(0x3663, 0x20);
    ret += sensor_os04a10_write_register(0x3665, 0x12);

    return ret;
}

static td_s32 sensor_os04a10_linear_4m30_12bit_init_part3()
{
    td_s32 ret = TD_SUCCESS;
    ret += sensor_os04a10_write_register(0x3667, 0xd4);
    ret += sensor_os04a10_write_register(0x3668, 0x80);
    ret += sensor_os04a10_write_register(0x366c, 0x00);
    ret += sensor_os04a10_write_register(0x366d, 0x00);
    ret += sensor_os04a10_write_register(0x366e, 0x00);
    ret += sensor_os04a10_write_register(0x366f, 0x00);
    ret += sensor_os04a10_write_register(0x3671, 0x08);
    ret += sensor_os04a10_write_register(0x3673, 0x2a);
    ret += sensor_os04a10_write_register(0x3681, 0x80);
    ret += sensor_os04a10_write_register(0x3700, 0x2d);
    ret += sensor_os04a10_write_register(0x3701, 0x22);
    ret += sensor_os04a10_write_register(0x3702, 0x25);
    ret += sensor_os04a10_write_register(0x3703, 0x28);
    ret += sensor_os04a10_write_register(0x3705, 0x00);
    ret += sensor_os04a10_write_register(0x3706, 0xf0);
    ret += sensor_os04a10_write_register(0x3707, 0x0a);
    ret += sensor_os04a10_write_register(0x3708, 0x36);
    ret += sensor_os04a10_write_register(0x3709, 0x57);
    ret += sensor_os04a10_write_register(0x370a, 0x03);
    ret += sensor_os04a10_write_register(0x370b, 0x15);
    ret += sensor_os04a10_write_register(0x3714, 0x01);
    ret += sensor_os04a10_write_register(0x3719, 0x24);
    ret += sensor_os04a10_write_register(0x371b, 0x1f);
    ret += sensor_os04a10_write_register(0x371c, 0x00);
    ret += sensor_os04a10_write_register(0x371d, 0x08);
    ret += sensor_os04a10_write_register(0x373f, 0x63);
    ret += sensor_os04a10_write_register(0x3740, 0x63);
    ret += sensor_os04a10_write_register(0x3741, 0x63);
    ret += sensor_os04a10_write_register(0x3742, 0x63);
    ret += sensor_os04a10_write_register(0x3743, 0x01);
    ret += sensor_os04a10_write_register(0x3756, 0xe7);
    ret += sensor_os04a10_write_register(0x3757, 0xe7);
    ret += sensor_os04a10_write_register(0x3762, 0x1c);
    ret += sensor_os04a10_write_register(0x376c, 0x00);
    ret += sensor_os04a10_write_register(0x3776, 0x05);
    ret += sensor_os04a10_write_register(0x3777, 0x22);
    ret += sensor_os04a10_write_register(0x3779, 0x60);
    ret += sensor_os04a10_write_register(0x377c, 0x48);
    ret += sensor_os04a10_write_register(0x3784, 0x06);
    ret += sensor_os04a10_write_register(0x3785, 0x0a);
    ret += sensor_os04a10_write_register(0x3790, 0x10);
    ret += sensor_os04a10_write_register(0x3793, 0x04);
    ret += sensor_os04a10_write_register(0x3794, 0x07);
    ret += sensor_os04a10_write_register(0x3796, 0x00);
    ret += sensor_os04a10_write_register(0x3797, 0x02);

    return ret;
}

static td_s32 sensor_os04a10_linear_4m30_12bit_init_part4()
{
    td_s32 ret = TD_SUCCESS;
    ret += sensor_os04a10_write_register(0x379c, 0x4d);
    ret += sensor_os04a10_write_register(0x37a1, 0x80);
    ret += sensor_os04a10_write_register(0x37bb, 0x88);
    ret += sensor_os04a10_write_register(0x37be, 0x48);
    ret += sensor_os04a10_write_register(0x37bf, 0x01);
    ret += sensor_os04a10_write_register(0x37c0, 0x01);
    ret += sensor_os04a10_write_register(0x37c4, 0x72);
    ret += sensor_os04a10_write_register(0x37c5, 0x72);
    ret += sensor_os04a10_write_register(0x37c6, 0x72);
    ret += sensor_os04a10_write_register(0x37ca, 0x21);
    ret += sensor_os04a10_write_register(0x37cc, 0x15);
    ret += sensor_os04a10_write_register(0x37cd, 0x90);
    ret += sensor_os04a10_write_register(0x37cf, 0x02);
    ret += sensor_os04a10_write_register(0x37d0, 0x00);
    ret += sensor_os04a10_write_register(0x37d1, 0xf0);
    ret += sensor_os04a10_write_register(0x37d2, 0x03);
    ret += sensor_os04a10_write_register(0x37d3, 0x15);
    ret += sensor_os04a10_write_register(0x37d4, 0x01);
    ret += sensor_os04a10_write_register(0x37d5, 0x00);
    ret += sensor_os04a10_write_register(0x37d6, 0x03);
    ret += sensor_os04a10_write_register(0x37d7, 0x15);
    ret += sensor_os04a10_write_register(0x37d8, 0x01);
    ret += sensor_os04a10_write_register(0x37dc, 0x00);
    ret += sensor_os04a10_write_register(0x37dd, 0x00);
    ret += sensor_os04a10_write_register(0x37da, 0x00);
    ret += sensor_os04a10_write_register(0x37db, 0x00);
    ret += sensor_os04a10_write_register(0x3800, 0x00);
    ret += sensor_os04a10_write_register(0x3801, 0x00);
    ret += sensor_os04a10_write_register(0x3802, 0x00);
    ret += sensor_os04a10_write_register(0x3803, 0x00);
    ret += sensor_os04a10_write_register(0x3804, 0x0a);
    ret += sensor_os04a10_write_register(0x3805, 0x8f);
    ret += sensor_os04a10_write_register(0x3806, 0x05);
    ret += sensor_os04a10_write_register(0x3807, 0xff);
    ret += sensor_os04a10_write_register(0x3808, 0x0a);
    ret += sensor_os04a10_write_register(0x3809, 0x80);
    ret += sensor_os04a10_write_register(0x380a, 0x05);
    ret += sensor_os04a10_write_register(0x380b, 0xf0);
    ret += sensor_os04a10_write_register(0x380c, 0x05);
    ret += sensor_os04a10_write_register(0x380d, 0xc4);
    ret += sensor_os04a10_write_register(0x380e, 0x06);
    ret += sensor_os04a10_write_register(0x380f, 0x58);
    ret += sensor_os04a10_write_register(0x3811, 0x09);
    ret += sensor_os04a10_write_register(0x3813, 0x09);
    ret += sensor_os04a10_write_register(0x3814, 0x01);

    return ret;
}

static td_s32 sensor_os04a10_linear_4m30_12bit_init_part5()
{
    td_s32 ret = TD_SUCCESS;
    ret += sensor_os04a10_write_register(0x3815, 0x01);
    ret += sensor_os04a10_write_register(0x3816, 0x01);
    ret += sensor_os04a10_write_register(0x3817, 0x01);
    ret += sensor_os04a10_write_register(0x381c, 0x00);
    ret += sensor_os04a10_write_register(0x3820, 0x02);
    ret += sensor_os04a10_write_register(0x3821, 0x00);
    ret += sensor_os04a10_write_register(0x3822, 0x14);
    ret += sensor_os04a10_write_register(0x3823, 0x18);
    ret += sensor_os04a10_write_register(0x3826, 0x00);
    ret += sensor_os04a10_write_register(0x3827, 0x00);
    ret += sensor_os04a10_write_register(0x3833, 0x40);
    ret += sensor_os04a10_write_register(0x384c, 0x05);
    ret += sensor_os04a10_write_register(0x384d, 0xc4);
    ret += sensor_os04a10_write_register(0x3858, 0x3c);
    ret += sensor_os04a10_write_register(0x3865, 0x02);
    ret += sensor_os04a10_write_register(0x3866, 0x00);
    ret += sensor_os04a10_write_register(0x3867, 0x00);
    ret += sensor_os04a10_write_register(0x3868, 0x02);
    ret += sensor_os04a10_write_register(0x3900, 0x13);
    ret += sensor_os04a10_write_register(0x3940, 0x13);
    ret += sensor_os04a10_write_register(0x3980, 0x13);
    ret += sensor_os04a10_write_register(0x3c01, 0x11);
    ret += sensor_os04a10_write_register(0x3c05, 0x00);
    ret += sensor_os04a10_write_register(0x3c0f, 0x1c);
    ret += sensor_os04a10_write_register(0x3c12, 0x0d);
    ret += sensor_os04a10_write_register(0x3c19, 0x00);
    ret += sensor_os04a10_write_register(0x3c21, 0x00);
    ret += sensor_os04a10_write_register(0x3c3a, 0x10);
    ret += sensor_os04a10_write_register(0x3c3b, 0x18);
    ret += sensor_os04a10_write_register(0x3c3d, 0xc6);
    ret += sensor_os04a10_write_register(0x3c55, 0xcb);
    ret += sensor_os04a10_write_register(0x3c5a, 0xe5);
    ret += sensor_os04a10_write_register(0x3c5d, 0xcf);
    ret += sensor_os04a10_write_register(0x3c5e, 0xcf);
    ret += sensor_os04a10_write_register(0x3d8c, 0x70);
    ret += sensor_os04a10_write_register(0x3d8d, 0x10);
    ret += sensor_os04a10_write_register(0x4000, 0xf9);
    ret += sensor_os04a10_write_register(0x4001, 0x2f);
    ret += sensor_os04a10_write_register(0x4004, 0x01);
    ret += sensor_os04a10_write_register(0x4005, 0x00);
    ret += sensor_os04a10_write_register(0x4008, 0x02);
    ret += sensor_os04a10_write_register(0x4009, 0x11);
    ret += sensor_os04a10_write_register(0x400a, 0x03);
    ret += sensor_os04a10_write_register(0x400b, 0x27);
    ret += sensor_os04a10_write_register(0x400e, 0x40);

    return ret;
}

static td_s32 sensor_os04a10_linear_4m30_12bit_init_part6()
{
    td_s32 ret = TD_SUCCESS;
    ret += sensor_os04a10_write_register(0x402e, 0x01);
    ret += sensor_os04a10_write_register(0x402f, 0x00);
    ret += sensor_os04a10_write_register(0x4030, 0x00);
    ret += sensor_os04a10_write_register(0x4031, 0x80);
    ret += sensor_os04a10_write_register(0x4032, 0x9f);
    ret += sensor_os04a10_write_register(0x4033, 0x80);
    ret += sensor_os04a10_write_register(0x4050, 0x00);
    ret += sensor_os04a10_write_register(0x4051, 0x07);
    ret += sensor_os04a10_write_register(0x4011, 0xbb);
    ret += sensor_os04a10_write_register(0x410f, 0x01);
    ret += sensor_os04a10_write_register(0x4288, 0xcf);
    ret += sensor_os04a10_write_register(0x4289, 0x00);
    ret += sensor_os04a10_write_register(0x428a, 0x46);
    ret += sensor_os04a10_write_register(0x430b, 0xff);
    ret += sensor_os04a10_write_register(0x430c, 0xff);
    ret += sensor_os04a10_write_register(0x430d, 0x00);
    ret += sensor_os04a10_write_register(0x430e, 0x00);
    ret += sensor_os04a10_write_register(0x4314, 0x04);
    ret += sensor_os04a10_write_register(0x4500, 0x18);
    ret += sensor_os04a10_write_register(0x4501, 0x18);
    ret += sensor_os04a10_write_register(0x4503, 0x10);
    ret += sensor_os04a10_write_register(0x4504, 0x00);
    ret += sensor_os04a10_write_register(0x4506, 0x32);
    ret += sensor_os04a10_write_register(0x4507, 0x02);
    ret += sensor_os04a10_write_register(0x4601, 0x30);
    ret += sensor_os04a10_write_register(0x4603, 0x00);
    ret += sensor_os04a10_write_register(0x460a, 0x50);
    ret += sensor_os04a10_write_register(0x460c, 0x60);
    ret += sensor_os04a10_write_register(0x4640, 0x62);
    ret += sensor_os04a10_write_register(0x4646, 0xaa);
    ret += sensor_os04a10_write_register(0x4647, 0x55);
    ret += sensor_os04a10_write_register(0x4648, 0x99);
    ret += sensor_os04a10_write_register(0x4649, 0x66);
    ret += sensor_os04a10_write_register(0x464d, 0x00);
    ret += sensor_os04a10_write_register(0x4654, 0x11);
    ret += sensor_os04a10_write_register(0x4655, 0x22);
    ret += sensor_os04a10_write_register(0x4800, 0x44);
    ret += sensor_os04a10_write_register(0x480e, 0x00);
    ret += sensor_os04a10_write_register(0x4810, 0xff);
    ret += sensor_os04a10_write_register(0x4811, 0xff);
    ret += sensor_os04a10_write_register(0x4813, 0x00);
    ret += sensor_os04a10_write_register(0x481f, 0x30);
    ret += sensor_os04a10_write_register(0x4837, 0x14);
    ret += sensor_os04a10_write_register(0x484b, 0x27);
    ret += sensor_os04a10_write_register(0x4d00, 0x4d);

    return ret;
}

static td_s32 sensor_os04a10_linear_4m30_12bit_init_part7()
{
    td_s32 ret = TD_SUCCESS;
    ret += sensor_os04a10_write_register(0x4d01, 0x9d);
    ret += sensor_os04a10_write_register(0x4d02, 0xb9);
    ret += sensor_os04a10_write_register(0x4d03, 0x2e);
    ret += sensor_os04a10_write_register(0x4d04, 0x4a);
    ret += sensor_os04a10_write_register(0x4d05, 0x3d);
    ret += sensor_os04a10_write_register(0x4d09, 0x4f);
    ret += sensor_os04a10_write_register(0x5000, 0x17);
    ret += sensor_os04a10_write_register(0x5001, 0x0d);
    ret += sensor_os04a10_write_register(0x5080, 0x00);
    ret += sensor_os04a10_write_register(0x50c0, 0x00);
    ret += sensor_os04a10_write_register(0x5100, 0x00);
    ret += sensor_os04a10_write_register(0x5200, 0x00);
    ret += sensor_os04a10_write_register(0x5201, 0x00);
    ret += sensor_os04a10_write_register(0x5202, 0x03);
    ret += sensor_os04a10_write_register(0x5203, 0xff);
    ret += sensor_os04a10_write_register(0x5780, 0x53);
    ret += sensor_os04a10_write_register(0x5782, 0x60);
    ret += sensor_os04a10_write_register(0x5783, 0xf0);
    ret += sensor_os04a10_write_register(0x5786, 0x01);
    ret += sensor_os04a10_write_register(0x5788, 0x60);
    ret += sensor_os04a10_write_register(0x5789, 0xf0);
    ret += sensor_os04a10_write_register(0x5792, 0x11);
    ret += sensor_os04a10_write_register(0x5793, 0x33);
    ret += sensor_os04a10_write_register(0x5857, 0xff);
    ret += sensor_os04a10_write_register(0x5858, 0xff);
    ret += sensor_os04a10_write_register(0x5859, 0xff);
    ret += sensor_os04a10_write_register(0x58d7, 0xff);
    ret += sensor_os04a10_write_register(0x58d8, 0xff);
    ret += sensor_os04a10_write_register(0x58d9, 0xff);

    return ret;
}

static void sensor_os04a10_linear_4m30_12bit_init()
{
    td_s32 ret = TD_SUCCESS;

    ret += sensor_os04a10_linear_4m30_12bit_init_part1();
    ret += sensor_os04a10_linear_4m30_12bit_init_part2();
    ret += sensor_os04a10_linear_4m30_12bit_init_part3();
    ret += sensor_os04a10_linear_4m30_12bit_init_part4();
    ret += sensor_os04a10_linear_4m30_12bit_init_part5();
    ret += sensor_os04a10_linear_4m30_12bit_init_part6();
    ret += sensor_os04a10_linear_4m30_12bit_init_part7();
    if (ret != TD_SUCCESS) {
        isp_err_trace("os04a10 write register failed!\n");
        return;
    }

    printf("========================================================================\n");
    printf("vi_pipe:%d,== os04a10 24Mclk 4M30fps(MIPI) 12bit linear init success! ==\n", 0);
    printf("========================================================================\n");
    return;
}
