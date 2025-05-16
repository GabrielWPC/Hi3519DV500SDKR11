/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <types.h>
#include <platform.h>
#include "share_drivers.h"
#include "err_print.h"
#include "tzasc.h"

#define TZASC_BYPASS_DIS	0x00
#define TZASC_RNG_EN		1

typedef union {
	struct {
		unsigned rgn_base_addr : 22;	// [21:0]
		unsigned reserved : 9;			// [30:22]
		unsigned rgn_en : 1;			// [31]
	} bits;
	u32 u32;
} sec_rgn_map;

typedef union {
	struct {
		unsigned rgn_size : 24;			// [23:0]
		unsigned reserved : 8;			// [31:24]
	} bits;
	u32 u32;
} sec_rgn_map_ext;

typedef union {
	struct {
		unsigned sp : 4;				// [3:0]
		unsigned security_inv : 1;		// [4]
		unsigned reserved_1 : 3;		// [7:5]
		unsigned mid_en : 1;			// [8]
		unsigned mid_inv : 1;			// [9]
		unsigned reserved_0 : 6;		// [15:10]
		unsigned tee_sp_nosec_w : 4;	// [19:16]
		unsigned tee_sp_nosec_r : 4;	// [23:20]
		unsigned tee_sp_sec_w : 4;		// [27:24]
		unsigned tee_sp_sec_r : 4;		// [31:28]
	} bits;
	u32 u32;
} sec_rgn_attr;

static volatile sec_rgn_map *get_sec_rgn_map_reg(u8 rgn)
{
	uintptr_t addr;
	if (rgn < TZASC_MAX_RGN_NUMS) {
		addr = SEC_MODULE_BASE + tzasc_rgn_offset(rgn) + TZASC_RGN_MAP_REG_OFFSET;
		return (sec_rgn_map *)addr;
	}
	return NULL;
}

static volatile sec_rgn_map_ext *get_sec_rgn_map_ext_reg(u8 rgn)
{
	uintptr_t addr;
	if (rgn < TZASC_MAX_RGN_NUMS) {
		addr = SEC_MODULE_BASE + tzasc_rgn_offset(rgn) + TZASC_RGN_MAP_EXT_REG_OFFSET;
		return (sec_rgn_map_ext *)addr;
	}
	return NULL;
}

static volatile sec_rgn_attr *get_sec_rgn_attr_reg(u8 rgn)
{
	uintptr_t addr;
	if (rgn < TZASC_MAX_RGN_NUMS) {
		addr = SEC_MODULE_BASE + tzasc_rgn_offset(rgn) + TZASC_RGN_ATTR_REG;
		return (sec_rgn_attr *)addr;
	}
	return NULL;
}

int tzasc_sec_config_read_back(u8 rgn, uintptr_t addr, size_t size)
{
	unsigned sec_rgn_base_addr;
	unsigned sec_rgn_size;
	volatile sec_rgn_map *map_reg = get_sec_rgn_map_reg(rgn);
	volatile sec_rgn_map_ext *map_ext_reg = get_sec_rgn_map_ext_reg(rgn);
	sec_rgn_base_addr = map_reg->bits.rgn_base_addr;
	sec_rgn_size = map_ext_reg->bits.rgn_size;
	if ((sec_rgn_base_addr != (addr >> TZASC_ALGIN_BITS)) || (sec_rgn_size != (size >> TZASC_ALGIN_BITS))) {
		err_print(TEE_SECURE_DDR_ADDR_SIZE_ERR);
		return EXT_SEC_FAILURE;
	}
	return EXT_SEC_SUCCESS;
}

int tzasc_bypass_disable()
{
	reg_set((u32 *)(SEC_MODULE_BASE + TZASC_BYPASS_REG_OFFSET), TZASC_BYPASS_DIS);
	if (reg_get(SEC_MODULE_BASE + TZASC_BYPASS_REG_OFFSET) != TZASC_BYPASS_DIS) {
		err_print(TZASC_BYPASS_DISABLE_ERR);
		return EXT_SEC_FAILURE;
	}
	return EXT_SEC_SUCCESS;
}

void tzasc_rgn_enable(u8 rgn)
{
	volatile sec_rgn_map *map_reg = get_sec_rgn_map_reg(rgn);
	if (map_reg == NULL)
		return;
	map_reg->bits.rgn_en = TZASC_RNG_EN;
}

void tzasc_set_rgn_map(u8 rgn, uintptr_t addr, size_t size)
{
	volatile sec_rgn_map *map_reg = get_sec_rgn_map_reg(rgn);
	volatile sec_rgn_map_ext *map_ext_reg = get_sec_rgn_map_ext_reg(rgn);
	if ((map_reg == NULL) || (map_ext_reg == NULL))
		return;
	map_reg->bits.rgn_base_addr = addr >> TZASC_ALGIN_BITS;
	map_ext_reg->bits.rgn_size = size >> TZASC_ALGIN_BITS;
}

void tzasc_set_rgn_attr(u8 rgn, u32 attr)
{
	volatile sec_rgn_attr *attr_reg = get_sec_rgn_attr_reg(rgn);
	if (attr_reg != NULL)
		attr_reg->u32 = attr;
}

void config_tzasc(u8 rgn, u32 attr, uintptr_t addr, size_t size)
{
	tzasc_set_rgn_attr(rgn, attr);
	tzasc_set_rgn_map(rgn, addr, size);
	tzasc_rgn_enable(rgn);
}