/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef DDR_LOW_FREQ_H
#define DDR_LOW_FREQ_H

#define CRG_DDR_PLL_CFG0       0x180  /* DPLL-1 configuration register */
#define CRG_DDR_PLL_CFG1       0x184  /* DPLL-2 configuration register */
#define DDR_DPLL_PD_BIT        20   /* bit[20] dpll_pd */
#define CRG_SOC_FREQ_CONFIG    0x2000 /* SOC frequency configuration register */
#define DDR_AXI_CKSEL_MASK     0x7  /* [14:12]ddraxi_cksel */
#define DDR_AXI_CKSEL_BIT      12   /* [14:12]ddraxi_cksel */
#define CRG_DDRAXI_CKSEL_24M   0x0  /* ddraxi_cksel 00:24MHz */
#define CRG_SOC_FREQ_INDICATE  0x2020 /* SOC frequency indication register */
#define DDR_AXI_SC_SELED_MASK  0x7  /* [14:12]ddraxi_sc_seled */
#define DDR_AXI_SC_SELED_BIT   12   /* [14:12]ddraxi_sc_seled */
#define CRG_DDR_CKSEL          0x2000 /* SOC-2 frequency configuration register */
#define DDR_CKSEL_MASK         0x7  /* [18:16]ddr_cksel */
#define DDR_CKSEL_BIT          16   /* [18:16]ddr_cksel */
#define CRG_DDR_CKSEL_24M      0x0  /* ddrtrn_cksel 000:24MHz */
#define CRG_DDR_CKSEL_PLL      0x1  /* ddrtrn_cksel 001:clk_dpll_pst */

/* value */
/* PLL 400MHz */
#define CRG_PLL7_TMP_1TO4 0x00001021
#define CRG_PLL6_TMP_1TO4 0x14aaaaaa
/* PLL 800MHz */
#define CRG_PLL7_TMP_1TO2 0x00001021
#define CRG_PLL6_TMP_1TO2 0x12aaaaaa
/* PLL 1600MHz */
#define CRG_PLL7_TMP_1TO1 0x00001021
#define CRG_PLL6_TMP_1TO1 0x11aaaaaa

#define DDR_FROM_VALUE0 0x0024140D /* partial boot form value */
#define DDR_FROM_VALUE1 0xFFDBEBF5 /* partial boot form value */

#define WL_TIME                 1 /* Number of write leveling times */
#endif /* DDR_LOW_FREQ_H */