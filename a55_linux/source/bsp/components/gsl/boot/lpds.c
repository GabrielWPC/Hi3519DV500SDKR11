/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include "lpds.h"
#include "types.h"
#include "platform.h"
#include "lib.h"
#include "flash_map.h"
#include "checkup.h"
#include "common.h"
#include "cipher.h"
#include "td_type.h"
#include "share_drivers.h"
#include "err_print.h"

void enable_lpds_lock()
{
	unsigned int value;
	value = reg_get(PWR_HASH_VALUE_LOCK_REG);
	value |= PWR_HASH_VALUE_LOCK_ENABLE;
	reg_set(PWR_HASH_VALUE_LOCK_REG, value);
	value = reg_get(PWR_HASH_VALUE_LOCK_REG) & 0x00000001;
	if (value != PWR_HASH_VALUE_LOCK_ENABLE) {
		log_serial_puts((const s8 *)"enable_lpds_lock error\r\n");
		call_reset();
	}
}

void set_low_power_mode(u32 data)
{
	reg_setbits(LOW_POWER_MODE_REG_ADDR, LOW_POWER_MODE_REG_BIT_OFFSET, LOW_POWER_MODE_REG_BIT_WIDTH, data);
}

int get_low_power_mode(void)
{
	td_u8 data;
	data = (u8)reg_getbits(LOW_POWER_MODE_REG_ADDR, LOW_POWER_MODE_REG_BIT_OFFSET, LOW_POWER_MODE_REG_BIT_WIDTH);
	return data;
}

void clear_lpds(void)
{
	unsigned int value;
	/* clean lpds data */
	value = reg_getbits(PWR_HASH_VALUE_LOCK_REG, LPDS_LOCK_REG_BIT_OFFSET, PDS_LOCK_REG_BIT_WIDTH);
	if (value == 0) {
		for (int i = 0; i < LPDS_HASH_REG_SIZE / LPDS_BIT_OFFSET; i++) {
			reg_set(LPDS_REG_BASE_REG + i * LPDS_BIT_OFFSET, 0);
		}
	}
	/* clean lower mode */
	set_low_power_mode(0);
	log_serial_puts((const s8 *)"\nlpds clear ok\n");
}

int lpds_hash_get(u8 *lpds_hash, u32 lpds_hash_addr, u32 hash_len)
{
	int ret = EXT_SEC_FAILURE;
	ret = memset_ss(lpds_hash, HASH_LEN, 0, hash_len, 0);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	for (int k = 0; k < HASH_LEN; k = k + LPDS_BIT_OFFSET) {
		*((u32 *)(uintptr_t)&lpds_hash[k]) = reg_get(lpds_hash_addr + k);
	}
	return EXT_SEC_SUCCESS;
}

int verify_all_lpds_data(void)
{
	if (get_low_power_mode() == SUSPEND_TO_LPDS) {
		int ret;
		ret = lpds_hash_cmp(LPDS_ALL_HASH_ADDR, (uintptr_t)LPDS_CFCT_HASH_ADDR, LPDS_ALL_HASH_ADDR - LPDS_CFCT_HASH_ADDR);
		if (ret != EXT_SEC_SUCCESS) {
			err_print(LPDS_ALL_HASH_CMP_FAIL);
			return EXT_SEC_FAILURE;
		}
	}
	return EXT_SEC_SUCCESS;
}

int calculate_all_lpds_data(void)
{
	return calc_hash((uintptr_t)LPDS_CFCT_HASH_ADDR, LPDS_ALL_HASH_ADDR - LPDS_CFCT_HASH_ADDR,
					 (unsigned char *)LPDS_ALL_HASH_ADDR, SHA_256_LEN, 0);
}