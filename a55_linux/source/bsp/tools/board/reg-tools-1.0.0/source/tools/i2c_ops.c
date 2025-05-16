/*
 *
 * Copyright (c) 2012-2021 Hisilicon Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ctype.h>

#include <sys/mman.h>
#include <memory.h>
#include <linux/fb.h>
#include "securec.h"
#include "bsp_type.h"
#include "i2c_dev.h"
#include "strfunc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define READ_MIN_CNT 5
#define READ_MAX_CNT 8
#define WRITE_MIN_CNT 5
#define WRITE_MAX_CNT 7
#define INT_MAX 2147483647
#define I2C_NAME_SIZE 0X10

#define I2C_READ_STATUS_OK 2
#define I2C_MSG_CNT 2
#define BYTE_CNT_2 2
#define BIT_CNT_8 8


struct i2c_rdwr_args {
	unsigned int i2c_num;
	unsigned int dev_addr;
	unsigned int reg_addr;
	union {
		unsigned int data;
		unsigned int reg_addr_end;
	} w_r_union;
	unsigned int reg_width;
	unsigned int data_width;
	unsigned int reg_step;
};

static void print_r_usage(void)
{
	printf("usage: i2c_read <i2c_num> <dev_addr> <reg_addr> <reg_addr_end>"
		   "<reg_width> <data_width> <reg_step>. sample: \n");
	printf("\t\t0x1 0x56 0x0 0x10 2 2. \n");
	printf("\t\t0x1 0x56 0x0 0x10 2 2 2. \n");
	printf("\t\t0x1 0x56 0x0 0x10. default reg_width, data_width, reg_step is 1. \n");
}

static void print_w_usage(void)
{
	printf("usage: i2c_write <i2c_num> <dev_addr> <reg_addr> <value> <reg_width> <data_width>. sample:\n");
	printf("\t\t 0x1 0x56 0x0 0x28 2 2. \n");
	printf("\t\t 0x1 0x56 0x0 0x28. default reg_width and data_width is 1. \n");
}

#define I2C_READ_BUF_LEN 4
static int parse_args(int argc, char *argv[], struct i2c_rdwr_args *args)
{
	unsigned int *cur_arg = NULL;
	char *cmd_str = argv[0];
	char *tmp = NULL;
	unsigned long val;
	int i;

	for (tmp = cmd_str; *tmp != '\0';) {
		if (*tmp++ == '/')
			cmd_str = tmp;
	}

	if (strcmp(cmd_str, "i2c_read") == 0) {
		if ((argc < READ_MIN_CNT) || (argc > READ_MAX_CNT))
			return -1;
	} else if (strcmp(cmd_str, "i2c_write") == 0) {
		if ((argc < WRITE_MIN_CNT) || (argc > WRITE_MAX_CNT))
			return -1;
	} else {
		return -1;
	}

	cur_arg = (unsigned int *)args;
	for (i = 1; i < argc; i++) {
		if (str2number(argv[i], &val) != TD_SUCCESS)
			return -1;
		*cur_arg = (unsigned int)val;
		cur_arg++;
	}

	args->dev_addr >>= 1;

	if (strcmp(cmd_str, "i2c_read") == 0) {
		if (args->w_r_union.reg_addr_end < args->reg_addr) {
			printf("reg_addr_end < reg_addr error!\n");
			return -1;
		}

		if ((args->reg_addr % args->reg_step) || (args->w_r_union.reg_addr_end % args->reg_step)) {
			printf("((reg_addr or reg_addr_end) %% reg_step) != 0, error!\n");
			return -1;
		}
	}

	if ((args->reg_width != 1) && (args->reg_width != BYTE_CNT_2)) {
		printf("reg_width must be 1 or 2\n");
		return -1;
	}

	if ((args->data_width != 1) && (args->data_width != BYTE_CNT_2)) {
		printf("data_width must be 1 or 2\n");
		return -1;
	}
	return 0;
}

static int i2c_open(int *fd, struct i2c_rdwr_args args)
{
	char file_name[I2C_NAME_SIZE];
	if (sprintf_s(file_name, I2C_NAME_SIZE, "/dev/i2c-%u", args.i2c_num) < 0) {
		printf("sprintf_s error!\n");
		return -1;
	}

	*fd = open(file_name, O_RDWR);
	if (*fd < 0) {
		printf("Open %s error!\n", file_name);
		return -1;
	}
	return 0;
}

static int i2c_set_mode(int fd,  struct i2c_rdwr_args args)
{
	if (fd < 0)
		return -1;
	if (ioctl(fd, I2C_SLAVE_FORCE, args.dev_addr)) {
		printf("set i2c device address error!\n");
		return -1;
	}
	return 0;
}

static int i2c_ioc_init(struct i2c_rdwr_ioctl_data *rdwr,
			unsigned char *buf,
			size_t buf_size,
			struct i2c_rdwr_args args)
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

static int i2c_read_regs(int fd, struct i2c_rdwr_args args)
{
	unsigned int cur_addr;
	unsigned int data;
	unsigned char buf[I2C_READ_BUF_LEN];
	static struct i2c_rdwr_ioctl_data rdwr;
	static struct i2c_msg msg[I2C_MSG_CNT];

	rdwr.msgs = &msg[0];
	rdwr.nmsgs = (__u32)I2C_MSG_CNT;
	if (i2c_ioc_init(&rdwr, buf, sizeof(buf), args) != 0)
		return -1;

	for (cur_addr = args.reg_addr; cur_addr <= args.w_r_union.reg_addr_end; cur_addr += args.reg_step) {
		if (args.reg_width == BYTE_CNT_2) {
			buf[0] = (cur_addr >> BIT_CNT_8) & 0xff;
			buf[1] = cur_addr & 0xff;
		} else {
			buf[0] = cur_addr & 0xff;
		}

		if (ioctl(fd, I2C_RDWR, &rdwr) != I2C_READ_STATUS_OK) {
			printf("CMD_I2C_READ error!\n");
			return -1;
		}

		if (args.data_width == BYTE_CNT_2)
			data = buf[1] | (buf[0] << BIT_CNT_8);
		else
			data = buf[0];

		printf("0x%x: 0x%x\n", cur_addr, data);
	}

	return 0;
}

static int i2c_write_reg(int fd, struct i2c_rdwr_args args)
{
	unsigned char buf[4];
	int index = 0;

	if (args.reg_width == BYTE_CNT_2) {
		buf[index] = (args.reg_addr >> BIT_CNT_8) & 0xff;
		index++;
		buf[index] = args.reg_addr & 0xff;
		index++;
	} else {
		buf[index] = args.reg_addr & 0xff;
		index++;
	}

	if (args.data_width == BYTE_CNT_2) {
		buf[index] = (args.w_r_union.data >> BIT_CNT_8) & 0xff;
		index++;
		buf[index] = args.w_r_union.data & 0xff;
	} else {
		buf[index] = args.w_r_union.data & 0xff;
	}

	if (write(fd, buf, (args.reg_width + args.data_width)) < 0) {
		printf("i2c write error!\n");
		return -1;
	}

	return 0;
}

int i2c_read(int argc, char *argv[])
{
	int retval = 0;
	int fd = -1;
	struct i2c_rdwr_args args = {
		.i2c_num = 0,
		.dev_addr = 0,
		.reg_addr = 0,
		.w_r_union.reg_addr_end = 0,
		.reg_width = 1,
		.data_width = 1,
		.reg_step = 1
	};

	if (parse_args(argc, argv, &args) != 0) {
		print_r_usage();
		return -1;
	}

	printf("i2c_num:0x%x, dev_addr:0x%x; reg_addr:0x%x; reg_addr_end:0x%x;"
		"reg_width: %u; data_width: %u; reg_step: %u. \n\n",
		args.i2c_num, args.dev_addr << 1, args.reg_addr, args.w_r_union.reg_addr_end,
		args.reg_width, args.data_width, args.reg_step);

	if (i2c_open(&fd, args) != 0)
		return -1;

	if (i2c_set_mode(fd, args) != 0) {
		retval = -1;
		goto end;
	}

	if (i2c_read_regs(fd, args) != 0) {
		retval = -1;
		goto end;
	}

	retval = 0;
end:
	close(fd);
	return retval;
}

int i2c_write(int argc, char *argv[])
{
	int retval = 0;
	int fd = -1;

	struct i2c_rdwr_args args = {
		.i2c_num = 0,
		.dev_addr = 0,
		.reg_addr = 0,
		.w_r_union.data = 0,
		.reg_width = 1,
		.data_width = 1,
	};

	if (parse_args(argc, argv, &args) != 0) {
		print_w_usage();
		return -1;
	}

	printf("i2c_num:0x%x, dev_addr:0x%x; reg_addr:0x%x; data:0x%x;"
		"reg_width: %u; data_width: %u.\n",
		args.i2c_num, args.dev_addr << 1, args.reg_addr, args.w_r_union.data,
		args.reg_width, args.data_width);

	if (i2c_open(&fd, args) != 0)
		return -1;

	if (i2c_set_mode(fd, args) != 0) {
		retval = -1;
		goto end;
	}

	if (i2c_write_reg(fd, args) != 0) {
		retval = -1;
		goto end;
	}

	retval = 0;
end:
	close(fd);
	return retval;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
