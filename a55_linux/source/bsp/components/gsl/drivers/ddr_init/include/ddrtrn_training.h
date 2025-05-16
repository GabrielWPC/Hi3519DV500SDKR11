/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef DDR_TRAINING_IMPL_H
#define DDR_TRAINING_IMPL_H

#include "share_drivers.h"
#include "ddrtrn_boot_adapt.h"
#include "ddrtrn_training_internal_config.h"
#include "ddrtrn_interface.h"

#ifndef CONFIG_MINI_BOOT
extern char g_ddrtrn_training_cmd_start[]; /* DDR training code start address */
extern char g_ddrtrn_training_cmd_end[];   /* DDR training code end address */
#endif

/* ***** special config define****************************************** */
#ifdef DDR_DATAEYE_NORMAL_NOT_ADJ_CONFIG
/* Adjust dataeye window consume a lot of time, disable it will make boot
 * faster.
 * NOTE: The WDQ Phase and RDQS MUST be config a good value in the init table
 * to avoid window trend to one side.
 */
#define DDR_DATAEYE_NORMAL_ADJUST (DDR_FALSE)
#else
#define DDR_DATAEYE_NORMAL_ADJUST (DDR_TRUE)
#endif
/* MUST adjust dataeye window after HW or MPR training */
#define DDR_DATAEYE_ABNORMAL_ADJUST (DDR_TRUE)

#define unused(x) (void)(x)

/****** ddr training item bypass mask define ****************************/
#define DDR_BYPASS_PHY0_MASK           0x1        /* [0]PHY0 training */
#define DDR_BYPASS_PHY1_MASK           0x2        /* [1]PHY1 training */
#define DDR_BYPASS_PHY2_MASK           0x4        /* [2]PHY2 training */
#define DDR_BYPASS_WL_MASK             0x10       /* [4]Write leveling */
#define DDR_BYPASS_GATE_MASK           0x100      /* [8]Gate training */
#define DDR_BYPASS_DATAEYE_MASK        0x10000    /* [16]Dataeye training */
#define DDR_BYPASS_PCODE_MASK          0x40000    /* [18]Pcode training */
#define DDR_BYPASS_HW_MASK             0x100000   /* [20]Hardware read training */
#define DDR_BYPASS_MPR_MASK            0x200000   /* [21]MPR training */
#define DDR_BYPASS_AC_MASK             0x400000   /* [22]AC training */
#define DDR_BYPASS_LPCA_MASK           0x800000   /* [23]LPDDR CA training */
#define DDR_BYPASS_VREF_HOST_MASK      0x1000000  /* [24]Host Vref training */
#define DDR_BYPASS_VREF_DRAM_MASK      0x2000000  /* [25]DRAM Vref training */
#define DDR_BYPASS_DCC_MASK            0x08000000 /* [27]DCC training */
#define DDR_BYPASS_DATAEYE_ADJ_MASK    0x10000000 /* [28]Dataeye adjust */
#define DDR_BYPASS_WL_ADJ_MASK         0x20000000 /* [29]WL write adjust */
#define DDR_BYPASS_HW_ADJ_MASK         0x40000000 /* [30]HW read adjust */
#define DDR_BYPASS_ALL_MASK            0xffffffff /* all bypass */

#define DDR_MODE_READ  (1 << 0)
#define DDR_MODE_WRITE (1 << 1)

#define DDR_ENTER_SREF (1 << 0)
#define DDR_EXIT_SREF  (1 << 1)

#define DDR_HWR_WAIT_TIMEOUT  0xffffffff
#define DDR_SFC_WAIT_TIMEOUT  1000
#define DDR_LPCA_WAIT_TIMEOUT 1000
#define DDR_LOOP_COUNT        0xffffffff

#ifdef CFG_EDA_VERIFY
#define DDR_AUTO_TIMING_DELAY   10
#define DDR_SET_FRE_DELAY_100NS 10
#define DDR_SET_FRE_DELAY_1US   2000
#define DDR_SET_FRE_DELAY_10US  2000
#define DDR_SET_FRE_DELAY_50US  2000
#define DDR_SET_FRE_DELAY_100US 5000
#else
#define DDR_AUTO_TIMING_DELAY   1000
#define DDR_SET_FRE_DELAY_100NS 200    /* wait 100ns */
#define DDR_SET_FRE_DELAY_1US   2000   /* wait 1 us */
#define DDR_SET_FRE_DELAY_10US  20000  /* wait 10 us */
#define DDR_SET_FRE_DELAY_50US  100000 /* wait 50 us */
#define DDR_SET_FRE_DELAY_100US 200000 /* wait 100 us */
#endif

