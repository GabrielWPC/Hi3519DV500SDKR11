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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <asm-generic/ioctl.h>
#include <linux/fb.h>
#include "securec.h"
#include "bsp_type.h"
#include "bsp_spi.h"
#include "utils/strfunc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define READ_MIN_CNT 5
#define READ_MAX_CNT 11
#define WRITE_MIN_CNT 6
#define WRITE_MAX_CNT 11
#define INT_MAX 2147483647
#define SPI_NAME_SIZE 0X20

#define SPI_SPEED_HZ 2000000
#define SPI_BIT_PER_WORD 8

#define BYTE_CNT_2 2
#define BIT_CNT_8 8

struct ssp_trans_args {
	unsigned int spi_num;
	unsigned int csn;
	unsigned int dev_addr;
	unsigned int reg_addr;
	union {
		unsigned int data;
		unsigned int num_reg;
	} w_r_union;
	unsigned int dev_width;
	unsigned int reg_width;
	unsigned int data_width;
	unsigned int reg_order;
	unsigned int data_order;
};

static void print_r_usage(void)
{
	printf("Usage: ssp_read <spi_num> <csn> <dev_addr> <reg_addr> [num_reg] [dev_width] "
		"[reg_width] [data_width] [reg_order] [data_order] .\n");
	printf("\tnum_reg, dev_width, reg_width, data_width, reg_order and data_order"
		"can be omitted. The defaults are 0x1.\n");
	printf("eg:\n");
	printf("\tssp_read 0x0 0x0 0x2 0x0 0x10 0x1 0x1 0x1 0x1 0x1.\n");
	printf("\tssp_read 0x0 0x0 0x2 0x0. The default num_reg, dev_width, reg_width, "
		"data_width, reg_order and data_order are 0x1.\n");
	return;
}

static void print_w_usage(void)
{
	printf("Usage: ssp_write <spi_num> <csn> <dev_addr> <reg_addr> <data> [dev_width] "
		"[reg_width] [data_width] [reg_order] [data_order].\n");
	printf("\tdev_width, reg_width, data_width, reg_order and data_order can be omitted. "
		"The defaults are 0x1.\n");
	printf("eg:\n");
	printf("\tssp_write 0x0 0x0 0x2 0x0 0x65 0x1 0x1 0x1 0x1 0x1.\n");
	printf("\tssp_write 0x0 0x0 0x2 0x0 0x65. Default dev_width, reg_width, data_width, "
		"reg_order and data_order are 0x1.\n");
	return;
}

#define SSP_BUF_LEN 0x10
static int parse_args(int argc, char *argv[], struct ssp_trans_args *args)
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

	if (strcmp(cmd_str, "ssp_read") == 0) {
		if ((argc < READ_MIN_CNT) || (argc > READ_MAX_CNT))
			return -1;
	} else if (strcmp(cmd_str, "ssp_write") == 0) {
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

	if ((args->dev_width != 1) && (args->dev_width != BYTE_CNT_2)) {
		printf("dev_width must be 1 or 2\n");
		return -1;
	}
	if ((args->reg_width != 1) && (args->reg_width != BYTE_CNT_2)) {
		printf("reg_width must be 1 or 2\n");
		return -1;
	}
	if ((args->data_width != 1) && (args->data_width != BYTE_CNT_2)) {
		printf("data_width must be 1 or 2\n");
		return -1;
	}
	if ((args->reg_order != 1) && (args->reg_order != 0)) {
		printf("reg_order must be 1 or 0\n");
		return -1;
	}
	if ((args->data_order != 1) && (args->data_order != 0)) {
		printf("data_order must be 1 or 0\n");
		return -1;
	}
	return 0;
}

static int ssp_open(int *fd, struct ssp_trans_args args)
{
	char file_name[SPI_NAME_SIZE];
	if (sprintf_s(file_name, SPI_NAME_SIZE, "/dev/spidev%u.%u",
				  args.spi_num, args.csn) < 0) {
		printf("sprintf_s error!\n");
		return -1;
	}

	*fd = open(file_name, 0);
	if (*fd < 0) {
		printf("Open %s error!\n", file_name);
		return -1;
	}
	return 0;
}

