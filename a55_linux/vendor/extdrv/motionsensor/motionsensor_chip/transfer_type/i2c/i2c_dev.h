/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description : i2c_dev.h
 * Author : ISP SW
 * Create : 2018-7-30
 * Version : Initial Draft
 */

#ifndef I2C_DEV_H
#define I2C_DEV_H

#include <linux/i2c.h>
#include "ot_type.h"

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

td_u8 motionsersor_i2c_write(struct i2c_client *ot_i2c_client,
                             td_u8 reg_addr, const td_u8 *reg_data, td_u32 cnt);
td_u8 motionsersor_i2c_read(struct i2c_client *ot_i2c_client,
                            td_u8 reg_addr, td_u8 *reg_data, td_u32 cnt);

td_u32 motionsersor_i2c_get_frequency(struct i2c_client *ot_i2c_client, td_s32 *frequency);

td_u32 motionsersor_i2c_init(struct i2c_client **ot_i2c_client,
                             struct i2c_board_info ot_i2c_board_info, td_s32 adapt_num);
td_s32 i2c_client_init(struct i2c_client *ot_i2c_client);

void motionsersor_i2c_exit(struct i2c_client **ot_i2c_client);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif
