/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include "types.h"
#include "platform.h"
#include "lib.h"
#include "flash_map.h"
#include "common.h"
#include "cipher.h"
#include "checkup.h"
#include "share_drivers.h"
#include "err_print.h"

static u32 get_ree_key_flash_offset(void)
{
	return GSL_CODE_OFFSET + get_gsl_code_area_len();
}

static u32 get_param_info_flash_offset(void)
{
	return get_ree_key_flash_offset() + REE_BOOT_KEY_AREA_SIZE;
}

static u32 get_first_param_data_flash_offset(void)
{
	return get_param_info_flash_offset() + PARM_AREA_INFO_SIZE + FILL_LEN_256BYTE; /* 512byte align for sd/emmc */
}

static u32 get_cur_param_data_flash_offset(u32 board_index)
{
	u32 para_len = get_ddr_param_len();
	para_area_info *para_info = (para_area_info *)(uintptr_t)get_ddr_param_info_addr();
	return (get_first_param_data_flash_offset() + para_info->board_index_hash_table[board_index] * para_len);
}

static u32 get_uboot_info_flash_offset(void)
{
	u32 ddr_para_len;
	u32 ddr_para_cnt;
	ddr_para_len = get_ddr_param_len();
	ddr_para_cnt = get_ddr_param_cnt();
	return (get_first_param_data_flash_offset() +  ddr_para_cnt * ddr_para_len);
}

static u32 get_uboot_code_flash_offset()
{
	return (get_uboot_info_flash_offset() + UBOOT_CODE_INFO_SIZE);
}

static int flash_read(u32 dest, u32 count, u32 offset)
{
	return dma_copy(dest, count, SPI_BASE_ADDR + offset);
}

int get_ree_key_and_paras_info_from_device(const backup_img_params_s *backup_params, u32 channel_type)
{
	u32 offset;
	u32 dst;
	u32 size;
	dst = get_ree_key_area_addr();
	offset = get_ree_key_flash_offset();
	size = REE_BOOT_KEY_AREA_SIZE + PARM_AREA_INFO_SIZE + FILL_LEN_256BYTE;
	if (channel_type == BOOT_SEL_FLASH) {
		offset += backup_params->offset_addr;
		return flash_read(dst, size, offset);
	} else if (channel_type == BOOT_SEL_EMMC) {
		offset += backup_params->offset_addr;
		return mmc_read((void *)(uintptr_t)dst, offset, size, READ_DATA_BY_CPU);
	} else if (channel_type == BOOT_SEL_SDIO) {
		if (!self_sdio_check()) {
			err_print(GSL_SDIO_CHECK_ERROR);
			return EXT_SEC_FAILURE;
		}
		enable_sdio_dma();
		set_sdio_pos(offset);
		(void)copy_from_sdio((void *)(uintptr_t)dst, size);
		return EXT_SEC_SUCCESS;
	}
	return EXT_SEC_FAILURE;
}

static int read_from_emmc(u32 dst, u32 first_offset, u32 cur_offset, u32 size, u32 board_index)
{
	u32 cycle;
	int ret = EXT_SEC_FAILURE;
	u32 para_cnt = get_ddr_param_cnt();
	u32 para_len = get_ddr_param_len();
	para_area_info *para_info = (para_area_info *)(uintptr_t)get_ddr_param_info_addr();
	u32 dst_tmp = dst + para_len;
	u32 emmc_offset = cur_offset;
	if (mmc_get_cur_mode() == BOOT_MODE) {
		emmc_offset = first_offset;
		cycle = para_info->board_index_hash_table[board_index];
		while (cycle) {
			ret = mmc_read((void *)(uintptr_t)dst_tmp, emmc_offset, para_len, READ_DATA_BY_CPU);
			if (ret != EXT_SEC_SUCCESS)
				return EXT_SEC_FAILURE;
			emmc_offset += para_len;
			cycle--;
		}
	}
	ret = mmc_read((void *)(uintptr_t)dst, emmc_offset, size, READ_DATA_BY_CPU);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	if (mmc_get_cur_mode() == BOOT_MODE) {
		emmc_offset += para_len;
		cycle = para_cnt - para_info->board_index_hash_table[board_index] - 1;
		if (cycle > MAX_PARAMS_NUM)
			return EXT_SEC_FAILURE;
		while (cycle) {
			ret = mmc_read((void *)(uintptr_t)(dst_tmp), emmc_offset, para_len, READ_DATA_BY_CPU);
			if (ret != EXT_SEC_SUCCESS)
				return EXT_SEC_FAILURE;
			emmc_offset += para_len;
			cycle--;
		}
	}
	return EXT_SEC_SUCCESS;
}

