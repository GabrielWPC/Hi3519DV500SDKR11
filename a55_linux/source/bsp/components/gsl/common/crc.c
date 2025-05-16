/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include "crc.h"

const unsigned short g_crc_ta[16]__attribute__((aligned(8))) = { \
						0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, \
						0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef \
						};
unsigned short cal_crc_perbyte(unsigned char byte, unsigned short crc)
{
	unsigned char  da;
	da = ((unsigned char)(crc >> BIT_CNT_8)) >> BIT_CNT_4;
	crc <<= BIT_CNT_4;
	crc ^= (unsigned short)g_crc_ta[da ^ (byte >> BIT_CNT_4)];
	da = ((unsigned char)(crc >> BIT_CNT_8)) >> BIT_CNT_4;
	crc <<= BIT_CNT_4;
	crc ^= (unsigned short)g_crc_ta[da ^ (byte & 0x0f)];
	return (crc);
}