#define DDR_PLL_LOCK_TIME  10000
#define DDR_PLL_LOOP_NUM   1000

#define DDR_FIND_DQ_BOTH  (1 << 0) /* find a valid value */
/* x is valid, (x-1) is invalid */
#define DDR_FIND_DQ_LEFT  (1 << 1)
/* x is valid, (x+1) is invalid */
#define DDR_FIND_DQ_RIGHT (1 << 2)

#define DDR_VREF_DRAM_VAL_MAX 0x32 /* 92.50%*VDDIO */
#define DDR_VREF_DRAM_VAL_MIN 0x0  /* 60.00%*VDDIO */

#define DDR_PHY_REG_DQ_NUM 4   /* one register has 4 DQ BDL */
#define DDR_PHY_BDL_DLY_NUM 12 /* 10 bdl and 2 phase */

#define DDR_PHY_CA_MAX 10
#define DDR_PHY_CA_REG_MAX (DDR_PHY_CA_MAX >> 1)

#define DDR_TRUE  1
#define DDR_FALSE 0

#define DDR_WIN_MIDDLE (1 << 0)
#define DDR_WIN_LEFT   (1 << 1)
#define DDR_WIN_RIGHT  (1 << 2)

#define DDR_DELAY_PHASE 1
#define DDR_DELAY_BDL   2

#ifndef DDR_DATAEYE_WIN_NUM
/* Dateeye window number. More bigger more slower when Vref training. */
#define DDR_DATAEYE_WIN_NUM 8
#endif
#ifndef DDR_LOOP_TIMES_LMT
/* Dataeye DQ deskew times for best result. More bigger more slower. */
#define DDR_LOOP_TIMES_LMT 1
#endif
#ifndef DDR_VREF_COMPARE_TIMES
/* Compare times when find best vref value. More bigger more slower. */
#define DDR_VREF_COMPARE_TIMES 3
#endif
#ifndef DDR_MPR_RDQS_FIND_TIMES
/* MPR Find first start rdqs times. More bigger, start rdqs more bigger. */
#define DDR_MPR_RDQS_FIND_TIMES 3
#endif
#ifndef DDR_VREF_COMPARE_STEP
/* Compare step when begin to find. More bigger, more mistake, more stable. */
#define DDR_VREF_COMPARE_STEP 3
#endif

#define DDR_REG_OFFSET      4 /* DDR register offset */
#define DDR_DQ_NUM_EACH_REG 4 /* Each bdl register includes four dqbdl */
#define DDR_BYTE_DQ         3 /* Shift left or shift right 3: 8 dq(1 byte) */
#define DDR_DQBDL_SHIFT_BIT 3 /* Shift left or shift right 3: 8 bit */

#define DDR_DATAEYE_RESULT_MASK 0xffff
#define DDR_DATAEYE_RESULT_BIT  16

#define DDR_WL_BDL_STEP   2 /* wl bdl step */
#define DDR_GATE_BDL_STEP 2 /* gate bdl step */
#define DDR_DQS_ADJ_STEP  1 /* WR/RD DQS adjust step */

#define DDR_DDRT_MODE_GATE    (1 << 0)
#define DDR_DDRT_MODE_DATAEYE (1 << 1)

#define DDR_CHECK_TYPE_DDRT (1 << 0)
#define DDR_CHECK_TYPE_MPR  (1 << 1)

#define DDR_MPR_BYTE_MASK 0xff
#define DDR_MPR_BIT_MASK  0x1
#define DDR_MPR_BYTE_BIT  16     /* 16 bit (2 byte) */
#define DDR_MPR_BYTE_SHIFT_BIT 3 /* 8 bit */

#define DDR_PHY_AC_TEST_VAL0 0x0
#define DDR_PHY_AC_TEST_VAL1 0xffffffff
#define DDR_PHY_AC_TEST_VAL2 0x55555555
#define DDR_PHY_AC_TEST_VAL3 0xaaaaaaaa

#define DDR_FREQ_STATUS            2 /* low freq and high freq */

/* [11:0] Error type */
/* 0x00000001 Write Leveling error */
#define DDR_ERR_WL              (1 << 0)
/* 0x00000002 Hardware Gatining error */
#define DDR_ERR_HW_GATING       (1 << 1)
/* 0x00000004 Sofeware Gatining error */
#define DDR_ERR_GATING          (1 << 2)
/* 0x00000008 DDRT test time out */
#define DDR_ERR_DDRT_TIME_OUT   (1 << 3)
/* 0x00000010 Hardware read dataeye error */
#define DDR_ERR_HW_RD_DATAEYE   (1 << 4)
/* 0x00000020 MPR error */
#define DDR_ERR_MPR             (1 << 5)
/* 0x00000040 Dataeye error */
#define DDR_ERR_DATAEYE         (1 << 6)
/* 0x00000080 LPDDR CA error */
#define DDR_ERR_LPCA            (1 << 7)

