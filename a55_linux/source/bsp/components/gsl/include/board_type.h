/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __BOARD_TYPE_H_
#define __BOARD_TYPE_H_

#define FRAME_LEN    4

#define OFFSET_8_BITS    8
#define OFFSET_16_BITS   16
#define OFFSET_24_BITS   24
#define OFFSET_27_BITS   27

#define BOARD_TYPE_FRAME_LEN   11
#define BOARD_TYPE_PAYLOAD_LEN  8
#define BOOT_TABLE_MAGIC 0x2b8c6e1a1a6e8c2b
#define BOOT_TABLE_VERSION_LEN 16
#define BOOT_TABLE_TIME_LEN 32
#define BOOT_TABLE_NAME_LEN 128
#define HEADINFO_LENTH  0xB8 /* magic(8bytes) + ver(16bytes) + time(32bytes) + table name(128bytes) */

typedef struct {
	unsigned long magic;
	char ddr_table_version[BOOT_TABLE_VERSION_LEN];
	char ddr_table_gen_time[BOOT_TABLE_TIME_LEN];
	char ddr_table_name[BOOT_TABLE_NAME_LEN];
} ddr_table_info;

int build_board_type_frame(unsigned int type, unsigned char *frame, unsigned int frame_len);
unsigned int get_board_param_index();

#endif /* __BOARD_TYPE_H_ */
