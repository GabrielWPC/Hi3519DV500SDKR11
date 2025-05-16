/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __OTP_DRV_H__
#define __OTP_DRV_H__
#include "platform.h"

u32 is_tee_dec_en_enable(void);
u32 is_ree_boot_dec_en_enable(void);
u32 is_sec_dbg_enable(void);
u32 is_sec_dbg_lv_enable(void);
u32 is_func_jtag_enable(void);
u32 is_soc_tee_enable();
u32 is_ree_verify_enable();
u32 is_tp_verify_enable();
u32 is_tee_verify_enable();
u32 opt_get_boot_backup_enable(void);

#define OTP_BIT_ALIGNED_LOCKABLE        		(OTP_SHADOW_BASE + 0x0000)
#define OTP_4BIT_ALIGNED_LOCKABLE       		(OTP_SHADOW_BASE + 0x0004)
#define OTP_BYTE_ALIGNED_LOCKABLE_0     		(OTP_SHADOW_BASE + 0x0010)
#define OTP_BYTE_ALIGNED_LOCKABLE_1    	 		(OTP_SHADOW_BASE + 0x0014)
#define OTP_SHADOWED_ONEWAY_0           		(OTP_SHADOW_BASE + 0x01E0)
#define OTP_SHADOWED_ONEWAY_1           		(OTP_SHADOW_BASE + 0x01E4)
#define OTP_SHADOWED_ONEWAY_2           		(OTP_SHADOW_BASE + 0x01E8)
#define OTP_SHADOWED_ONEWAY_3           		(OTP_SHADOW_BASE + 0x01EC)

#define OTP_TEE_HASH_FLASH_ROOTKEY      		0x40
#define OTP_REE_HASH_FLASH_ROOTKEY      		0x60
#define OTP_TP_HASH_FLASH_ROOTKEY       		0x80

#define OTP_HASH_FLASH_KEY_LOCKER       		0x1FB
#define OTP_TP_HASH_FLASH_ROOTKEY_LOCKER        	0x1FC

#define OTP_TEE_BOOT_VERSION                    0x1B0  /* 2bytes */
#define OTP_UPGRADER_VERSION                    0x1B2  /* 2bytes */
#define OTP_PROVISION_VERSION                   0x1B4  /* 2bytes */
#define OTP_TEE_OS_VERSION                      0x1B8  /* 8bytes */
#define OTP_TEE_OS_VERSION_EX                   0x1BC  /* 8bytes */
#define OTP_PARAMS_VERSION                      0x1CC  /* 4bytes */
#define OTP_REE_BOOT_VERSION                    0x1D0  /* 4bytes */
#define OTP_REE_APP_VERSION                     0x1D4  /* 4bytes */

#define OTP_TP_KEY_VERSION                      0x1DF   /* 4bit */

#define OTP_MSID                                0x30
#define OTP_DIE_ID                              0xF0   /* 0xF0~0xFF, 16bytes */

typedef union {
	struct {
		unsigned int scs_alg_sel                      : 1; /* [0] */
		unsigned int tee_owner_sel                    : 1; /* [1] */
		unsigned int rkp_deob_alg_sel                 : 1; /* [2] */
		unsigned int ds_disable                       : 1; /* [3] */
		unsigned int chip_features_cfg_disable        : 1; /* [4] */
		unsigned int wfi_wdg_rst_en                   : 1; /* [5] */
		unsigned int dice_cdi_enable                  : 1; /* [6] */
		unsigned int boot_backup_enable               : 1; /* [7] */
		unsigned int reserved_1                       : 16; /* [23:8] */
		unsigned int sha1_disable                     : 1; /* [24] */
		unsigned int sm4_disable                      : 1; /* [25] */
		unsigned int sm3_disable                      : 1; /* [26] */
		unsigned int sm2_disable                      : 1; /* [27] */
		unsigned int chip_features_global_disable     : 1; /* [28] */
		unsigned int reserved_2                       : 3; /* [31:29] */
	} bits;
	unsigned int u32;
} otp_bit_aligned_lockable; /* 0x0 */

