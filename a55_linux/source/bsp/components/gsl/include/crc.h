/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __CRC_H
#define __CRC_H

#define CRC16_LEN		2
#define CRC16_LEN		2
#define BIT_CNT_4	4
#define BIT_CNT_8	8

unsigned short cal_crc_perbyte(unsigned char byte, unsigned short crc);

#endif  /* __CRC_H */
