/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __BURN_PROTOCOL_H__
#define __BURN_PROTOCOL_H__

#define XSTART  0xFE 		/* START frame head byte */
#define XFILE   0xFE		/* FILE frame head byte */
#define XDATA   0xDA		/* DATA frame head byte */
#define XBOARD  0xCE		/* BOARD TYPE frame head byte */
#define XEOT    0xED		/* EOT frame head byte */

#define ACK     0xAA		/* UE response byte ACK */
#define NAK     0x55		/* UE response byte NAK */

#define LOAD_RAM    0x01	/* file transfer type: RAM init file */
#define LOAD_USB    0x02	/* fiel transfer type: USB load file */

#define XUSER_DATA_LENGTH   1024	/* Data length of data frame */

#define FRAME_HEAD          0		/* The head of frame */
#define FRAME_LENGTH_FILE   13		/* The length of file frame(start from 0) */
#define FRAME_LENGTH_DATA   (XUSER_DATA_LENGTH + 4)	/* The length of standard data frame(start from 0) */
#define FRAME_LENGTH_EOT    4		/* The length of standard EOT frame(start from 0) */

#define PROC_BYTE_HEAD          0	/* The offset of frame head byte */
#define PROC_BYTE_SEQ           1	/* The offset of seq byte */
#define PROC_BYTE_SEQF          2	/* The offset of inverse seq byte */
#define PROC_BYTE_FILE_TYPE     3	/* The offset of file type byte */
#define PROC_BYTE_FILE_LENG0    4	/* The offset of file length byte */
#define PROC_BYTE_FILE_LENG1    5	/* The offset of file length byte */
#define PROC_BYTE_FILE_LENG2    6	/* The offset of file length byte */
#define PROC_BYTE_FILE_LENG3    7	/* The offset of file length byte */
#define PROC_BYTE_FILE_ADDR0    8	/* The offset of file addr byte */
#define PROC_BYTE_FILE_ADDR1    9	/* The offset of file addr byte */
#define PROC_BYTE_FILE_ADDR2    10	/* The offset of file addr byte */
#define PROC_BYTE_FILE_ADDR3    11	/* The offset of file addr byte */

#define OCR_STATUS_OK       0x01	/* protocol receive status: no error */
#define OCR_STATUS_ERROR    0x02	/* protocol receive status: frame error */
#define OCR_STATUS_SKIP     0x03	/* protocol receive status: data error, need to clear */
#define OCR_STATUS_FIN      0x04	/* The file has been download completely */

#endif