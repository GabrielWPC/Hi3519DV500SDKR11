/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef DDR_DDRC_V520_H
#define DDR_DDRC_V520_H

/* ******* DMC ************************* */
/* base address: DDR_REG_BASE_DMC0 DDR_REG_BASE_DMC1 */
/* register offset address */
#define DDR_DMC_CTRL_SREF     0X0  /* DDRC self-refresh control. */
#define DDR_DMC_CFG_SREF      0x20 /* DDRC self-refresh config. */
#define DDR_DMC_CFG_PD        0x28 /* PowerDown */
#define DDR_DMC_CFG_DDRMODE   0x50
#define DDR_DMC_CFG_SCRAMB    0x58 /* DDR scramb config */
#define ddr_dmc_cfg_rnkvol(dmc_rnks) (0x60 + ((dmc_rnks) << 2)) /* bit operation , 2 : dmc_rnks */
#define DDR_DMC_CFG_EMRS01    0xf0
#define DDR_DMC_TIMING2       0x108
#define DDR_DMC_TIMING8       0x120 /* DDRC timing parameter register 8 */
#define DMC_TIMING8_TRFC_AB_BIT  19  /* [27:19]trfc_ab */
#define DMC_TIMING8_TRFC_AB_MASK 0x1ff /* [27:19]trfc_ab */
#define DDR_DMC_SFCREQ        0xc
#define DDR_DMC_SFCCMD        0x210
#define DDR_DMC_SFCADDR       0x214 /* read col and row */
#define DDR_DMC_SFCBANK       0x218
#define DDR_DMC_CURR_FUNC     0x294
#ifndef DDR_DMC_SFC_RDATA0
#define DDR_DMC_SFC_RDATA0 0x4A8 /* SFC read data[31:0] */
#endif
#ifndef DDR_DMC_SFC_RDATA1
#define DDR_DMC_SFC_RDATA1 0x4AC /* SFC read data[63:32] */
#endif
#ifndef DDR_DMC_SFC_RDATA2
#define DDR_DMC_SFC_RDATA2 0x4B0 /* SFC read data[95:64] */
#endif
#ifndef DDR_DMC_SFC_RDATA3
#define DDR_DMC_SFC_RDATA3 0x4B4 /* SFC read data[127:96] */
#endif
#ifndef DDR_DMC_SFC_RDATA4
#define DDR_DMC_SFC_RDATA4 0x4B8 /* SFC read data[159:128] */
#endif
#ifndef DDR_DMC_SFC_RDATA5
#define DDR_DMC_SFC_RDATA5 0x4BC /* SFC read data[191:160] */
#endif
#ifndef DDR_DMC_SFC_RDATA6
#define DDR_DMC_SFC_RDATA6 0x4C0 /* SFC read data[223:192] */
#endif
#ifndef DDR_DMC_SFC_RDATA7
#define DDR_DMC_SFC_RDATA7 0x4C4 /* SFC read data[255:224] */
#endif
#define DDR_DMC_TEST_GENPOSE0  0x600

/* register mask */
#define DMC_CMD_MRS_MASK            0xffff
/* storing data bus width. [00]8bit, [01]16bit, [10]32bit, [11]64bit */
#define DMC_MEM_WIDTH_MASK          0x3
#define DMC_DDRMODE_RANK_MASK       0x3 /* [21:20] rank */
#define DMC_MRS_MASK                0xffff /* [15:0] Mode Register mask */
#define DMC_MR0_BL_MASK             0x3
#define DMC_CFG_DRAM_TYPE_MASK      0xf /* [3:0]101:DDR2, 110:DDR3, 111:DDR4 */
#define DMC_CFG_MEM_BG_MASK         0x3 /* [11:10]0:1, 1:2, 2:4 Bank Group */
#define DMC_CURR_FUNC_IN_SREF_MASK  0x1
#define DMC_RNKVOL_MEM_BG_MASK      0x3 /* [11:10] */
#define DMC_RNKVOL_MEM_BANK_MASK    0x3 /* [9:8] */
#define DMC_RNKVOL_MEM_ROW_MASK     0x7 /* [6:4] */
#define DMC_RNKVOL_MEM_COL_MASK     0x7 /* [2:0] */
#define DMC_CFG_INIT_XSREF_PD_MASK  0xc /* [3:2] */
#define DMC_CFG_PD_PRD_MASK         0xfff /* [15:4]pd_prd */

/* register bit */
#define DMC_MEM_WIDTH_BIT       4  /* storing data bus width */
#define DMC_DDRMODE_RANK_BIT    20 /* [21:20] rank */
/* [CUSTOM] precharge disable/enable bit */
#define DMC_SFC_PRE_DIS_BIT     30
/* [CUSTOM] [29:12]config MR when LMR command */
#define DMC_SFC_CMD_MRS_BIT     12
#define DMC_SFC_RANK_BIT        16 /* [CUSTOM] [31:16]sfc_rank */
#define DMC_CFG_MEM_BG_BIT      10 /* [11:10] mem_bankgroup */
#define DMC_RNKVOL_MEM_BANK_BIT 8  /* [9:8] mem_bank */
#define DMC_RNKVOL_MEM_ROW_BIT  4  /* [6:4] mem_row */
#define DMC_RNKVOL_MEM_COL_BIT  0  /* [2:0] mem_col */
#define DMC_CFG_PD_PRD_BIT      4  /* [15:4]pd_prd */
#define DMC_CFG_PD_EN_BIT       0  /* [0] pd_en */