/* [13:12] Error phy */
/* 0x00001000 PHY0 training error */
#define DDR_ERR_PHY0            (1 << 12)
/* 0x00002000 PHY1 training error */
#define DDR_ERR_PHY1            (1 << 13)

#define DDR_ERR_BYTE_BIT 24  /* [28:24] Error DQ0-31 */
#define DDR_ERR_DQ_BIT   20  /* [22:20] Error Byte0-3 */

#define CMD_ENABLE    1
#define CMD_DISABLE   0
#define RES_ENABLE    1
#define RES_DISABLE   0
#define DDR_LOW_FREQ_START_TRUE    1  /* Executing Low Frequency Start */
#define DDR_LOW_FREQ_START_FALSE   0  /* Do not execute Low Frequency Start */

#define DDR_1_RANK  0x1 /* 1 rank */
#define DDR_2_RANK  0x2 /* 2 rank */
#define DDR_10_BIT  10 /* Move 10 bits */

#define DDR_SIZE_512M   0x200  /* unit:MB, equal to 512MB */
#define DDR_SIZE_1G     0x400  /* unit:MB, equal to 1GB */
#define DDR_SIZE_2G     0x800  /* unit:MB, equal to 2GB */
#define DDR_SIZE_4G     0x1000 /* unit:MB, equal to 4GB */

#define DDR_REG_LOW_16BIT_MASK    0x0000ffff /* Lower 16-bit mask of the register */
#define DDR_REG_HIGH_16BIT_MASK   0xffff0000 /* Upper 16-bit mask of the register */
#define DDR_REG_32BIT_MASK        0xffffffff /* 32-bit mask of the register */

/*******data define*********************************************/
#ifndef DDR_RELATE_REG_DECLARE
struct ddrtrn_hal_training_custom_reg {};
#endif

struct ddrtrn_dmc_cfg_sref {
	unsigned int val[DDR_DMC_PER_PHY_MAX];
};

struct ddrtrn_hal_bdl {
	unsigned int bdl[DDR_PHY_BYTE_MAX];
};

struct ddrtrn_hal_timing {
	unsigned int val[DDR_DMC_PER_PHY_MAX];
};

struct ddrtrn_rdqs_data {
	struct ddrtrn_hal_bdl origin;
	struct ddrtrn_hal_bdl rank[DDR_SUPPORT_RANK_MAX];
};

struct ddrtrn_hal_delay {
	unsigned int phase[DDR_PHY_BYTE_MAX];
	unsigned int bdl[DDR_PHY_BYTE_MAX];
};

struct ddrtrn_hal_training_relate_reg {
	unsigned int auto_ref_timing;
	unsigned int power_down;
	unsigned int dmc_scramb;
	unsigned int dmc_scramb_cfg;
	unsigned int misc_scramb;
	unsigned int ac_phy_ctl;
	unsigned int swapdfibyte_en;
	struct ddrtrn_hal_training_custom_reg custom;
	struct ddrtrn_ddrc_data ddrc;
};

struct ddrtrn_hal_trianing_dq_data {
	unsigned int dq03[DDR_PHY_BYTE_MAX]; /* DQ0-DQ3 BDL */
	unsigned int dq47[DDR_PHY_BYTE_MAX]; /* DQ4-DQ7 BDL */
	unsigned int rdqs[DDR_PHY_BYTE_MAX]; /* RDQS */
	unsigned int rdm[DDR_PHY_BYTE_MAX];  /* RDM */
	unsigned int wdm[DDR_PHY_BYTE_MAX];  /* WDM */
};

struct ddrtrn_hal_training_dq_adj {
	unsigned int wdqsdly;    /* DXNWDQSDLY */
	unsigned int wdqsphase;  /* wdqsphase */
	unsigned int wdqsbdl;    /* wdqsbdl */
	unsigned int wdqdly;     /* DXNWDQDLY */
	unsigned int wdqphase;   /* wdqsphase */
	unsigned int dxnwlsl;    /* DXNWLSL */
	unsigned int wlsl;       /* wlsl */
	unsigned int wdqs_sum;
	unsigned int wdqs_average;
};

struct ddrtrn_hal_training_dq_adj_rank {
	struct ddrtrn_hal_training_dq_adj byte_dq_adj[DDR_PHY_BYTE_MAX];
};

struct ddrtrn_hal_training_dq_adj_phy {
	struct ddrtrn_hal_training_dq_adj_rank rank_dq_adj[DDR_RANK_NUM];
};