typedef union {
	struct {
		unsigned int chip_life_cycle             : 4; /* [3:0] */
		unsigned int rma_re_enable               : 4; /* [7:4] */
		unsigned int shadow_check_enable         : 4; /* [11:8] */
		unsigned int clr_router_test_disable     : 4; /* [15:12] */
		unsigned int tee_dec_enable              : 4; /* [19:16] */
		unsigned int ree_dec_enable              : 4; /* [23:20] */
		unsigned int reserved                    : 4; /* [27:24] */
		unsigned int wakeup_check_en_lock        : 4; /* [31:28] */
	} bits;
	unsigned int u32;
} otp_4bit_aligned_lockable; /* 0x4 */

typedef union {
	struct {
		unsigned int otp_not_blank               : 16; /* [15:0] */
		unsigned int soc_tee_enable              : 8; /* [23:16] */
		unsigned int obfu_mrk1_owner_id_low      : 8; /* [31:24] */
	} bits;
	unsigned int u32;
} otp_byte_aligned_lockable_0; /* 0x10 */

typedef union {
	struct {
		unsigned int obfu_mrk1_owner_id_high     : 8; /* [7:0] */
		unsigned int tp_verify_enable            : 8; /* [15:8] */
		unsigned int ree_verify_enable           : 8; /* [23:16] */
		unsigned int tee_verify_enable           : 8; /* [31:24] */
	} bits;
	unsigned int u32;
} otp_byte_aligned_lockable_1; /* 0x14 */

typedef union {
	struct {
		unsigned int msid                  : 32; /* [31:0] */
	} bits;
	unsigned int u32;
} otp_word_aligned_lockable; /* 0x30 */

typedef union {
	struct {
		unsigned int version_id                  : 8; /* [0] */
		unsigned int die_code                    : 24; /* [1] */
	} bits;
	unsigned int u32;
} otp_ate_data_version_id; /* 0x120 */

typedef union {
	struct {
		unsigned int tee_boot_ver               : 16; /* [15:0] */
		unsigned int reserved                   : 16; /* [31:16] */
	} bits;
	unsigned int u32;
} otp_tee_oneway_0; /* 0x1b0 */


typedef union {
	struct {
		unsigned int reserved_0                 : 24; /* [23:0] */
		unsigned int tp_key_ver                 : 4; /* [27:24] */
		unsigned int reserved                   : 4; /* [31:28] */
	} bits;
	unsigned int u32;
} otp_noshadowed_oneway_0; /* 0x1dc */


typedef union {
	struct {
		unsigned int scs_alg_sel_lock                         : 1; /* [0] */
		unsigned int tee_owner_sel_lock                       : 1; /* [1] */
		unsigned int rkp_deob_alg_sel_lock                    : 1; /* [2] */
		unsigned int hisi_ds_disable_lock                     : 1; /* [3] */
		unsigned int chip_features_cfg_disable_lock           : 1; /* [4] */
		unsigned int wfi_wdg_rst_en_lock                      : 1; /* [5] */
		unsigned int dice_cdi_enable_lock                     : 1; /* [6] */
		unsigned int boot_backup_enable_lock                  : 1; /* [7] */
		unsigned int reserved_1                               : 16; /* [23:8] */
		unsigned int sha1_disable_lock                        : 1; /* [24] */
		unsigned int sm4_disable_lock                         : 1; /* [25] */
		unsigned int sm3_disable_lock                         : 1; /* [26] */
		unsigned int sm2_disable_lock                         : 1; /* [27] */
		unsigned int chip_features_global_disable_lock        : 1; /* [28] */
		unsigned int reserved_2                               : 3; /* [31:29] */
	} bits;
	unsigned int u32;
} otp_bit_aligned_locker; /* 0x01F0 */

typedef union {
	struct {
		unsigned int chip_life_cycle_lock             : 1; /* [0] */
		unsigned int rma_re_enable_lock               : 1; /* [1] */
		unsigned int shadow_check_enable_lock         : 1; /* [2] */
		unsigned int clr_router_test_disable_lock     : 1; /* [3] */
		unsigned int tee_dec_enable_lock              : 1; /* [4] */
		unsigned int ree_dec_enable_lock              : 1; /* [5] */
		unsigned int reserved_1                       : 1; /* [6] */
		unsigned int wakeup_check_en_lock             : 1; /* [7] */
		unsigned int reserved_2                       : 24; /* [31:8] */
	} bits;
	unsigned int u32;
} otp_4bit_aligned_locker; /* 0x1F4 */

