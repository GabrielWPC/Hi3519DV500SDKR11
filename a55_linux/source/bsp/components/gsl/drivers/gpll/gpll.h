/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __GPLL_H__
#define __GPLL_H__
#include "platform.h"

#define REG_PERI_CRG2064			0x2040
#define REG_PERI_CRG2048			0x2000

#define LOW_FREQ_BOOT_MODE			0
#define HIGH_FREQ_BOOT_MODE			1

#define BOOT_FREQ_MODE_SEL_BIT_OFFSET		24
#define BOOT_FREQ_MODE_SEL_BIT_WIDTH		1
#define OTP_SOC_FUSE_ADDR_BSAE			(OTP_SHADOW_BASE + 0x001c)

#define DDR_SCRUB_AND_GPLL_SWITHC_OFFSET	5
#define DDR_SCRUB_AND_GPLL_SWITHC_WIDTH		1
#define OTP_SHADOWED_ATE_CHIP_VER          	(OTP_SHADOW_BASE + 0x0124)

#define OPEN					1

#define A55_FCM_CKSEL_MASK			(0x7 << 11)
#define A55_FCM_CKSEL_675M			(0x2 << 11)
#define A55_FCM_CKSEL_24M			(0x0 << 11)

#define A55_CKSEL_MASK				(0x7 << 8)
#define A55_CKSEL_24M				(0x0 << 8)
#define A55_CKSEL_675M				(0x1 << 8)
#define A55_CKSEL_833M				(0x4 << 8)

#define DDRAXI_CKSEL_MASK			(0x7 << 12)
#define DDRAXI_CKSEL_540M			(0x5 << 12)
#define DDRAXI_CKSEL_594M			(0x6 << 12)


#define PLL_FIANL_LOCK_BIT_OFFSET		4
#define PLL_FIANL_LOCK_BIT_WIDTH		1

#define PERI_CRG_PLL206				0x338
#define GPLL_LOCK_STATUS_REG			(REG_BASE_CRG + PERI_CRG_PLL206)

#define PERI_CRG_PLL192         		0x300
#define PERI_CRG_PLL193         		0x304

#define GPLL_CTRL_REG0      			(REG_BASE_CRG + PERI_CRG_PLL192)
#define GPLL_CTRL_REG0_VAL			0x12800000

#define GPLL_CTRL_REG1      			(REG_BASE_CRG + PERI_CRG_PLL193)

#define PLL_LOCK_STATUS_COUNT 			3
#define PLL_LOCK_QUERY_MAX_TIMES  		16 /* total time 19.5us * 16 =312us > 300us(timerout time) */

#define PLL_POWER_DOWN				0
#define PLL_POWER_ON				1
#define PLL_LOCKED				2
#define PLL_FINAL_LOCKED			1

#define POWER_ON				0
#define POWER_DOWN				1
#define PLL_POWER_SWITCH_BIT_OFFSET		20
#define PLL_POWER_SWITCH_BIT_WIDTH		1

#define FMC_CLK_EN_OFFSET			4
#define FMC_CLK_EN_WIDTH			1
#define FMC_CLK_EN_VAL				1
#define FMC_CLK_DIS_VAL				0

#define MMC_CLK_EN_OFFSET			0
#define MMC_CLK_EN_WIDTH			1
#define MMC_CLK_EN_VAL				1
#define MMC_CLK_DIS_VAL				0

#define UART0_CLK_EN_OFFSET			4
#define UART0_CLK_EN_WIDTH			1
#define UART0_CLK_EN_VAL			1
#define UART0_CLK_DIS_VAL			0

#define _20_US_					20
#define _1_US_					1

typedef enum {
	SETTING = 1,
	RESUME = 2,
} cpu_and_sys_sub_state;

void bsp_gpll_config(u32 channel_type);
u8 get_ddr_scrub_and_gpll_switch(void);
#endif