struct ddrtrn_hal_training_dq_adj_all {
	struct ddrtrn_hal_training_dq_adj_phy phy_dq_adj[DDR_PHY_NUM];
};

struct ddrtrn_ck_phase {
	unsigned int ck_phase[PHY_ACCTL_DCLK_NUM];
};

struct ddrtrn_delay_ckph_phy {
	unsigned int rdqscyc_lowfreq[DDR_PHY_BYTE_MAX];
	unsigned int rdqscyc_highfreq[DDR_PHY_BYTE_MAX];
	unsigned int phase2bdl_lowfreq[DDR_PHY_BYTE_MAX];
	unsigned int phase2bdl_highfreq[DDR_PHY_BYTE_MAX];
	struct ddrtrn_ck_phase ckph_st[DDR_FREQ_STATUS]; /* Distinguish low-freq and high-freq scenarios */
};

struct ddrtrn_delay_ckph {
	struct ddrtrn_delay_ckph_phy tr_delay_ck[DDR_PHY_NUM];
};

struct ddrtrn_ca_bit {
	unsigned int bit_p;
	unsigned int bit_n;
};

struct ddrtrn_ca_data {
	unsigned int base_dmc;
	unsigned int base_phy;
	unsigned int done; /* whether all ca found bdl range */
	unsigned int min;  /* min left bound */
	unsigned int max;  /* max right bound */
	unsigned def[DDR_PHY_CA_REG_MAX];
	int left[DDR_PHY_CA_MAX];
	int right[DDR_PHY_CA_MAX];
	struct ddrtrn_ca_bit bits[DDR_PHY_CA_MAX];
};

struct ddrtrn_dcc_ck {
	unsigned int val[DDR_CK_RESULT_MAX];
	unsigned int win;
	unsigned int win_min_ctl;
	unsigned int win_max_ctl;
	unsigned int win_min_duty;
	unsigned int win_max_duty;
	unsigned int def_bp;
	unsigned int def_ctl;
	unsigned int def_duty;
	unsigned int idx_duty;
	unsigned int idx_duty_ctl;
	unsigned int idx_ctl;
	unsigned int bypass_ck_bit;
	unsigned int acioctl21_ctl_bit;
	unsigned int acioctl21_ck_bit;
};

struct ddrtrn_dcc_data {
	struct ddrtrn_hal_trianing_dq_data rank[DDR_RANK_NUM];
	struct ddrtrn_dcc_ck ck[DDR_CK_NUM];
	unsigned int item[DDR_CK_NUM];
	unsigned int ioctl21_tmp;
};

struct ddrtrn_hal_bdl_dly {
	unsigned int value[DDR_PHY_BDL_DLY_NUM];
};

struct ddrtrn_phy_ctrl_val {
	unsigned int ac_phy_ctrl1;
	unsigned int dx0_phy_ctrl;
	unsigned int dx1_phy_ctrl;
};

struct ddrtrn_phy_ctrl {
	struct ddrtrn_phy_ctrl_val phy_ctrl[DDR_PHY_NUM];
};

struct ddrtrn_rank_rnkvol_data {
	struct ddrtrn_ddrc_rnkvol_data rnkvol_rank[DDR_RANK_NUM];
};

struct ddr_capat_by_phy {
	unsigned int capat_low16bit;
	unsigned int capat_high16bit;
	unsigned int capacity;
};

struct ddr_capat_phy_all {
	struct ddr_capat_by_phy phy_capat[DDR_PHY_NUM];
	unsigned int ddr_capat_sum;
	unsigned int cur_phy_rank_num[DDR_PHY_NUM];
};

struct ddrtrn_dq_data_rank {
	struct ddrtrn_hal_trianing_dq_data rank[DDR_RANK_NUM];
};

struct ddrtrn_dq_data_phy {
	struct ddrtrn_dq_data_rank phy[DDR_PHY_NUM];
};

/*******Uart early function ***********************************************/
#ifndef DDR_PUTS
#define DDR_PUTS uart_early_puts
#endif
#ifndef DDR_PUT_HEX
#define DDR_PUT_HEX uart_early_put_hex
#endif
#ifndef DDR_PUTC
#define DDR_PUTC uart_early_putc
#endif

#if defined(DDR_TRAINING_UART_CONFIG) || defined(DDR_TRAINING_LOG_CONFIG)
extern void uart_early_puts(const char *s);
extern void uart_early_put_hex(const int hex);
extern void uart_early_putc(const char c);
#else
#undef DDR_PUTS
#undef DDR_PUT_HEX
#undef DDR_PUTC
#endif
/*******function interface define******************************************/
#ifndef DDR_SW_TRAINING_FUNC
#define DDR_SW_TRAINING_FUNC_PUBLIC
#define DDR_SW_TRAINING_FUNC ddrtrn_sw_training_func
#endif

