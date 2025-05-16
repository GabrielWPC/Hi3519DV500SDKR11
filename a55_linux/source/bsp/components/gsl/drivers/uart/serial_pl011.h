/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __SERIAL_PL011_H__
#define __SERIAL_PL011_H__
#include <types.h>

#define DEFAULT_BAUDRATE    		     115200
#define BAUDRATE_380400     		     380400
#define BAUDRATE_921600     		     921600
#define BAUDRATE_2M         		     2000000
#define BAUDRATE_3M         		     3000000
#define BAUDRATE_4M         		     4000000

#define DATA_BIT_8 			     8

#define UART_MULTIPLE_10 		     10

#define UART_5_DATA_BITS                     5
#define UART_6_DATA_BITS                     6
#define UART_7_DATA_BITS                     7
#define UART_8_DATA_BITS                     8

#define UART_1_STOP_BIT                      1
#define UART_2_STOP_BITS                     2

#define UART_NONE_PARITY                     0
#define UART_ODD_PARITY                      1
#define UART_EVEN_PARITY                     2

#define THRESHOLD_ONE_EIGHT_FIFO             0
#define THRESHOLD_ONE_QUARTER_FIFO           1
#define THRESHOLD_HARF_FIFO                  2
#define THRESHOLD_THREE_QUARTERS_FIFO        3
#define THRESHOLD_SEVEN_EIGHTS_FIFO          4

#define FLOW_CTRL_NONE                       0
#define FLOW_CTRL_RTS_CTS                    1
#define FLOW_CTRL_RTS_ONLY                   2
#define FLOW_CTRL_CTS_ONLY                   3


#define OFFSET_3_BITS                        3
#define OFFSET_4_BITS                        4
#define OFFSET_5_BITS                        5
#define OFFSET_6_BITS                        6
#define OFFSET_12_BITS                       12
#define OFFSET_13_BITS                       13
#define OFFSET_14_BITS                       14
#define OFFSET_15_BITS                       15
#define OFFSET_28_BITS                       28

#define TIMEOUT_UART                         2000000

typedef struct {
	u32 baudrate;
	u32 uart_clk;
	u8 databit;
	u8 stopbit;
	u8 parity;
	u8 burn_mode;
} uart_cfg;

/* Serial register */
#define REG_UART0_BASE                   0x11040000

/* CRG register */
#define SHARE_BUS_CRG_CKEN               (REG_CRG_BASE + 0x0104)
#define SHARE_UART_CKEN                  1

typedef enum {
	UART_CLK_3  = 3000000,
	UART_CLK_24 = 24000000,
	UART_CLK_50 = 50000000,
	UART_CLK_100 = 100000000,
} uart_valid_clks;

typedef enum {
	UART_0 = REG_UART0_BASE,
} uart_num;

/*-----------------------------------------------------------------
 * serial interface
------------------------------------------------------------------*/
s32 serial_putc(uart_num uart_base, const s8 c);
s32 serial_puts(uart_num uart_base, const s8 *s);
s32 serial_tstc(uart_num uart_base);
s32 serial_getc(uart_num uart_base, u8 *ch);
void serial_put_hex(unsigned int hex);
void serial_put_dec(int dec);
void debug_puts(const char *buf, u32 hex);
int wait_uart_tx_busy_state(void);
#endif   /* __SERIAL_REG_H__ */
