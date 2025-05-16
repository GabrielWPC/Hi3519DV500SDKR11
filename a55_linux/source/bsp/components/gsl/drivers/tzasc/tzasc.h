/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __TZASC_H__
#define __TZASC_H__

#define TZASC_ATTR_SEC_R    (0x5 << 28)
#define TZASC_ATTR_SEC_W    (0x5 << 24)
#define TZASC_ATTR_NOSEC_R  (0x5 << 20)
#define TZASC_ATTR_NOSEC_W  (0x5 << 16)
#define TZASC_ATTR_MID_INV  (1 << 9)
#define TZASC_ATTR_MID_EN   (1 << 8)
#define TZASC_ATTR_SEC_INV  (1 << 4)

#define TZASC_ALGIN_BITS	12	// 4k alignment

int tzasc_bypass_disable();
int tzasc_sec_config_read_back(u8 rgn, uintptr_t addr, size_t size);
void tzasc_set_rgn_attr(u8 rgn, u32 attr);
// Notice: addr and size of function "tzasc_set_rgn_map" must align to 4K
void tzasc_set_rgn_map(u8 rgn, uintptr_t addr, size_t size);
void tzasc_rgn_enable(u8 rgn);
void config_tzasc(u8 rgn, u32 attr, uintptr_t addr, size_t size);

#endif /* __TZASC_H__ */
