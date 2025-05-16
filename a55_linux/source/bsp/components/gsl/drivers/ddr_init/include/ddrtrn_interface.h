/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef DDR_INTERFACE_H
#define DDR_INTERFACE_H

#define DDR_PHY_BYTE_MAX 4
#define DDR_PHY_BIT_NUM  8
/* support max bit 32 */
#define DDR_PHY_BIT_MAX (DDR_PHY_BYTE_MAX * DDR_PHY_BIT_NUM)

#define DDR_REG_NAME_MAX 32 /* register name */
#define DDR_CA_ADDR_MAX  10

#define DDR_SUPPORT_PHY_MAX  3 /* support max phy number */
#define DDR_SUPPORT_RANK_MAX 2 /* support max rank number */
#define DDR_CK_RESULT_MAX    2 /* DCC CK result number */

#define DDR_INT_MAX    0x7FFFFFFF
#define DDR_INT_MIN    (-DDR_INT_MAX - 1)
#define DDR_UINT_MAX   0xFFFFFFFFU

/*
 * DDR training register number:
 * WDQS              4
 * WDQ Phase         4
 * WDQ BDL           8
 * WDM               4
 * Write DQ/DQS OE   4
 * RDQS              4
 * RDQ BDL           8
 * Gate              4
 * CS                1
 * CLK               1
 * Host Vref         4
 * DRAM Vref         4
 * CA Phase          1
 * CA BDL            5
 * -------------------
 * 60
 */
#define DDR_TRAINING_REG_NUM 60
/* register max. */
#define DDR_TRAINING_REG_MAX (DDR_TRAINING_REG_NUM * DDR_SUPPORT_PHY_MAX)

#define DDR_TRAINING_CMD_SW       (1 << 0)
#define DDR_TRAINING_CMD_HW       (1 << 1)
#define DDR_TRAINING_CMD_MPR      (1 << 2)
#define DDR_TRAINING_CMD_WL       (1 << 3)
#define DDR_TRAINING_CMD_GATE     (1 << 4)
#define DDR_TRAINING_CMD_DATAEYE  (1 << 5)
#define DDR_TRAINING_CMD_VREF     (1 << 6)
#define DDR_TRAINING_CMD_AC       (1 << 7)
#define DDR_TRAINING_CMD_LPCA     (1 << 8)
#define DDR_TRAINING_CMD_SW_NO_WL (1 << 9)
#define DDR_TRAINING_CMD_CONSOLE  (1 << 10)
#define DDR_TRAINING_CMD_DCC      (1 << 11)
#define DDR_TRAINING_CMD_PCODE    (1 << 12)
#define DDR_TRAINING_CMD_DPMC     (1 << 13)

#define DDR_TRAINING_BOOT_RESULT_ADDR (TEXT_BASE + 0x1000000) /* boot + 16M */

#define DDR_TRAINING_VER "V2.2.3 20230531"
#define DDR_VERSION    0x223
struct ddrtrn_training_result_data {
	unsigned int ddrtrn_bit_result[DDR_PHY_BIT_MAX];
	unsigned int ddrtrn_bit_best[DDR_PHY_BIT_MAX];
	unsigned int ddrtrn_win_sum;
};

struct ddrtrn_training_data {
	unsigned int base_phy;
	unsigned int byte_num;
	unsigned int rank_idx;
	struct ddrtrn_training_result_data read;
	struct ddrtrn_training_result_data write;
	unsigned int ca_addr[DDR_CA_ADDR_MAX];
};

struct ddrtrn_rank_data {
	unsigned int item;
	struct ddrtrn_training_data ddrtr_data;
};

struct ddrtrn_phy_data {
	unsigned int rank_num;
	struct ddrtrn_rank_data rank[DDR_SUPPORT_RANK_MAX];
};

struct ddrtrn_training_result {
	unsigned int phy_num;
	struct ddrtrn_phy_data phy[DDR_SUPPORT_PHY_MAX];
};

struct ddrtrn_reg_val {
	unsigned int rank_index;
	unsigned int byte_index;
	unsigned int offset;
	unsigned int val;
	char name[DDR_REG_NAME_MAX];
};

struct ddrtrn_reg_val_conf {
	unsigned int offset;
	unsigned int val;
};

struct ddrtrn_cmd {
	unsigned int cmd;
	unsigned int level;
	unsigned int start;
	unsigned int length;
};

typedef struct ddrtrn_training_result *(*ddrtrn_cmd_entry_func)(struct ddrtrn_cmd *cmd);

/* DDR training interface before boot */
int ddrtrn_sw_training_if(unsigned int sw_item_mask);
int ddrtrn_hw_training_if(void);
int ddrtrn_pcode_training_func(void);
int ddrtrn_training_console_if(void);
void ddrtrn_suspend_store_para(void);
int ddrtrn_resume(void);
int ddrtrn_hw_training_init(unsigned int hw_item_mask, int low_freq_flag);

/* DDR training check interface when boot */
struct ddrtrn_training_result *ddrtrn_cmd_training_if(struct ddrtrn_cmd *cmd);
int check_ddr_training(void);

/* DDR training command interface after boot */
void ddrtrn_reg_result_display(const struct ddrtrn_training_result *ddrtr_result);
void ddrtrn_cmd_result_display(const struct ddrtrn_training_result *ddrtr_result, unsigned int cmd);
void ddrtrn_hal_cmd_prepare_copy(void);
void ddrtrn_hal_cmd_site_save(void);
void ddrtrn_hal_cmd_site_restore(void);
#endif /* DDR_INTERFACE_H */
