/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __SHARE_DRIVERS__
#define __SHARE_DRIVERS__
#include "soc_common_api.h"
#include "soc_cipher_api.h"
#include "soc_pke_api.h"
#include "soc_otp_api.h"
#include "soc_klad_api.h"
#include "soc_keyslot_api.h"
#include "soc_cipher_api.h"
#include "serial_pl011.h"

#define EXT_SM2_ID                    "\x31\x32\x33\x34\x35\x36\x37\x38\x31\x32\x33\x34\x35\x36\x37\x38"
#define EXT_SM2_ID_LEN                0x10
typedef enum {
	INIT_MODE  = 0x0,
	SLEEP_MODE = 0x5A,
	BOOT_MODE,
	DS_MODE,
	TRAN_MODE
} emmc_mode_u;

typedef enum {
	COLD_BOOT = 0x0,
	WARM_BOOT
} sys_boot_u;

/* mmc */
int mmc_init(void);
int mmc_read(void *ptr, size_t src, size_t size, size_t read_type);
unsigned int is_bootmode(void);
emmc_mode_u mmc_get_cur_mode(void);
void mmc_set_bootup_mode(sys_boot_u mode);
/* sdio */
int copy_from_sdio(void *buffer, unsigned long maxsize);
void set_sdio_pos(unsigned long pos);
void enable_sdio_dma(void);
/* usb */
int copy_from_usb(void *dest, size_t *count, u32 max_length);
void usb3_driver_init(void);
int send_to_usb(char *addr, size_t count);
int self_usb_check(void);
/* uart */
void pl011_putc(uart_num uart_base, s8 c);
td_s32 pl011_getc(uart_num uart_base);
td_s32 pl011_tstc(uart_num uart_base);
td_s32 serial_init(uart_num uart_base, const uart_cfg *cfg);
void log_serial_puts(const s8 *s);
td_s32 copy_from_uart(void *dest, size_t *count, td_u32 max_length);
/* timer */
void timer_init();
void timer_deinit();
void timer_start();
unsigned long timer_get_val();
void udelay(unsigned long usec);
void mdelay(unsigned long msec);
/* watchdog */
td_s32 watchdog_enable(td_u32 n, td_u32 timeout, td_u32 wdg_freq);
td_s32 watchdog_disable(td_u32 n);
td_s32 watchdog_feed(td_u32 n, td_u32 timeout);
/* sec mem opt */
td_s32 memcmp_ss(const td_void *cs, const td_void *ct, td_size_t count, uintptr_t check_word);
td_s32 memcpy_ss(td_void *dest, td_size_t dest_max, const td_void *src, td_size_t count, uintptr_t check_word);
td_s32 memset_ss(td_void *dest, td_size_t dest_max, td_u8 c, td_size_t count, uintptr_t check_word);
errno_t memcpy_s(void *dest, size_t dest_max, const void *src, size_t count);
errno_t memset_s(void *dest, size_t dest_max, int c, size_t count);
void save_cur_point_syscnt(void);
int self_sdio_check(void);
#endif /* __SHARE_DRIVERS__ */
