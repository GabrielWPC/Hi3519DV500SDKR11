/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include "types.h"
#include "platform.h"
#include "lib.h"
#include "flash_map.h"
#include "checkup.h"
#include "common.h"
#include "cipher.h"

void clean_boot_img_addr_size(void)
{
	reg_set(BOOT_IMG_ADDR_REG_ADDR, 0);
	reg_set(BOOT_IMG_SIZE_REG_ADDR, 0);
}

void add_boot_img_size(u32 size)
{
	u32 val;
	val = reg_get(BOOT_IMG_SIZE_REG_ADDR);
	val += size;
	reg_set(BOOT_IMG_SIZE_REG_ADDR, val);
}

void set_verify_backup_img_reg(unsigned int type)
{
	/* Set verify_backup_img flag to type */
	reg_setbits(PWR_USER_REG2, BACKUP_IMG_FLAG_OFFSET, BACKUP_IMG_FLAG_WIDTH, type);
}

u32 get_verify_backup_img_reg(void)
{
	return reg_getbits(PWR_USER_REG2, BACKUP_IMG_FLAG_OFFSET, BACKUP_IMG_FLAG_WIDTH);
}