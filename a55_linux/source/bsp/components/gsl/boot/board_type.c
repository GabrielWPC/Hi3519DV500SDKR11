/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include "types.h"
#include "platform.h"
#include "board_type.h"
#include "burn_protocol.h"
#include "crc.h"
#include "lib.h"
#include "flash_map.h"
#include "share_drivers.h"

int build_board_type_frame(unsigned int type, unsigned char *frame, unsigned int frame_len)
{
	int i;
	unsigned short crc = 0;
	if (frame == NULL || frame_len < BOARD_TYPE_FRAME_LEN)
		return EXT_SEC_FAILURE;
	frame[0] = XBOARD;
	frame[1] = 1;
	frame[2] = 0;
	frame[3] = FRAME_LEN;
	frame[4] = (type >> OFFSET_24_BITS) & 0xff;
	frame[5] = (type >> OFFSET_16_BITS) & 0xff;
	frame[6] = (type >> OFFSET_8_BITS) & 0xff;
	frame[7] = type & 0xff;
	for (i = 0; i < BOARD_TYPE_PAYLOAD_LEN; i++)
		crc = cal_crc_perbyte(frame[i], crc);
	frame[8] = (crc >> OFFSET_8_BITS) & 0xff;
	frame[9] = crc & 0xff;
	frame[10] = (unsigned char)ACK;
	return EXT_SEC_SUCCESS;
}

unsigned int get_board_param_index()
{
	unsigned int board_index = readl(SYSCTRL_REG + SYS_STAT_REG);
	board_index = (board_index >> OFFSET_27_BITS) & 0x7; // bit[27:29]
	log_serial_puts((const s8 *)"\r\nboot table board switch:0x");
	serial_put_hex(board_index);
	return board_index;
}

