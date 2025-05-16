/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __UART_DRV_H_
#define __UART_DRV_H_

#define IN
#define OUT

#ifndef CFG_EDA_VERIFY
#define PROC_TIME_OUT       10  /* timeout threshold  ms */
#else
#define PROC_TIME_OUT       1   /* for eda */
#endif

typedef struct {
	u32  file_type;		/* File type(RAM/USB) */
	u32  file_address;	/* The write addr for downloaded file */
	u32  file_length;	/* The length for downloaded file */
	u32  total_frame;	/* The total frame number for the file */
	u32  count_frame;	/* Current frame number been transfered */
	u8   exp_frame_idx;	/* The next frame index to expect */

	void (*SubFunction)(void);	/* The function pointer for the downloaded file */
} data_handle_t;

typedef struct {
	u16 frame_crc;
	u16 calc_crc;
	u16 len;
	u8  seq;
	u8 *write_addr;
} frame_info;

extern unsigned int uart_inited;

bool self_boot_check();
void uart_proc_first_section(void *dest, size_t count);
int send_board_type_to_uart(unsigned int type);
#endif