/* register value */
#define DMC_BANK_MR1                1
#define DMC_BANK_MR3                0x3
#define DMC_CMD_TYPE_LMR            0x2
#define DMC_CMD_TYPE_READ           0x5  /* read  */
#define DMC_CMD_TYPE_PRECHARGE_ALL  0x6  /* precharge all */
#define DMC_CMD_MRS_MR3             0x4  /* MR3 equal 0x4 */
#define DMC_CMD_MRS_A7              0x80
/* value 1 means exexute command. cmd_rank[0] control DDR RANK0 */
#define DMC_CMD_RANK0               0x1
#define DMC_MR0_BL_BUST8            0x0  /* BC8 (fixed) */
#define DMC_MR0_BL_BUST4            0x2  /* BC4 (fixed) */
#define DMC_AUTO_TIMING_DIS         0xfffff000 /* auto refresh disable */
#define DMC_POWER_DOWN_DIS          0xfffffffe /* powerDown disable */
#define DMC_SCRAMB_DIS              0xffffbfff /* [14] scramb disable */
/* [4] scramb_seed_type, [2:0] scramb_seed_sort */
#define DMC_SCRAMB_CFG              0xffffffe8
#define DMC_CFG_DRAM_TYPE_DDR4      0x7  /* DDR4 */
#define DMC_CFG_DRAM_TYPE_LPDDR4    0x8  /* LPDDR4 */
#define DMC_CFG_MEM_2BG             0x1  /* 2 Bank Group */
#define DMC_CFG_INIT_XSREF          0x8  /* bit[3] */
#define DMC_CFG_SREF_PD             0x4  /* bit[2] */

#define DMC_CTRL_SREF_ENTER   0x1  /* 1 Enter Auto-self refresh */
#define DMC_CTRL_SREF_EXIT    0x2  /* 2 Exit Auto-self refresh */
#define DMC_RNKVOL_MEM_BG_4   0x2  /* 10: 4 Bank Group */
#define DMC_RNKVOL_MEM_BANK_16 0x2  /* 10: 16 bank */
#define DMC_RNKVOL_MEM_ROW_17 0x6  /* 110: 17 bit */
#define DMC_RNKVOL_MEM_COL_10 0x2  /* 010: 10 bit */
#define DMC_DDRMODE_RANK_1    0x0  /* 00: 1 rank */
#define DMC_DDRMODE_RANK_2    0x1  /* 01: 2 rank */
#define DMC_TRFC_AB_350       0x15e /* 350ns */
#define DMC_TRFC_AB_550       0x226 /* 550ns */

/* memory width */
#define MEM_WIDTH_16BIT  0x1 /* 16bit */
#define MEM_WIDTH_32BIT  0x2 /* 32bit */

#ifndef DDR_DMC_PER_PHY_MAX
#define DDR_DMC_PER_PHY_MAX 2
#endif

#ifndef DDR_AXI_SWITCH_NUM
#define DDR_AXI_SWITCH_NUM 4 /* ddr training axi switch number */
#endif

#ifndef DDR_RANK_NUM
#define DDR_RANK_NUM 2 /* rank number */
#endif


/* ******* AXI ************************* */
/* *
 * DMC -- PHY
 * /
 * DDRT -- AXI
 * \
 * DMC -- PHY
 */
/* base address: DDR_REG_BASE_AXI */
/* register offset address */
#define DDR_AXI_ACTION         0x20  /* module action */
#define SPECIAL_INTLV_EN_BIT   3 /* special_intlv_en */
#define DDR_AXI_CHSEL_REMAP    0x40  /* Channel selection remapping */
#define DDR_AXI_REGION0_MAP1    0x100 /* Address area mapping 1 register 0 */
#define DDR_AXI_REGION1_MAP1    0x110 /* Address area mapping 1 register 1 */
#define DDR_AXI_REGION0_ATTRIB1 0x104 /* Address Area Attributes 1 rigion 0 */
#define DDR_AXI_REGION1_ATTRIB1 0x114 /* Address Area Attributes 1 rigion 1 */
#define DDR_AXI_REGION0_MAP2    0x804 /* Address area mapping 2 register 0 */
#define DDR_AXI_REGION1_MAP2    0x814 /* Address area mapping 2 register 1 */
#define DDR_AXI_REGION0_ATTRIB2 0x808 /* Address Area Attributes 2 rigion 0 */
#define DDR_AXI_REGION1_ATTRIB2 0x818 /* Address Area Attributes 2 rigion 1 */

/* register mask */
#define AXI_REGION_ATTRIB_CH_MASK 0xffffffc0 /* channel mask */

/* register value */
/* Map to the single channel, independent address */
#define AXI_RNG_ATTR_CH_MODE    0x8
#define AXI_RNG_ATTR_CH_START_0 0x0
#define AXI_RNG_ATTR_CH_START_1 0x1
#define AXI_RNG_ATTR_CH_START_2 0x2
#define AXI_RNG_ATTR_CH_START_3 0x3
#define AXI_RNG_NUM             2 /* region number */

#define AXI_CHSEL_REMAP_LPDDR4 0x76543210
#define AXI_CHSEL_REMAP_NONLPDDR4 0x76531420

/* QOS */
#define qos_ddrc_cfg_rnkvol(chs, rnks)  (0x4634 + ((chs) << 4) + ((rnks) << 2)) /* 4:channels 2:ranks */

/* *******data define*********************************** */
struct ddrtrn_ddrc_data {
	unsigned int region_attrib[AXI_RNG_NUM];
	unsigned int rnkvol;
	unsigned int chsel_remap;
};

struct ddrtrn_ddrc_rnkvol_data {
	unsigned int rnkvol[DDR_DMC_PER_PHY_MAX];
	unsigned int qos_rnkvol[DDR_DMC_PER_PHY_MAX];
};
#endif /* DDR_DDRC_V520_H */
