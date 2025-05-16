/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "platform.h"
#include "types.h"
#include "common.h"
#include "lib.h"
#include "securecutil.h"
#include "uart.h"
#include "burn_protocol.h"
#include "board_type.h"
#include "crc.h"
#include "share_drivers.h"

static bool calc_check_crc(frame_info *frame, u32 pos, u8 cr)
{
	if (pos <= frame->len - CRC16_LEN) {
		/* calculated crc */
		frame->calc_crc = cal_crc_perbyte(cr, frame->calc_crc);
	} else {
		/* received crc */
		frame->frame_crc |= (u16)(cr << (BIT_CNT_8 * (frame->len - pos)));
		if (pos == frame->len)
			/* compare calcuated crc and received crc in the last */
			return (frame->calc_crc == frame->frame_crc);
	}
	return TRUE;
}

static int uart_wait_data(int status)
{
	u32 timer_out = PROC_TIME_OUT * timer_get_divider();
	/* initialize timer controller */
	timer_start();
	/* does uart controller have any data? */
	while (!serial_tstc(REG_BASE_SERIAL0)) {
		/* time out. */
		if (timer_get_val() >= timer_out) {
			if (status == OCR_STATUS_ERROR)
				return OCR_STATUS_ERROR;
			else
				return OCR_STATUS_SKIP;
		}
	}
	return	OCR_STATUS_OK;
}

static int parse_board_type_para(frame_info *frame, u32 pos, u8 cr)
{
	switch (pos) {
	case PROC_BYTE_HEAD:
		frame->len = FRAME_LENGTH_FILE;
		break;
	case PROC_BYTE_SEQ:
		frame->seq = cr;
		if (frame->seq != 0)
			return -1;
		break;
	case PROC_BYTE_SEQF:
		if (cr != (u8) ~frame->seq)
			return -1;
		break;
	default:
		break;
	}
	if (!calc_check_crc(frame, pos, cr))
		return -1;
	return 0;
}

int send_board_type_to_uart(unsigned int type)
{
	u8 cr;
	u32 pos = FRAME_HEAD;
	frame_info frame;
	unsigned char board_type_frame_buf[BOARD_TYPE_FRAME_LEN];
	int i;
	if (memset_s(&frame, sizeof(frame_info), 0, sizeof(frame_info)) != EOK)
		return EXT_SEC_SUCCESS;
	while (1) {
		if (uart_wait_data(OCR_STATUS_OK) != OCR_STATUS_OK)
			return EXT_SEC_SUCCESS;
		serial_getc(REG_BASE_SERIAL0, &cr);
		if (pos == FRAME_HEAD && cr != XBOARD)
			continue;
		if (parse_board_type_para(&frame, pos, cr)) {
			pos = FRAME_HEAD;
			continue;
		}
		if (pos == frame.len) {
			(void)build_board_type_frame(type, board_type_frame_buf, BOARD_TYPE_FRAME_LEN);
			for (i = 0; i < BOARD_TYPE_FRAME_LEN; i++) {
				serial_putc(REG_BASE_SERIAL0, board_type_frame_buf[i]);
			}
			return EXT_SEC_SUCCESS;
		}
		/* increase the index within the frame */
		pos++;
	}
	return EXT_SEC_SUCCESS;
}

