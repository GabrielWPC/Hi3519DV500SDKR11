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

#define OS08A20_I2C_ADDR    0x6c
#define OS08A20_ADDR_BYTE   2
#define OS08A20_DATA_BYTE   1

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

/* os08a20 Register Address */
#define OS08A20_EXPO_H_ADDR          0x3501
#define OS08A20_EXPO_L_ADDR          0x3502
#define OS08A20_AGAIN_H_ADDR         0x3508
#define OS08A20_AGAIN_L_ADDR         0x3509
#define OS08A20_DGAIN_H_ADDR         0x350a
#define OS08A20_DGAIN_L_ADDR         0x350b

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

static int sensor_os08a20_i2c_init()
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

    ret = ioctl(g_fd, OT_I2C_SLAVE_FORCE, (OS08A20_I2C_ADDR >> 1));
    if (ret < 0) {
        printf("I2C_SLAVE_FORCE error!\n");
        close(g_fd);
        g_fd = -1;
        return ret;
    }
#endif

    return TD_SUCCESS;
}

int sensor_os08a20_i2c_exit()
{
    if (g_fd >= 0) {
        close(g_fd);
        g_fd = -1;
        return TD_SUCCESS;
    }
    return TD_FAILURE;
}

static td_u32 sensor_os08a20_read_register(td_u32 addr)
{
    if (g_fd < 0) {
        printf("os08a20_read_register fd not opened!\n");
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
    args.dev_addr = (OS08A20_I2C_ADDR >> 1);
    args.reg_addr = addr;
    args.reg_addr_end = addr;
    args.reg_width = OS08A20_ADDR_BYTE;
    args.data_width = OS08A20_DATA_BYTE;
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

static td_s32 sensor_os08a20_write_register(td_u32 addr, td_u32 data)
{
    td_s32 ret;
    if (g_fd < 0) {
        return TD_SUCCESS;
    }

#ifdef OT_GPIO_I2C
    i2c_data.dev_addr = OS08A20_I2C_ADDR;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = OS08A20_ADDR_BYTE;
    i2c_data.data = data;
    i2c_data.data_byte_num = OS08A20_DATA_BYTE;

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

    ret = write(g_fd, buf, OS08A20_ADDR_BYTE + OS08A20_DATA_BYTE);
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

static td_void sensor_os08a20_write_exp(td_u32 sns_exp)
{
    td_s32 ret = TD_SUCCESS;
    td_u32 exp_reg;

    exp_reg = clip3(sns_exp, 8, 2306); /* Range:[8, 2306] */
    ret += sensor_os08a20_write_register(OS08A20_EXPO_L_ADDR, low_8bits(exp_reg));
    ret += sensor_os08a20_write_register(OS08A20_EXPO_H_ADDR, high_8bits(exp_reg));
    if (ret < 0) {
        printf("os08a20_write_sns_exp error!\n");
        return;
    }
    return;
}

#define GAIN_NODE_NUM 64
static td_u32 g_gain_table[GAIN_NODE_NUM] = {
    1024, 1088, 1152, 1216, 1280, 1344, 1408, 1472, 1536, 1600, 1664, 1728, 1792, 1856, 1920, 1984, 2048, 2176,
    2304, 2432, 2560, 2688, 2816, 2944, 3072, 3200, 3328, 3456, 3584, 3712, 3840, 3968, 4096, 4352, 4608, 4864,
    5120, 5376, 5632, 5888, 6144, 6400, 6656, 6912, 7168, 7424, 7680, 7936, 8192, 8704, 9216, 9728, 10240, 10752,
    11264, 11776, 12288, 12800, 13312, 13824, 14336, 14848, 15360, 15872
};

static td_void sensor_os08a20_write_gain(td_u32 sns_again, td_u32 sns_dgain)
{
    td_s32 i, again_index = 0, ret = TD_SUCCESS;
    td_u32 again_reg, dgain_reg;

    again_reg = sns_again;
    dgain_reg = sns_dgain;

    if (again_reg >= g_gain_table[GAIN_NODE_NUM - 1]) {
        again_reg = g_gain_table[GAIN_NODE_NUM - 1];
        again_index = GAIN_NODE_NUM - 1;
    } else {
        for (i = 1; i < GAIN_NODE_NUM; i++) {
            if (again_reg < g_gain_table[i]) {
                again_reg = g_gain_table[i - 1];
                again_index = i - 1;
                break;
            }
        }
    }
    again_reg = g_gain_table[again_index];
    again_reg = clip3((again_reg >> 3), 0x80, 0x7C0); /* shift 3 */

    if (dgain_reg < 0x80) {
        dgain_reg = 0x400;
    } else if (dgain_reg < 0x800) {
        dgain_reg = dgain_reg * 0x8;
    } else {
        dgain_reg = 0x3FFF;
    }

    ret += sensor_os08a20_write_register(OS08A20_AGAIN_L_ADDR, (again_reg & 0xff));
    ret += sensor_os08a20_write_register(OS08A20_AGAIN_H_ADDR, ((again_reg & 0x3f00) >> 8)); /* shift 8 */

    ret += sensor_os08a20_write_register(OS08A20_DGAIN_L_ADDR, (dgain_reg & 0xff));
    ret += sensor_os08a20_write_register(OS08A20_DGAIN_H_ADDR, ((dgain_reg & 0x3f00) >> 8)); /* shift 8 */
    if (ret < 0) {
        printf("os08a20_write_sns_gain error!\n");
        return;
    }
    return;
}

static td_void sensor_os08a20_linear_8m30_12bit_init();

td_void sensor_os08a20_init(td_u32 sns_exp, td_u32 sns_again, td_u32 sns_dgain)
{
    td_s32 ret;
    delay_ms(1);
    ret = sensor_os08a20_i2c_init();
    if (ret != TD_SUCCESS) {
        isp_err_trace("i2c init failed!\n");
        return;
    }

    sensor_os08a20_linear_8m30_12bit_init();
    sensor_os08a20_write_exp(sns_exp);
    sensor_os08a20_write_gain(sns_again, sns_dgain);

    delay_ms(5); /* delay 5 ms */
    sensor_os08a20_write_register(0x0100, 0x01);

    return;
}

td_void sensor_os08a20_read_exp(td_u32 *sns_exp)
{
    td_u32 exp_reg_low, exp_reg_high;

    exp_reg_low = sensor_os08a20_read_register(OS08A20_EXPO_L_ADDR);
    exp_reg_high = sensor_os08a20_read_register(OS08A20_EXPO_H_ADDR);

    *sns_exp = ((exp_reg_high & 0xff) << 8) | (exp_reg_low & 0xff); /* shift 8 */
    return;
}

td_void sensor_os08a20_read_gain(td_u32 *sns_again, td_u32 *sns_dgain)
{
    td_u32 again_reg_low, again_reg_high, dgain_reg_low, dgain_reg_high;
    td_u32 again, dgain;

    again_reg_low = sensor_os08a20_read_register(OS08A20_AGAIN_L_ADDR);
    again_reg_high = sensor_os08a20_read_register(OS08A20_AGAIN_H_ADDR);

    dgain_reg_low = sensor_os08a20_read_register(OS08A20_DGAIN_L_ADDR);
    dgain_reg_high = sensor_os08a20_read_register(OS08A20_DGAIN_H_ADDR);

    again = ((again_reg_low & 0xff) | ((again_reg_high & 0x3f) << 8)); /* shift 8 */
    dgain = ((dgain_reg_low & 0xff) | ((dgain_reg_high & 0x3f) << 8)); /* shift 8 */

    *sns_again = again << 3; /* shift 3 */
    *sns_dgain = dgain >> 3; /* shift 3 */
    return;
}

void sensor_os08a20_exit()
{
    td_s32 ret;

    ret = sensor_os08a20_i2c_exit();
    if (ret != TD_SUCCESS) {
        isp_err_trace("OS08A20 exit failed!\n");
    }

    return;
}

static td_s32 sensor_os08a20_linear_8m30_12bit_init_part1()
{
    td_s32 ret = TD_SUCCESS;
    ret += sensor_os08a20_write_register(0x0100, 0x00);
    ret += sensor_os08a20_write_register(0x0103, 0x01);
    ret += sensor_os08a20_write_register(0x0303, 0x01);
    ret += sensor_os08a20_write_register(0x0305, 0x5a);
    ret += sensor_os08a20_write_register(0x0306, 0x00);
    ret += sensor_os08a20_write_register(0x0308, 0x03);
    ret += sensor_os08a20_write_register(0x0309, 0x04);
    ret += sensor_os08a20_write_register(0x032a, 0x00);
    ret += sensor_os08a20_write_register(0x300f, 0x11);
    ret += sensor_os08a20_write_register(0x3010, 0x01);
    ret += sensor_os08a20_write_register(0x3011, 0x04);
    ret += sensor_os08a20_write_register(0x3012, 0x41);
    ret += sensor_os08a20_write_register(0x3016, 0xf0);
    ret += sensor_os08a20_write_register(0x301e, 0x98);
    ret += sensor_os08a20_write_register(0x3031, 0xa9);
    ret += sensor_os08a20_write_register(0x3103, 0x92);
    ret += sensor_os08a20_write_register(0x3104, 0x01);
    ret += sensor_os08a20_write_register(0x3106, 0x10);
    ret += sensor_os08a20_write_register(0x3400, 0x04);
    ret += sensor_os08a20_write_register(0x3025, 0x03);
    ret += sensor_os08a20_write_register(0x3425, 0x01);
    ret += sensor_os08a20_write_register(0x3428, 0x01);
    ret += sensor_os08a20_write_register(0x3406, 0x08);
    ret += sensor_os08a20_write_register(0x3408, 0x03);
    ret += sensor_os08a20_write_register(0x340c, 0xff);
    ret += sensor_os08a20_write_register(0x340d, 0xff);
    ret += sensor_os08a20_write_register(0x031e, 0x0a);
    ret += sensor_os08a20_write_register(0x3501, 0x08);
    ret += sensor_os08a20_write_register(0x3502, 0xe5);
    ret += sensor_os08a20_write_register(0x3505, 0x83);
    ret += sensor_os08a20_write_register(0x3508, 0x00);
    ret += sensor_os08a20_write_register(0x3509, 0x80);
    ret += sensor_os08a20_write_register(0x350a, 0x04);
    ret += sensor_os08a20_write_register(0x350b, 0x00);
    ret += sensor_os08a20_write_register(0x350c, 0x00);
    ret += sensor_os08a20_write_register(0x350d, 0x80);
    ret += sensor_os08a20_write_register(0x350e, 0x04);
    ret += sensor_os08a20_write_register(0x350f, 0x00);
    ret += sensor_os08a20_write_register(0x3600, 0x00);
    ret += sensor_os08a20_write_register(0x3603, 0x2c);
    ret += sensor_os08a20_write_register(0x3605, 0x50);
    ret += sensor_os08a20_write_register(0x3609, 0xdb);
    ret += sensor_os08a20_write_register(0x3610, 0x39);
    ret += sensor_os08a20_write_register(0x360c, 0x01);
    ret += sensor_os08a20_write_register(0x3628, 0xa4);

    return ret;
}

static td_s32 sensor_os08a20_linear_8m30_12bit_init_part2()
{
    td_s32 ret = TD_SUCCESS;
    ret += sensor_os08a20_write_register(0x362d, 0x10);
    ret += sensor_os08a20_write_register(0x3660, 0xd3);
    ret += sensor_os08a20_write_register(0x3661, 0x06);
    ret += sensor_os08a20_write_register(0x3662, 0x00);
    ret += sensor_os08a20_write_register(0x3663, 0x28);
    ret += sensor_os08a20_write_register(0x3664, 0x0d);
    ret += sensor_os08a20_write_register(0x366a, 0x38);
    ret += sensor_os08a20_write_register(0x366b, 0xa0);
    ret += sensor_os08a20_write_register(0x366d, 0x00);
    ret += sensor_os08a20_write_register(0x366e, 0x00);
    ret += sensor_os08a20_write_register(0x3680, 0x00);
    ret += sensor_os08a20_write_register(0x36c0, 0x00);
    ret += sensor_os08a20_write_register(0x3701, 0x02);
    ret += sensor_os08a20_write_register(0x373b, 0x02);
    ret += sensor_os08a20_write_register(0x373c, 0x02);
    ret += sensor_os08a20_write_register(0x3736, 0x02);
    ret += sensor_os08a20_write_register(0x3737, 0x02);
    ret += sensor_os08a20_write_register(0x3705, 0x00);
    ret += sensor_os08a20_write_register(0x3706, 0x72);
    ret += sensor_os08a20_write_register(0x370a, 0x01);
    ret += sensor_os08a20_write_register(0x370b, 0x30);
    ret += sensor_os08a20_write_register(0x3709, 0x48);
    ret += sensor_os08a20_write_register(0x3714, 0x21);
    ret += sensor_os08a20_write_register(0x371c, 0x00);
    ret += sensor_os08a20_write_register(0x371d, 0x08);
    ret += sensor_os08a20_write_register(0x3740, 0x1b);
    ret += sensor_os08a20_write_register(0x3741, 0x04);
    ret += sensor_os08a20_write_register(0x375e, 0x0b);
    ret += sensor_os08a20_write_register(0x3760, 0x10);
    ret += sensor_os08a20_write_register(0x3776, 0x10);
    ret += sensor_os08a20_write_register(0x3781, 0x02);
    ret += sensor_os08a20_write_register(0x3782, 0x04);
    ret += sensor_os08a20_write_register(0x3783, 0x02);
    ret += sensor_os08a20_write_register(0x3784, 0x08);
    ret += sensor_os08a20_write_register(0x3785, 0x08);
    ret += sensor_os08a20_write_register(0x3788, 0x01);
    ret += sensor_os08a20_write_register(0x3789, 0x01);
    ret += sensor_os08a20_write_register(0x3797, 0x04);
    ret += sensor_os08a20_write_register(0x3762, 0x11);
    ret += sensor_os08a20_write_register(0x3800, 0x00);
    ret += sensor_os08a20_write_register(0x3801, 0x00);
    ret += sensor_os08a20_write_register(0x3802, 0x00);
    ret += sensor_os08a20_write_register(0x3803, 0x0c);
    ret += sensor_os08a20_write_register(0x3804, 0x0e);
    ret += sensor_os08a20_write_register(0x3805, 0xff);

    return ret;
}

static td_s32 sensor_os08a20_linear_8m30_12bit_init_part3()
{
    td_s32 ret = TD_SUCCESS;
    ret += sensor_os08a20_write_register(0x3806, 0x08);
    ret += sensor_os08a20_write_register(0x3807, 0x73);
    ret += sensor_os08a20_write_register(0x3808, 0x0f);
    ret += sensor_os08a20_write_register(0x3809, 0x00);
    ret += sensor_os08a20_write_register(0x380a, 0x08);
    ret += sensor_os08a20_write_register(0x380b, 0x88);
    ret += sensor_os08a20_write_register(0x380c, 0x08);
    ret += sensor_os08a20_write_register(0x380d, 0x18);
    ret += sensor_os08a20_write_register(0x380e, 0x09);
    ret += sensor_os08a20_write_register(0x380f, 0x0a);
    ret += sensor_os08a20_write_register(0x3813, 0x00);
    ret += sensor_os08a20_write_register(0x3814, 0x01);
    ret += sensor_os08a20_write_register(0x3815, 0x01);
    ret += sensor_os08a20_write_register(0x3816, 0x01);
    ret += sensor_os08a20_write_register(0x3817, 0x01);
    ret += sensor_os08a20_write_register(0x381c, 0x00);
    ret += sensor_os08a20_write_register(0x3820, 0x00);
    ret += sensor_os08a20_write_register(0x3821, 0x04);
    ret += sensor_os08a20_write_register(0x3823, 0x08);
    ret += sensor_os08a20_write_register(0x3826, 0x00);
    ret += sensor_os08a20_write_register(0x3827, 0x08);
    ret += sensor_os08a20_write_register(0x382d, 0x08);
    ret += sensor_os08a20_write_register(0x3832, 0x02);
    ret += sensor_os08a20_write_register(0x3833, 0x00);
    ret += sensor_os08a20_write_register(0x383c, 0x48);
    ret += sensor_os08a20_write_register(0x383d, 0xff);
    ret += sensor_os08a20_write_register(0x3d85, 0x0b);
    ret += sensor_os08a20_write_register(0x3d84, 0x40);
    ret += sensor_os08a20_write_register(0x3d8c, 0x63);
    ret += sensor_os08a20_write_register(0x3d8d, 0xd7);
    ret += sensor_os08a20_write_register(0x4000, 0xf8);
    ret += sensor_os08a20_write_register(0x4001, 0xeb);
    ret += sensor_os08a20_write_register(0x4004, 0x01);
    ret += sensor_os08a20_write_register(0x4005, 0x00);
    ret += sensor_os08a20_write_register(0x400a, 0x01);
    ret += sensor_os08a20_write_register(0x400f, 0xa0);
    ret += sensor_os08a20_write_register(0x4010, 0x12);
    ret += sensor_os08a20_write_register(0x4018, 0x00);
    ret += sensor_os08a20_write_register(0x4008, 0x02);
    ret += sensor_os08a20_write_register(0x4009, 0x0d);
    ret += sensor_os08a20_write_register(0x401a, 0x58);
    ret += sensor_os08a20_write_register(0x4050, 0x00);
    ret += sensor_os08a20_write_register(0x4051, 0x01);
    ret += sensor_os08a20_write_register(0x4028, 0x2f);

    return ret;
}

static td_s32 sensor_os08a20_linear_8m30_12bit_init_part4()
{
    td_s32 ret = TD_SUCCESS;
    ret += sensor_os08a20_write_register(0x4052, 0x00);
    ret += sensor_os08a20_write_register(0x4053, 0x80);
    ret += sensor_os08a20_write_register(0x4054, 0x00);
    ret += sensor_os08a20_write_register(0x4055, 0x80);
    ret += sensor_os08a20_write_register(0x4056, 0x00);
    ret += sensor_os08a20_write_register(0x4057, 0x80);
    ret += sensor_os08a20_write_register(0x4058, 0x00);
    ret += sensor_os08a20_write_register(0x4059, 0x80);
    ret += sensor_os08a20_write_register(0x430b, 0xff);
    ret += sensor_os08a20_write_register(0x430c, 0xff);
    ret += sensor_os08a20_write_register(0x430d, 0x00);
    ret += sensor_os08a20_write_register(0x430e, 0x00);
    ret += sensor_os08a20_write_register(0x4501, 0x18);
    ret += sensor_os08a20_write_register(0x4502, 0x00);
    ret += sensor_os08a20_write_register(0x4600, 0x00);
    ret += sensor_os08a20_write_register(0x4601, 0x20);
    ret += sensor_os08a20_write_register(0x4603, 0x01);
    ret += sensor_os08a20_write_register(0x4643, 0x00);
    ret += sensor_os08a20_write_register(0x4640, 0x01);
    ret += sensor_os08a20_write_register(0x4641, 0x04);
    ret += sensor_os08a20_write_register(0x4800, 0x64);
    ret += sensor_os08a20_write_register(0x4809, 0x2b);
    ret += sensor_os08a20_write_register(0x4813, 0x90);
    ret += sensor_os08a20_write_register(0x4817, 0x04);
    ret += sensor_os08a20_write_register(0x4833, 0x18);
    ret += sensor_os08a20_write_register(0x4837, 0x0b);
    ret += sensor_os08a20_write_register(0x483b, 0x00);
    ret += sensor_os08a20_write_register(0x484b, 0x03);
    ret += sensor_os08a20_write_register(0x4850, 0x7c);
    ret += sensor_os08a20_write_register(0x4852, 0x06);
    ret += sensor_os08a20_write_register(0x4856, 0x58);
    ret += sensor_os08a20_write_register(0x4857, 0xaa);
    ret += sensor_os08a20_write_register(0x4862, 0x0a);
    ret += sensor_os08a20_write_register(0x4869, 0x18);
    ret += sensor_os08a20_write_register(0x486a, 0xaa);
    ret += sensor_os08a20_write_register(0x486e, 0x03);
    ret += sensor_os08a20_write_register(0x486f, 0x55);
    ret += sensor_os08a20_write_register(0x4875, 0xf0);
    ret += sensor_os08a20_write_register(0x5000, 0x89);
    ret += sensor_os08a20_write_register(0x5001, 0x40);
    ret += sensor_os08a20_write_register(0x5004, 0x40);
    ret += sensor_os08a20_write_register(0x5005, 0x00);
    ret += sensor_os08a20_write_register(0x5180, 0x00);
    ret += sensor_os08a20_write_register(0x5181, 0x10);
    ret += sensor_os08a20_write_register(0x580b, 0x03);

    return ret;
}

static td_s32 sensor_os08a20_linear_8m30_12bit_init_part5()
{
    td_s32 ret = TD_SUCCESS;
    ret += sensor_os08a20_write_register(0x4d00, 0x03);
    ret += sensor_os08a20_write_register(0x4d01, 0xc9);
    ret += sensor_os08a20_write_register(0x4d02, 0xbc);
    ret += sensor_os08a20_write_register(0x4d03, 0xc6);
    ret += sensor_os08a20_write_register(0x4d04, 0x4a);
    ret += sensor_os08a20_write_register(0x4d05, 0x25);
    ret += sensor_os08a20_write_register(0x4700, 0x2b);
    ret += sensor_os08a20_write_register(0x4e00, 0x2b);
    return ret;
}

static void sensor_os08a20_linear_8m30_12bit_init()
{
    td_s32 ret = TD_SUCCESS;

    ret += sensor_os08a20_linear_8m30_12bit_init_part1();
    ret += sensor_os08a20_linear_8m30_12bit_init_part2();
    ret += sensor_os08a20_linear_8m30_12bit_init_part3();
    ret += sensor_os08a20_linear_8m30_12bit_init_part4();
    ret += sensor_os08a20_linear_8m30_12bit_init_part5();
    if (ret != TD_SUCCESS) {
        isp_err_trace("os08a20 write register failed!\n");
        return;
    }

    printf("========================================================================\n");
    printf("vi_pipe:%d,== os08a20 24Mclk 4M30fps(MIPI) 12bit linear init success! ==\n", 0);
    printf("========================================================================\n");
    return;
}
