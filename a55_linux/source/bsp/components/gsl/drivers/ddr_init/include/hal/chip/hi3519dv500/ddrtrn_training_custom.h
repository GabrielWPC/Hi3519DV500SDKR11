/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef DDR_TRAINING_CUSTOM_H
#define DDR_TRAINING_CUSTOM_H

/* boot adaptation */
#if BOOTLOADER == DDR_BOOT_TYPE_UBOOT
#define DDR_VREF_TRAINING_CONFIG
#define DDR_VREF_WITHOUT_BDL_CONFIG
#elif BOOTLOADER == DDR_BOOT_TYPE_CMD_BIN
#define DDR_VREF_TRAINING_CONFIG
#define DDR_VREF_WITHOUT_BDL_CONFIG
#elif BOOTLOADER == DDR_BOOT_TYPE_HSL
#define DDR_TRAINING_LOG_DISABLE
#elif BOOTLOADER == DDR_BOOT_TYPE_AUX_CODE
#define DDR_TRAINING_LOG_DISABLE
#else
# error Unknown boot Type
#endif

/* config DDRC, PHY, DDRT typte */
#define DDR_DDRC_S14_CONFIG
#define DDR_PHY_C28_V200_CONFIG
#define DDR_DDRT_V2_3_SHF0_CONFIG

/* config special item */
/* s40/t28/t16 not support dcc training */
#define DDR_CHANNEL_MAP_PHY0_DMC0_PHY1_DMC2
#define DDR_DCC_TRAINING_CONFIG
#define DDR_AUTO_PD_CONFIG
#define DDR_LOW_FREQ_CONFIG
#define DDR_RETRAIN_CONFIG
#define DDR_RDQS_OFFSET_CONFIG
#define DDR_OE_CONFIG
#define DDR_CAPAT_ADAPT_CFG
#define DDR_TRAINING_UART_DISABLE
#define DDR_TRAINING_MINI_LOG_CONFIG

#define DDR_PHY_NUM         1 /* phy number */

#define DDR_DMC_PER_PHY_MAX 2 /* dmc number per phy max */

#define DDR_RDQS_OFFSET          10 /* rdqs offset */
#define DDR_RDQS_OFFSET_LPDDR4   0 /* rdqs offset */
#define DDR_RDQ_OFFSET           0x3 /* rdq offset */

/* config DDRC, PHY, DDRT base address */
/* [CUSTOM] DDR PHY0 base register */
#define DDR_REG_BASE_PHY0        0x11150000
/* [CUSTOM] DDR DMC0 base register */
#define DDR_REG_BASE_DMC0        0x11148000
/* [CUSTOM] DDR DMC1 base register */
#define DDR_REG_BASE_DMC1        0x11149000
/* [CUSTOM] DDR DMC2 base register */
#define DDR_REG_BASE_DMC2        0x1114a000
/* [CUSTOM] DDR DMC3 base register */
#define DDR_REG_BASE_DMC3        0x1114b000
/* [CUSTOM] DDR DDRT base register */
#define DDR_REG_BASE_DDRT        0x11160000
/* [CUSTOM] DDR training item system control */
#define DDR_REG_BASE_SYSCTRL     0x11020000
#define DDR_REG_BASE_AXI         0x11140000
/* Serial Configuration */
#define DDR_REG_BASE_UART0       0x11040000

/* config offset address */
/* Assume sysctrl offset address for DDR training as follows,
if not please define. */
/* [CUSTOM] ddr hw training item */
#define SYSCTRL_DDR_HW_PHY0_RANK0       0x90
#define SYSCTRL_DDR_HW_PHY0_RANK1       0x94
#define SYSCTRL_DDR_HW_PHY1_RANK0       0x98
#define SYSCTRL_DDR_HW_PHY1_RANK1       0x9c
/* [CUSTOM] ddr sw training item */
#define SYSCTRL_DDR_TRAINING_CFG        0xa0 /* RANK0 */
#define SYSCTRL_DDR_TRAINING_CFG_SEC    0xa4 /* RANK1 */
/* [CUSTOM] PHY0 ddrt reversed data */
#define SYSCTRL_DDRT_PATTERN            0xa8
/* [CUSTOM] PHY1 ddrt reversed data */
#define SYSCTRL_DDRT_PATTERN_SEC        0xac
/* [CUSTOM] ddr training stat */
#define SYSCTRL_DDR_TRAINING_STAT       0xb0
/* [CUSTOM] ddr training version flag */
#define SYSCTRL_DDR_TRAINING_VERSION_FLAG	0xb4
#define SYSCTRL_DDR_TRAINING_CTX        0xb8
#define SYSCTRL_DDR_TRAINING_PHY        0xbc
#define SYSCTRL_DDR_TRAINING_ADJUST     0xc0
#define SYSCTRL_DDR_TRAINING_DQ_TYPE    0xc4

/* MISC DDRT control register */
#define MISC_DDRT_CTRL     0x4214
#define DDRT_MST_SEL_BIT   0 /* bit[0] ddrt_mst_sel */

/* config other special */
/* [CUSTOM] DDR training start address. MEM_BASE_DDR */
#define DDRT_CFG_BASE_ADDR       0x40000000
/* [CUSTOM] SRAM start address.
NOTE: Makefile will parse it, plase define it as Hex. eg: 0xFFFF0C00 */
#define DDR_TRAINING_RUN_STACK      0x04035000

#define CRG_REG_BASE_ADDR  0x11010000
#define CRG_DDRT           0x22a0   /* DDRT clock and soft reset control register */
#define DDR_TEST0_CKEN_BIT     4  /* ddrtest0_cken */
#define DDR_TEST0_SRST_REQ_BIT 0  /* ddrtest0_srst_req */
#define CRG_GPLL_SSMOD_CTRL      0x190 /* DPLL spread spectrum module control register */
#define SSMOD_CKEN_BIT             0 /* bit[0] ssmod_cken */
#define SSMOD_DISABLE_BIT          2 /* bit[2] ssmod_disable */

#define DDR_RELATE_REG_DECLARE
struct ddrtrn_hal_training_custom_reg {
	unsigned int ddrt_clk_reg;
	unsigned int age_compst_en[DDR_PHY_NUM];
	unsigned int trfc_en[DDR_PHY_NUM];
	unsigned int ddrt_ctrl;
};
int ddrtrn_hal_boot_cmd_save(struct ddrtrn_hal_training_custom_reg *custom_reg);
void ddrtrn_hal_boot_cmd_restore(const struct ddrtrn_hal_training_custom_reg *custom_reg);
#endif /* DDR_TRAINING_CUSTOM_H */