int get_paras_data_from_flash(const backup_img_params_s *backup_params, u32 board_index, u32 channel_type)
{
	u32 first_offset;
	u32 cur_offset;
	u32 para_len = get_ddr_param_len();
	u32 dst;
	u32 size;
	dst = get_ddr_param_data_addr();
	cur_offset = get_cur_param_data_flash_offset(board_index);
	size = para_len;
	if (channel_type == BOOT_SEL_FLASH) {
		cur_offset += backup_params->offset_addr;
		return flash_read(dst, size, cur_offset);
	} else if (channel_type == BOOT_SEL_EMMC) {
		first_offset = backup_params->offset_addr + get_first_param_data_flash_offset();
		cur_offset += backup_params->offset_addr;
		return read_from_emmc(dst, first_offset, cur_offset, size, board_index);
	} else if (channel_type == BOOT_SEL_SDIO) {
		enable_sdio_dma();
		set_sdio_pos(cur_offset);
		return copy_from_sdio((void *)(uintptr_t)dst, size);
	}
	return EXT_SEC_FAILURE;
}

int get_uboot_info_from_flash(const backup_img_params_s *backup_params, u32 channel_type)
{
	u32 offset;
	u32 dst;
	u32 size;
	dst = get_uboot_info_download_ddr_addr();
	offset = get_uboot_info_flash_offset();
	size = UBOOT_CODE_INFO_SIZE;
	if (channel_type == BOOT_SEL_FLASH) {
		offset += backup_params->offset_addr;
		return flash_read(dst, size, offset);
	} else if (channel_type == BOOT_SEL_EMMC) {
		offset += backup_params->offset_addr;
		return mmc_read((void *)(uintptr_t)dst, offset, size, READ_DATA_BY_CPU);
	} else if (channel_type == BOOT_SEL_SDIO) {
		enable_sdio_dma();
		set_sdio_pos(offset);
		return copy_from_sdio((void *)(uintptr_t)dst, size);
	}
	return EXT_SEC_FAILURE;
}

#define EDA_LODA_LEN 	1024
int get_uboot_code_from_flash(const backup_img_params_s *backup_params,  u32 channel_type)
{
	u32 offset;
	u32 dst;
	u32 size;
	uboot_code_info *uboot_info = (uboot_code_info *)get_uboot_info_download_ddr_addr();
	dst = uboot_info->uboot_entry_point;
	offset = get_uboot_code_flash_offset();
	size = uboot_info->code_area_len;
#ifdef CFG_EDA_VERIFY
	size = EDA_LODA_LEN;
#endif
	if (channel_type == BOOT_SEL_FLASH) {
		offset += backup_params->offset_addr;
		return flash_read(dst, size, offset);
	} else if (channel_type == BOOT_SEL_EMMC) {
		offset += backup_params->offset_addr;
		return mmc_read((void *)(uintptr_t)dst, offset, size, READ_DATA_BY_CPU);
	} else if (channel_type == BOOT_SEL_SDIO) {
		enable_sdio_dma();
		set_sdio_pos(offset);
		return copy_from_sdio((void *)(uintptr_t)dst, size);
	}
	return EXT_SEC_FAILURE;
}

