/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef DDRTRN_GET_DDR_SIZE_H
#define DDRTRN_GET_DDR_SIZE_H

#define WINDING_STEP                 0x20000 /* unit:KB, equal to 128MB */
#define WINDING_STEP_FOR_DDR4_2DIE   0x2 /* unit:KB, equal to 2KB */
#define DDR_MAX_CAPAT                0x400000 /* unit: KB, equal to 4GB */
#define DDR_MAX_CAPAT_3_5_G          0x380000 /* unit: KB, equal to 3.5GB */
#define DDR_MAX_CAPAT_FOR_DDR4_2DIE  0x10 /* unit:KB, equal to 16KB */
#define DDR4_WINDING_POSITION1       0x4 /* unit:KB, equal to 4KB */
#define DDR4_WINDING_POSITION2       0x8 /* unit:KB, equal to 8KB */

#define DDR_STORE_NUM  1 /* 1:4Byte */

#define DDR_WINDING_NUM1  0x12345678
#define DDR_WINDING_NUM2  0xedcba987

struct ddr_data {
	unsigned int reg_val[DDR_STORE_NUM];
};
#endif /* DDRTRN_GET_DDR_SIZE_H */
