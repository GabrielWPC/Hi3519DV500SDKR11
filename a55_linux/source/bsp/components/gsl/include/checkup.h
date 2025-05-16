/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include "types.h"
#include "td_type.h"
typedef enum {
	SOURCE_CLEAN            = 0,
	SECURE_MODE_INIT_FAILED = 1,
	DETECT_BOOT_DEVICE_FAILED,
	LOAD_IMG_FROM_DEVICE_FAILED,
	VERIFY_FLASH_ROOT_KEYS_FAILED,
	VERIFY_GSL_FAILED,
	VERIFY_REE_KEY_FAILED,
	VERIFY_PARAMS_FAILED,
	VERIFY_UBOOT_FAILED,
	VERIFY_TEE_OS_FAILED
} failure_source_type;

td_s32 hash_compare(uintptr_t area_addr, td_u32 area_len, const td_u8 *hash_addr);
td_s32 lpds_hash_cmp(td_u32 hash_addr, uintptr_t area_addr, td_u32 area_len);
u32 get_gsl_code_info_addr(void);
u32 get_gsl_code_addr(void);
u32 get_gsl_code_area_len(void);
u32 get_gsl_heap_addr(void);
u32 get_gsl_heap_len(void);
u32 get_ree_key_area_addr(void);
u32 get_ddr_param_info_addr(void);
u32 get_ddr_param_len(void);
u32 get_ddr_param_cnt(void);
u32 get_ddr_param_data_addr(void);
u32 get_ddr_param_data_end_addr(void);
uintptr_t get_uboot_info_download_ddr_addr();
uintptr_t get_uboot_code_ddr_addr(void);
uintptr_t get_uboot_code_size(void);
uintptr_t get_uboot_entrypoint_ddr_addr(void);

int handle_ree_key_area(void);
int handle_ddr_param_info(unsigned int board_index);
int handle_ddr_param(unsigned int board_index);
int handle_uboot_info(void);
int handle_uboot_code(void);

int handle_tee_key(uintptr_t tee_key_addr);
int handle_tee_info(uintptr_t tee_key_addr, uintptr_t tee_info_addr);
int handle_atf_code(uintptr_t addr, uintptr_t tee_info_addr, uintptr_t atf_code_addr);
int handle_tee_code(uintptr_t addr, uintptr_t tee_info_addr, uintptr_t tee_code_addr);