#ifndef DDR_HW_TRAINING_FUNC
#define DDR_HW_TRAINING_FUNC_PUBLIC
#define DDR_HW_TRAINING_FUNC ddrtrn_hw_training_func
#endif

#ifndef DDR_TRAINING_CONSOLE
#define DDR_TRAINING_CONSOLE_PUBLIC
#define DDR_TRAINING_CONSOLE ddrtrn_training_console
#endif
/*******Custom function ***********************************************/
#ifndef ddr_training_ddrt_prepare_func
#define ddr_training_ddrt_prepare_func()
#endif
#ifndef ddr_training_save_reg_func
#define ddr_training_save_reg_func(relate_reg, mask)
#endif
#ifndef ddr_training_restore_reg_func
#define ddr_training_restore_reg_func(relate_reg)
#endif
/*******function define*********************************************/
/* asm function */
/* DSB to make sure the operation is complete */

#ifndef ddr_asm_dsb
#ifdef __riscv
#define ddr_asm_dsb()                     { __asm__ __volatile__("fence"); }
#else
#define ddr_asm_dsb() { \
		__asm__ __volatile__("dsb sy"); \
	}
#endif
#endif

#ifndef ddr_asm_nop
#define ddr_asm_nop()                     { __asm__ __volatile__("nop"); }
#endif

/* ***** common define ********************************************* */
/* special ddrt need special read and write register */
unsigned int ddrt_reg_read(unsigned int addr);
void ddrt_reg_write(unsigned int val, unsigned int addr);

int ddrtrn_sw_training_func(void);
int ddrtrn_training_boot_func(void);
int ddrtrn_training_cmd_func(void);

void *ddrtrn_set_data(void *b, int c, unsigned int len);
void *ddrtrn_copy_data(void *dst, const void *src, unsigned int len);
void ddrtrn_hal_cfg_init(void);
void ddrtrn_hal_hw_item_cfg(unsigned int form_value);
void ddrtrn_hal_sw_item_cfg(unsigned int form_value);
void ddrtrn_hal_training_delay(unsigned int cnt);
int ddrtrn_training_by_dmc(void);
int ddrtrn_training_by_rank(void);
int ddrtrn_training_by_phy(void);
int ddrtrn_training_all(void);
int ddrtrn_dataeye_training_func(void);
int ddrtrn_vref_training_func(void);
int ddrtrn_wl_func(void);
int ddrtrn_gating_func(void);
int ddrtrn_ac_training_func(void);
int ddrtrn_lpca_training_func(void);
int ddrtrn_dcc_training_func(void);
#ifdef DDR_MPR_TRAINING_CONFIG
int ddrtrn_mpr_training_func(void);
#endif
int ddrtrn_training_ctrl_easr(unsigned int sref_req);
void ddrtrn_sref_cfg(struct ddrtrn_dmc_cfg_sref *cfg_sref, unsigned int value);
void ddrtrn_sref_cfg_save(struct ddrtrn_dmc_cfg_sref *cfg_sref);
void ddrtrn_sref_cfg_restore(const struct ddrtrn_dmc_cfg_sref *cfg_sref);
void ddrtrn_hal_phy_reset(unsigned int base_phy);
void ddrtrn_hal_switch_rank_all_phy(unsigned int rank_idx);
void ddrtrn_hal_phy_cfg_update(unsigned int base_phy);
void ddrtrn_hal_phy_set_dq_bdl(unsigned int value);
int ddrtrn_hal_adjust_get_val(void);
void ddrtrn_hal_adjust_set_val(int val);
void ddrtrn_hal_training_adjust_wdq(void);
void ddrtrn_hal_training_adjust_wdqs(void);