typedef union {
	struct {
		unsigned int reserved_0                  : 1; /* [0] */
		unsigned int otp_reload_disable          : 1; /* [1] */
		unsigned int uart_halt_interval          : 2; /* [3:2] */
		unsigned int secret_scan_disable         : 4; /* [7:4] */
		unsigned int sec_boot_dbg_lv             : 2; /* [9:8] */
		unsigned int reserved_1                  : 2; /* [11:10] */
		unsigned int uart_selfboot_disable       : 1; /* [12] */
		unsigned int usb_selfboot_disable        : 1; /* [13] */
		unsigned int sdio_selfboot_disable       : 1; /* [14] */
		unsigned int wdg_rst_enable              : 1; /* [15] */
		unsigned int sec_boot_dbg_disable        : 8; /* [23:16] */
		unsigned int rom_root_pub_key_sel        : 4; /* [27:24] */
		unsigned int tsmc_efuse_rep_dis          : 4; /* [31:28] */
	} bits;
	unsigned int u32;
} otp_shadowed_oneway_0; /* 0x1e0 */


typedef union {
	struct {
		unsigned int secret_debug_disable     : 8; /* [7:0] */
		unsigned int func_jtag_mode           : 8; /* [15:8] */
		unsigned int dft_test_jtag_mode       : 8; /* [23:16] */
		unsigned int tee_priv_dbg_mode        : 2; /* [25:24] */
		unsigned int soc_jtag_mode            : 2; /* [27:26] */
		unsigned int reserved_1               : 2; /* [29:28] */
		unsigned int reserved_2               : 2; /* [31:30] */
	} bits;
	unsigned int u32;
} otp_shadowed_oneway_1; /* 0x1e4 */

typedef union {
	struct {
		unsigned int reserved_0                       : 2; /* [1:0] */
		unsigned int tee_priv_dbg_mode_ext            : 2; /* [3:2] */
		unsigned int reserved_1                       : 4; /* [7:4] */
		unsigned int wakeup_disable                   : 4; /* [11:8] */
		unsigned int flow_checker_disable             : 4; /* [15:12] */
		unsigned int rom_dbg_disable                  : 4; /* [19:16] */
		unsigned int cfct_key_ver                     : 4; /* [23:20] */
		unsigned int reserved_2                       : 4; /* [27:24] */
		unsigned int reserved_3                       : 4; /* [31:28] */
	} bits;
	unsigned int u32;
} otp_shadowed_oneway_2; /* 0x1e8 */

typedef union {
	struct {
		unsigned int reserved_0                       : 8; /* [7:0] */
		unsigned int cw_crc_rd_disable                : 8; /* [15:8] */
		unsigned int reserved_1                       : 4; /* [19:16] */
		unsigned int reserved_2                       : 4; /* [23:20] */
		unsigned int reserved_3                       : 8; /* [31:24] */
	} bits;
	unsigned int u32;
} otp_shadowed_oneway_3; /* 0x1ec */


typedef union {
	struct {
		unsigned char msid_lock                    : 1; /* [0] */
		unsigned char aidsp_msid_lock              : 1; /* [1] */
		unsigned char reserved0                    : 2; /* [3:2] */
		unsigned char tee_hash_flash_root_key_lock : 2; /* [5:4] */
		unsigned char ree_hash_flash_root_key_lock : 2; /* [7:6] */
	} bits;
	unsigned char u8;
} otp_hash_flash_key_locker; /* 0x1fb */

typedef union {
	struct {
		unsigned char tp_hash_flash_root_key_lock  : 2; /* [1:0] */
		unsigned char obfu_mrk0_lock               : 1; /* [2] */
		unsigned char obfu_mrk1_lock               : 1; /* [3] */
		unsigned char obfu_mrk2_lock               : 1; /* [4] */
		unsigned char obfu_usk_lock                : 1; /* [5] */
		unsigned char obfu_rusk_lock               : 1; /* [6] */
		unsigned char reserved                     : 1; /* [7] */
	} bits;
	unsigned char u8;
} otp_tp_hash_flash_key_locker; /* 0x1fc */
#endif /* __OTP_DRV_H__ */
