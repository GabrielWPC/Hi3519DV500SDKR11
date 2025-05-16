/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef DDR_SUSPEND_RESUME_H
#define DDR_SUSPEND_RESUME_H

#define DATA_PHY_BASE_ADDR 0x844000

#define lpds_ddrtrn_hal_phy_parab(y)     (0x1000 + ((y) << 2)) /* used to store DDR PHY parameters */
#define lpds_hpp_reseved_areac(y)        (0x1800 + ((y) << 2)) /* used to store DDR PHY parameters */

#define DDR_WDQPHASE_ADJUST_OFFSET (-8)

/* DDRPHY low power control register */
#define SYSCTRL_DDRPHY_LP_EN 0x58
#define SYS_DDRPHY_IO_OE_MASK   0x3 /* bit[5:4]sys_ddrphy_io_oe */
#define SYS_DDRPHY_LP_EN0_MASK  0x3 /* bit[1:0]sys_ddrphy_lp_en:ddrphy0 retention */
#define SYS_DDRPHY_LP_EN1_MASK  0x3 /* bit[3:2]sys_ddrphy_lp_en1:ddrphy1 retention */
#define SYS_DDRPHY_IO_OE_BIT    4 /* bit[5:4]sys_ddrphy_io_oe */
#define SYS_DDRPHY_LP_EN0_BIT   0 /* bit[1:0]sys_ddrphy_lp_en:ddrphy0 retention */
#define SYS_DDRPHY_LP_EN1_BIT   2 /* bit[3:2]sys_ddrphy_lp_en1:ddrphy1 retention */

struct ddrtrn_hal_phy_param_s {
	unsigned int reg_addr;
	unsigned int reg_num;
	unsigned int save_addr;
};
#endif /* DDR_SUSPEND_RESUME_H */