int ddrtrn_hw_training(void);
int ddrtrn_pcode_training(void);
int ddrtrn_hal_pcode_get_cksel(void);
#ifdef DDR_MPR_TRAINING_CONFIG
int ddrtrn_mpr_check(void);
int ddrtrn_mpr_training(void);
#endif
int ddrtrn_write_leveling(void);
int ddrtrn_gate_training(void);
int ddrtrn_dataeye_training(void);
int ddrtrn_vref_training(void);
int ddrtrn_ac_training(void);
int ddrtrn_lpca_training(void);
int ddrtrn_dataeye_deskew(struct ddrtrn_training_result_data *training);
void ddrtrn_adjust_dataeye(struct ddrtrn_training_result_data *training);
void ddrtrn_result_data_save(const struct ddrtrn_training_result_data *training);
void ddrtrn_lpca_data_save(const struct ddrtrn_ca_data *data);
unsigned int ddrtrn_ddrt_get_test_addr(void);
int ddrtrn_ddrt_test(unsigned int mask, int byte, int dq);
int ddrtrn_dataeye_check_dq(void);
void ddrtrn_ddrt_init(unsigned int mode);
int ddrtrn_hal_check_bypass(unsigned int mask);
void ddrtrn_hal_save_reg(struct ddrtrn_hal_training_relate_reg *relate_reg, unsigned int mask);
void ddrtrn_hal_restore_reg(const struct ddrtrn_hal_training_relate_reg *relate_reg);
void ddrtrn_hal_training_switch_axi(void);
void ddrtrn_training_log(const char *func, int level, const char *fmt, ...);
void ddrtrn_hal_training_stat(unsigned int mask, unsigned int phy, int byte, int dq);
void ddrtrn_training_error(unsigned int mask, unsigned int phy, int byte, int dq);
void ddrtrn_training_start(void);
void ddrtrn_training_success(void);
unsigned int ddrtrn_hal_phy_get_byte_num(unsigned int base_dmc);
void ddrtrn_hal_set_timing(unsigned int base_dmc, unsigned int timing);
int ddrtrn_hw_dataeye_read(void);
void ddrtrn_save_rdqbdl_phy(struct ddrtrn_dq_data_phy *dq_phy);
void ddrtrn_restore_rdqbdl_phy(struct ddrtrn_dq_data_phy *dq_phy);

/* function ddr phy s14 */
void ddrtrn_hal_vref_get_host_max(unsigned int rank, int *val);
void ddrtrn_hal_vref_phy_host_get(unsigned long base_phy, unsigned int rank,
								  unsigned int byte_index, unsigned int *val);
void ddrtrn_hal_phy_vref_host_display_cmd(unsigned long base_phy, unsigned int rank, unsigned int byte_num);
void ddrtrn_hal_vref_phy_dram_get(unsigned long base_phy, unsigned int *val, unsigned int byte_index);
void ddrtrn_hal_phy_vref_dram_display_cmd(unsigned long base_phy, unsigned int byte_num);
void ddrtrn_hal_phy_dx_dpmc_display_cmd(unsigned long base_phy, unsigned int byte_num);
void ddrtrn_hal_phy_dcc_display_cmd(unsigned long base_phy);
void ddrtrn_hal_phy_addrph_display_cmd(unsigned long base_phy);
void ddrtrn_hal_phy_addrbdl_display_cmd(unsigned long base_phy);
/* PHY t28 DDR4 RDQS synchronize to RDM */
void ddrtrn_hal_phy_rdqs_sync_rdm(int val);
/* dqs swap */
void ddrtrn_hal_dqsswap_save_func(unsigned int *swapdfibyte_en, unsigned long base_phy);
void ddrtrn_hal_dqsswap_restore_func(unsigned int swapdfibyte_en, unsigned long base_phy);
void ddrtrn_hal_phy_switch_rank(unsigned int base_phy, unsigned int val);

/* function ddrc hal */
void ddrtrn_hal_axi_save_func(struct ddrtrn_hal_training_relate_reg *relate_reg);
void ddrtrn_hal_axi_restore_func(const struct ddrtrn_hal_training_relate_reg *relate_reg);
void ddrtrn_hal_axi_chsel_remap_func(void);
void ddrtrn_hal_dmc_sfc_cmd_write(unsigned int sfc_cmd, unsigned long addr);
void ddrtrn_hal_dmc_sfc_bank_write(unsigned int sfc_bank, unsigned long addr);
void ddrtrn_hal_axi_switch_func(void);
void ddrtrn_hal_dmc_set_sref_cfg(unsigned int i, unsigned int value);
void ddrtrn_hal_axi_special_intlv_en(void);
void ddrtrn_hal_timing8_trfc_ab_cfg_by_dmc(void);
void ddrtrn_hal_timing8_trfc_ab_cfg(struct ddr_capat_phy_all *capat_phy_all);

/* save rank0 for ddrt address */
void ddrtrn_hal_rnkvol_save_func(struct ddrtrn_hal_training_relate_reg *relate_reg, unsigned int base_dmc);
void ddrtrn_hal_rnkvol_restore_func(const struct ddrtrn_hal_training_relate_reg *relate_reg, unsigned int base_dmc);
/* set mem_row to 0 */
void ddrtrn_hal_rnkvol_set_func(void);