static int ssp_set_mode(int fd)
{
	int spi_mode = SPI_MODE_3 | SPI_LSB_FIRST;
	if (fd < 0)
		return -1;
	if (ioctl(fd, SPI_IOC_WR_MODE, &spi_mode)) {
		printf("set spi mode fail!\n");
		return -1;
	}
	return 0;
}

static int ssp_ioc_init(struct spi_ioc_transfer *mesg,
			unsigned char *buf,
			size_t buf_size,
			struct ssp_trans_args args)
{
	if (memset_s(mesg, sizeof(struct spi_ioc_transfer),
			0, sizeof(struct spi_ioc_transfer)) != EOK) {
		printf("memset_s fail!\n");
		return -1;
	}
	if (memset_s(buf, buf_size, 0, SSP_BUF_LEN) != EOK) {
		printf("memset_s fail!\n");
		return -1;
	}
	mesg->tx_buf = (uintptr_t)buf;
	mesg->rx_buf = (uintptr_t)buf;
	mesg->len = args.dev_width + args.reg_width + args.data_width;
	mesg->speed_hz = SPI_SPEED_HZ;
	mesg->bits_per_word = BIT_CNT_8;
	mesg->cs_change = 1;
	return 0;
}

static void ssp_fill_tx_data(struct spi_ioc_transfer *mesg, struct ssp_trans_args args)
{
	unsigned char *buf = (unsigned char *)mesg->tx_buf;
	int index = 0;

	if (args.dev_width == 1) {
		*(__u8 *)(&buf[index]) = args.dev_addr & 0xff;
		index++;
	} else {
		*(__u16 *)(&buf[index]) = args.dev_addr & 0xffff;
		index += BYTE_CNT_2;
	}
	if (args.reg_width == 1) {
		*(__u8 *)(&buf[index]) = args.reg_addr & 0xff;
		index++;
	} else {
		if (args.reg_order) {
			*(__u16 *)(&buf[index]) = args.reg_addr & 0xffff;
		} else {
			*(__u8 *)(&buf[index]) = (args.reg_addr >> BIT_CNT_8)  & 0xff;
			*(__u8 *)(&buf[index + 1]) = args.reg_addr & 0xff;
		}
		index += BYTE_CNT_2;
	}
	if (args.data_width == 1) {
		*(__u8 *)(&buf[index]) = args.w_r_union.data & 0xff;
	} else {
		if (args.data_order) {
			*(__u16 *)(&buf[index]) = args.w_r_union.data & 0xffff;
		} else {
			*(__u8 *)(&buf[index]) = (args.w_r_union.data >> BIT_CNT_8)  & 0xff;
			*(__u8 *)(&buf[index + 1]) = args.w_r_union.data   & 0xff;
		}
	}
}

static unsigned int ssp_read_rx_data(const struct spi_ioc_transfer *mesg,
				struct ssp_trans_args args)
{
	unsigned char *buf = (unsigned char *)mesg->rx_buf;
	unsigned int data;
	unsigned int index = args.dev_width + args.reg_width;

	if (args.data_width == 1) {
		data = *(__u8 *)(&buf[index]);
	} else {
		if (args.data_order) {
			data = *(__u16 *)(&buf[index]);
		} else {
			data = *(__u8 *)(&buf[index]) << BIT_CNT_8;
			data += (*(__u8 *)(&buf[index + 1]));
		}
	}
	return data;
}

static int ssp_read_regs(int fd, struct spi_ioc_transfer mesg[], struct ssp_trans_args args)
{
	unsigned int i;
	unsigned int num_reg;

	/* retain the number of registers to read and set data to zero */
	num_reg = args.w_r_union.num_reg;
	args.w_r_union.data = 0;

	printf("====reg_addr:0x%04x====", args.reg_addr);
	for (i = 0; i < num_reg; i++) {
		unsigned int data;
		ssp_fill_tx_data(&mesg[0], args);
		if (ioctl(fd, spi_ioc_message(1), mesg) != (int)mesg[0].len) {
			printf("SPI_IOC_MESSAGE error \n");
			return -1;
		}
		data = ssp_read_rx_data(&mesg[0], args);
		args.reg_addr++;

		if ((i % 0x10) == 0)
			printf("\n0x%04x:  ", i);

		printf("0x%04x ", data);
	}

	return 0;
}

