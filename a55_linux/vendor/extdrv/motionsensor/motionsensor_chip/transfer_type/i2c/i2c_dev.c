/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description : i2c_dev.c
 * Author : ISP SW
 * Create : 2018-7-30
 * Version : Initial Draft
 */

/* include <liteos/i2c.h> */

//#include <asm/uaccess.h>
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
#include <linux/i2c.h>
#include <linux/module.h>
#include "ot_osal.h"
#include "ot_common.h"
#include "securec.h"
#include "i2c_dev.h"
#include "motionsensor.h"
#include "icm42670.h"
struct i2c_client g_i2c_client_obj; /* i2c control struct */
#define SLAVE_ADDR     0x34         /* i2c dev addr */
#define SLAVE_REG_ADDR 0x300f       /* i2c dev register */
#define I2C_INDEX_0 2
#ifdef TRANSFER_I2C
static struct i2c_board_info g_ot_icm42670_info = {
    I2C_BOARD_INFO("ICM42670", IMU_DEV_ADDR),
};
#endif
/* client initial */
static struct i2c_client *icm42670_client = NULL;
td_s32 i2c_client_init(struct i2c_client *ot_i2c_client)
{
   printk("i2c_client_init 1\n");
   struct i2c_adapter *i2c_adap = NULL;

    i2c_adap = i2c_get_adapter(I2C_INDEX_0);
    if (i2c_adap == NULL) {
        printk("i2c%d get adapter failed.\n", I2C_INDEX_0);
        return -1;
    }
	printk("i2c_client_init 2\n");
    icm42670_client = i2c_new_client_device(i2c_adap, &g_ot_icm42670_info);
    if (icm42670_client == NULL) {
        printk("i2c%d get new device failed.\n", I2C_INDEX_0);
        return -1;
    }
	printk("i2c_client_init 3\n");
    i2c_put_adapter(i2c_adap);
	printk("i2c_client_init 4\n");
    return 0;
}
static void i2c_client_exit(struct i2c_client *ot_i2c_client)
{
    if (ot_i2c_client != NULL) {
        i2c_unregister_device(ot_i2c_client);
        ot_i2c_client = NULL;
    }
}


static td_s32 sample_i2c_write(struct i2c_client *ot_i2c_client, td_u8 reg_addr, const td_u8 *reg_data, td_u32 cnt)
{
    td_s32 ret;
	//printk("sample_i2c_write 1\n");
    /* struct i2c_client * i2c_client0 is & g_i2c_client_obj */
    td_char buf[4] = { 0 }; /* 4 buf */
    /* i2c_client_init */
	//i2c_client_init(ot_i2c_client);
    buf[0] = reg_addr & 0xff;
    ret = memcpy_s(&buf[1], cnt, reg_data, cnt);
    if (ret != EOK) {
        return ret;
    }
	//printk("sample_i2c_write 2\n");
if(icm42670_client == NULL)
{
	printk("sample_i2c_write 2222\n");
	//i2c_client_init(ot_i2c_client);
	
	return -1;
}
    /* buf[1] is (SLAVE_REG_ADDR >> 8) & 0xff */
    /* buf[2] is 0x03   write value to i2c */
    /* call I2C standard function drv to write */
    ret = i2c_master_send(icm42670_client, buf, cnt + 1);
//i2c_client_exit(ot_i2c_client);
//printk("sample_i2c_write 3\n");
    return 0;
}

static td_s32 sample_i2c_read(struct i2c_client *ot_i2c_client, td_u8 reg_addr, td_u8 *reg_data, td_u32 cnt)
{
    unsigned char tmp_buf0[4]; /* 4: buf len */
    unsigned char tmp_buf1[4]; /* 4: buf len */
    int ret;
    int ret_data = 0xFF;
    int idx = 0;
    static struct i2c_client client;
    static struct i2c_msg msg[2]; /* 2: msg len */

    if (icm42670_client == NULL) {
        return -1;
    }

    ret = memcpy_s(&client, sizeof(client), icm42670_client, sizeof(*icm42670_client));
    if (ret != EOK) {
        return -1;
    }
int i=0;
td_u8 tmp=0;
	for(i=0;i<cnt;i++)
{
	tmp=reg_addr+i;
    msg[0].addr = client.addr;
    msg[0].flags = client.flags & I2C_M_TEN;
    msg[0].len = 1;
    msg[0].buf = &tmp;



    msg[1].addr = client.addr;
    msg[1].flags = client.flags & I2C_M_TEN;
    msg[1].flags |= I2C_M_RD;
    msg[1].len = 1;
    msg[1].buf = reg_data+i;
//printk("sample_i2c_read 1\n");
ret = i2c_transfer(client.adapter, msg, 2); 
//printk("reg_data[%d]=%xret=%d cnt=%d\n",i,reg_data[i],ret,cnt);
}
//printk("reg_data[0]=%x reg_data[1]=%x reg_data[2]=%xret=%d\n",reg_data[0],reg_data[1],reg_data[2],ret);
//printk("sample_i2c_read 2\n");

    return 0;
}
#if 0
{
    td_s32 ret = TD_SUCCESS;
	printk("sample_i2c_read 1\n");
    /* struct i2c_client *i2c_client0 is & g_i2c_client_obj */
   // struct i2c_rdwr_ioctl_data rdwr;
    struct i2c_msg msg[2]; /* 2 msg */
    td_u8 recvbuf[4]; /* 4 bytes */
    (td_void)memset_s(recvbuf, sizeof(recvbuf), 0x0, sizeof(recvbuf));
    /* i2c_client_init */
//i2c_client_init(ot_i2c_client);
printk("sample_i2c_read 2\n");
if(icm42670_client == NULL)
{
	printk("sample_i2c_read 2222\n");
	//i2c_client_init(ot_i2c_client);
	
	return -1;
}
	printk("sample_i2c_read 3\n");
    msg[0].addr = icm42670_client->addr;
    msg[0].flags = icm42670_client->flags & I2C_M_TEN;
    msg[0].len = 1;
    msg[0].buf = &reg_addr;
    msg[1].addr = icm42670_client->addr;
    msg[1].flags = icm42670_client->flags & I2C_M_TEN;
    msg[1].flags |= I2C_M_RD;
    msg[1].len = cnt;
    msg[1].buf = reg_data;
    /* rdwr.msgs = &msg[0] */
   // rdwr.nmsgs = 2; /* 2 msg */
    recvbuf[0] = SLAVE_REG_ADDR & 0xff;
    recvbuf[1] = (SLAVE_REG_ADDR >> 8) & 0xff; /* 8 bits */
printk("sample_i2c_read 31\n");
    bsp_i2c_transfer(icm42670_client->adapter, msg, 2);
printk("sample_i2c_read 4\n");
//i2c_client_exit(ot_i2c_client);
    /* dprintf("val is 0x%x\n",recvbuf[0])  buf[0] save the value read from i2c dev */
    return ret;
}
#endif
td_u8 motionsersor_i2c_write(struct i2c_client *ot_i2c_client,
                             td_u8 reg_addr, const td_u8 *reg_data, td_u32 cnt)
{
    return sample_i2c_write(ot_i2c_client, reg_addr, reg_data, cnt);
}

td_u8 motionsersor_i2c_read(struct i2c_client *ot_i2c_client,
                            td_u8 reg_addr, td_u8 *reg_data, td_u32 cnt)
{
    return sample_i2c_read(ot_i2c_client, reg_addr, reg_data, cnt);
}