/* function hal phy */
void ddrtrn_hal_rdqs_sync(int val);
unsigned int ddrtrn_hal_phy_get_dq_bdl(void);
int ddrtrn_hal_hw_training_process(unsigned int item);
void ddrtrn_hal_rdqs_sync_rank_rdq(int offset);
void ddrtrn_hal_rdqbdl_adj(struct ddrtrn_hal_bdl_dly *bdl_dly_s);
void ddrtrn_hal_restore_dly_value(const struct ddrtrn_hal_training_dq_adj *wdq_st,
								  unsigned int base_phy, unsigned int rank_idx, unsigned int byte_idx);
void ddrtrn_hal_wdqs_bdl2phase(unsigned int base_phy, unsigned int rank, unsigned int byte_idx);

/* function hal adjust */
unsigned int ddrtrn_hal_adjust_get_average(void);
/* function hal ddrt */
int ddrtrn_ddrt_check(void);
unsigned int ddrtrn_hal_ddrt_get_mem_width(void);
unsigned int ddrtrn_hal_ddrt_get_addr(void);

/* function hal ddrc */
unsigned int ddrtrn_hal_ddrc_get_bank_group(void);
int ddrtrn_hal_ddrc_easr(unsigned int base_dmc, unsigned int sref_req);
void ddrtrn_hal_save_timing(struct ddrtrn_hal_timing *timing);
void ddrtrn_training_restore_timing(struct ddrtrn_hal_timing *timing);
unsigned int ddrtrn_hal_dmc_get_sref_cfg(unsigned int i);
unsigned int ddrtrn_hal_dmc_get_sref_cfg(unsigned int i);
unsigned int ddrtrn_hal_get_rank_size(void);

/* function hw */
int ddrtrn_hw_training_by_phy(void);

/* function hal hw */
int ddrtrn_hal_hw_restore_rdqsbdl(unsigned int bdl_0, unsigned int bdl_1, unsigned int index);
void ddrtrn_hal_hw_read_adj(void);
void ddrtrn_hal_training_get_rdqs(struct ddrtrn_hal_bdl *rdqs);
void ddrtrn_hal_hw_save_rdqsbdl(void);
int ddrtrn_hal_hw_dataeye_adapt(unsigned int *ddr_temp);
void ddrtrn_hal_ck_cfg(unsigned int base_phy);
void ddrtrn_hal_hw_rdqs_offset_cfg(unsigned int base_phy, unsigned int byte_num, unsigned int dram_type);

#ifdef DDR_WRITE_DM_DISABLE
int ddrtrn_hal_hw_restore_write_dm(unsigned int ddrtrn_temp);
#endif /* DDR_WRITE_DM_DISABLE */
int ddrtrn_hal_hw_dataeye_vref_set(void);
int ddrtrn_hal_hw_training_normal_conf(void);
void ddrtrn_hal_training_set_rdqs(struct ddrtrn_hal_bdl *rdqs);
void ddrtrn_hal_hw_clear_rdq(unsigned int i);
#ifdef DDR_OE_CONFIG
void ddrtrn_ac_oe_enable(void);
void ddrtrn_dummy_io_oe_enable(void);
#endif

/* function hal lpca */
#ifdef DDR_LPCA_TRAINING_CONFIG
void ddrtrn_hal_lpca_get_bit(unsigned int index, struct ddrtrn_ca_data *data);
void ddrtrn_hal_lpca_get_def(struct ddrtrn_ca_data *data);
void ddrtrn_lpca_restore_def(struct ddrtrn_ca_data *data);
void ddrtrn_lpca_set_bdl(unsigned int base_phy, unsigned int bdl);
void ddrtrn_lpca_update_bdl(struct ddrtrn_ca_data *data);
void ddrtrn_lpca_display(struct ddrtrn_ca_data *data);
void ddr_lpca_check(struct ddrtrn_ca_data *data, unsigned int bdl, unsigned int is_ca49);
unsigned int ddrtrn_hal_check_lowpower_support(void);
#endif

/* function hal dataeye */
unsigned int ddrtrn_hal_dataeye_get_dm(void);
void ddrtrn_hal_dataeye_set_dq_sum(unsigned int dq_sum);
unsigned int ddrtrn_hal_get_dq_type(void);
void ddrtrn_hal_set_dq_type(unsigned int dq_check_type);