static int ssp_write_reg(int fd, struct spi_ioc_transfer mesg[], struct ssp_trans_args args)
{
	/* fill transmit data to mesg[0].tx_buf */
	ssp_fill_tx_data(&mesg[0], args);

	if (ioctl(fd, spi_ioc_message(1), mesg) != (int)mesg[0].len) {
		printf("SPI_IOC_MESSAGE error \n");
		return -1;
	}

	return 0;
}

int ssp_read(int argc, char *argv[])
{
	int retval = 0;
	int fd = -1;
	struct spi_ioc_transfer mesg[1];
	unsigned char buf[SSP_BUF_LEN];
	struct ssp_trans_args args = {
		.spi_num = 0,
		.csn = 0,
		.dev_addr = 0,
		.reg_addr = 0,
		.w_r_union.num_reg = 1,
		.dev_width = 1,
		.reg_width = 1,
		.data_width = 1,
		.reg_order = 1,
		.data_order = 1
	};

	if (parse_args(argc, argv, &args) != 0) {
		print_r_usage();
		return -1;
	}

	printf("spi_num:%u, csn:%u\n"
		"dev_addr:0x%04x, reg_addr:0x%04x, num_reg:%u, dev_width:%u, "
		"reg_width:%u, data_width:%u, reg_order: %u, data_order: %u\n",
		args.spi_num, args.csn,
		args.dev_addr, args.reg_addr, args.w_r_union.num_reg, args.dev_width,
		args.reg_width, args.data_width, args.reg_order, args.data_order);

	if (ssp_open(&fd, args) != 0)
		return -1;

	if (ssp_set_mode(fd) != 0) {
		retval = -1;
		goto end;
	}

	if (ssp_ioc_init(&mesg[0], buf, sizeof(buf), args) != 0) {
		retval = -1;
		goto end;
	}

	if (ssp_read_regs(fd, mesg, args) != 0) {
		retval = -1;
		goto end;
	}

	printf("\n[END]\n");
	retval = 0;
end:
	close(fd);
	return retval;
}

int ssp_write(int argc, char *argv[])
{
	int retval = 0;
	int fd = -1;
	struct spi_ioc_transfer mesg[1];
	unsigned char buf[SSP_BUF_LEN];
	struct ssp_trans_args args = {
		.spi_num = 0,
		.csn = 0,
		.dev_addr = 0,
		.reg_addr = 0,
		.w_r_union.data = 1,
		.dev_width = 1,
		.reg_width = 1,
		.data_width = 1,
		.reg_order = 1,
		.data_order = 1
	};

	if (parse_args(argc, argv, &args) != 0) {
		print_w_usage();
		return -1;
	}

	printf("spi_num:%u, csn:%u\n"
		"dev_addr:0x%04x, reg_addr:0x%04x, data:0x%04x, dev_width:%u, reg_width:%u, "
		"data_width:%u, reg_order: %u, data_order: %u\n",
		args.spi_num, args.csn,
		args.dev_addr, args.reg_addr, args.w_r_union.data, args.dev_width, args.reg_width,
		args.data_width, args.reg_order, args.data_order);

	if (ssp_open(&fd, args) != 0)
		return -1;

	if (ssp_set_mode(fd) != 0) {
		retval = -1;
		goto end;
	}

	if (ssp_ioc_init(&mesg[0], buf, sizeof(buf), args) != 0) {
		retval = -1;
		goto end;
	}

	if (ssp_write_reg(fd, mesg, args) != 0) {
		retval = -1;
		goto end;
	}

	printf("\n[END]\n");
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

