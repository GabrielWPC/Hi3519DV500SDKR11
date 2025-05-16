/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __LPDS_H_
#define __LPDS_H_
#include "types.h"
void clear_lpds(void);
int get_low_power_mode();
void set_low_power_mode();
void enable_lpds_lock();
int calculate_all_lpds_data(void);
int verify_all_lpds_data(void);
int lpds_hash_get(u8 *lpds_hash, u32 lpds_hash_addr, u32 hash_len);

#endif /* __LPDS_H_ */