/* function hal vref */
void ddrtrn_hal_vref_status_set(unsigned int val);
void ddrtrn_hal_vref_save_bdl(struct ddrtrn_hal_trianing_dq_data *dq_data);
void ddrtrn_hal_vref_restore_bdl(struct ddrtrn_hal_trianing_dq_data *dq_data);
/* function hal dcc */
void ddrtrn_hal_dcc_set_ioctl21(unsigned int ioctl21);
unsigned int ddrtrn_hal_dcc_get_ioctl21(void);
void ddrtrn_hal_dcc_rdet_enable(void);
unsigned int ddrtrn_hal_dcc_get_dxnrdbound(unsigned int byte_index);
void ddrtrn_hal_restore_rdqbdl(struct ddrtrn_hal_trianing_dq_data *rank_dcc_data);
unsigned int ddrtrn_hal_dcc_get_gated_bypass(void);
unsigned int ddrtrn_hal_dcc_get_ioctl21(void);
void ddrtrn_hal_save_rdqbdl(struct ddrtrn_hal_trianing_dq_data *rank_dcc_data);
void ddrtrn_hal_dcc_set_gated_bypass(unsigned int gated_bypass_temp);
void ddrtrn_dmc_auto_power_down_cfg(void);
void ddrtrn_retrain_enable(void);
int ddrtrn_hal_console_getc(void);
void ddrtrn_hal_dmc_auto_pd_by_phy(unsigned int base_phy, unsigned int dmc0, unsigned int dmc1);
unsigned int ddrtrn_hal_get_gt_status(unsigned int base_phy);
int ddrtrn_hal_check_sw_item(void);
int ddrtrn_hal_check_not_dcc_item(void);
void ddrtrn_hal_set_adjust(unsigned int adjust);
unsigned int ddrtrn_hal_get_adjust(void);
void ddrtrn_hal_clear_sysctrl_stat_reg(void);
void ddrtrn_hal_phy_wdqs_display_cmd(unsigned int base_phy, unsigned int rank, unsigned int byte_num);
void ddrtrn_hal_phy_wdqphase_display_cmd(unsigned int base_phy, unsigned int rank, unsigned int byte_num);
void ddrtrn_hal_phy_wdqbdl_display_cmd(unsigned int base_phy, unsigned int rank, unsigned int byte_num);
void ddrtrn_hal_phy_wdm_display_cmd(unsigned int base_phy, unsigned int rank, unsigned int byte_num);
void ddrtrn_hal_phy_wdq_wdqs_oe_display_cmd(unsigned int base_phy, unsigned int rank, unsigned int byte_num);
void ddrtrn_hal_phy_rdqs_display_cmd(unsigned int base_phy, unsigned int byte_num);
void ddrtrn_hal_phy_rdqbdl_display_cmd(unsigned int base_phy, unsigned int rank, unsigned int byte_num);
void ddrtrn_hal_phy_gate_display_cmd(unsigned int base_phy, unsigned int rank, unsigned int byte_num);
void ddrtrn_hal_phy_cs_display_cmd(unsigned int base_phy);
void ddrtrn_hal_phy_clk_display_cmd(unsigned int base_phy, unsigned int acphyctl7);
void ddrtrn_hal_set_sysctrl_cfg(unsigned int item);
#ifdef SYSCTRL_DDR_TRAINING_VERSION_FLAG
void ddrtrn_hal_version_flag(void);
#endif
/* low freq */
void ddrtrn_hal_get_dly_value(struct ddrtrn_hal_training_dq_adj *wdq,
							  unsigned int base_phy, unsigned int rank_idx, unsigned int byte_idx);
int ddrtrn_low_freq_start(unsigned int hw_item_mask);
unsigned int ddrtrn_hal_get_misc_val(unsigned int base_phy);
unsigned int ddrtrn_hal_get_trfc_threshold1_val(unsigned int base_phy);
unsigned int ddrtrn_hal_get_dmsel(unsigned int base_phy);
unsigned int ddrtrn_hal_get_phyctrl0(unsigned int base_phy);
unsigned int ddrtrn_hal_read_repeatedly(unsigned int base_addr, unsigned int offset_addr);
void ddrtrn_hal_enable_rdqs_anti_aging(unsigned int base_phy);
void ddrtrn_hal_set_misc_val(unsigned int misc_val, unsigned int base_phy);
unsigned int ddrtrn_hal_get_trfc_ctrl_val(unsigned int base_phy);
void ddrtrn_hal_set_trfc_ctrl(unsigned int trfc_ctrl, unsigned int base_phy);
unsigned int ddrtrn_hal_get_sysctrl_cfg(unsigned int cfg_offset_addr);
unsigned int ddrtrn_hal_adjust_get_rdqs(void);

void ddrtrn_reg_config(const struct ddrtrn_reg_val_conf *reg_val, unsigned int array_size);

unsigned int ddrtrn_capat_adapt_func(void);

#ifdef DDR_TRAINING_EXEC_TIME
void ddrtrn_hal_chip_timer_start(void);
void ddrtrn_hal_chip_timer_stop(void);
#endif
#endif /* DDR_TRAINING_IMPL_H */
