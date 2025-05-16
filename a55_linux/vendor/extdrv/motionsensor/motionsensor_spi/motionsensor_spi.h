/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: motionsensor_spi.h for linux
 * Author: ISP SW
 * Create: 2018-7-30
 * Version: Initial Draft
 */

#ifndef MOTIONSENSOR_SPI_H
#define MOTIONSENSOR_SPI_H

#include "ot_type.h"

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#define SSP_READ_ALT  0x1
#define SSP_WRITE_ALT 0X3

typedef struct {
    td_u32 spi_no;
    td_u8 dev_addr;
    td_u32 dev_byte_num;
    td_u32 reg_addr;
    td_u32 addr_byte_num;
    td_u32 data;
    td_u32 data_byte_num;
} spi_data;

td_s32 ot_motionsensor_ssp_get_num(td_void);
td_s32 ot_motionsensor_ssp_write_alt(td_u32 ssp_no, td_u8 reg_addr, const td_u8 *data);
td_u16 ot_motionsensor_ssp_read_alt(td_u32 ssp_no, td_u8 reg_addr, td_u8 *reg_data,
    td_u32 cnt, td_bool fifo_mode);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif
