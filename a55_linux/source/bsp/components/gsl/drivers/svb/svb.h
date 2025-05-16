/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __SVB_H__
#define __SVB_H__
#include "platform.h"

#define HPM_STD_VOLT					990
#define HPM_NUM        				 	9
#define CYCLE_NUM						8
#define CORE_TRAINING   				40
#define CORE_TRAINING_LOW_VOLT			845
#define SVB_VER_REG						0x11020168
#define SVB_VER_VAL						0x01000000 // SVB VER

#define SYSCTRL_BASE_REG				0x11020000
#define UBOOT_REG_TSENSOR_CTRL			0x1102A000
#define DDR_PHY_REG						0x11148050
#define TSENSOR0_CTRL0					0x0
#define TSENSOR_CTRL0_CFG0				0xc08ffc00
#define TSENSOR0_CTRL1					0x4
#define TSENSOR_CTRL1_CFG0				0x0
#define TSENSOR_STATUS0					0x8

#define HPM_NPU_RVT40_CLK_REG			0x1101497C
#define HPM_CORE_RVT40_CLK_REG			0x11014980
#define HPM_CPU_HVT35_CLK_REG			0x11014984
#define HPM_CLK_CFG0					0x10

#define HPM_BASE_ADDR					0x1102B000
#define HPM_CTRL_VALUE 					0x60080003
#define HPM_LIMIT						0x3ff
#define CORE_HPM_CTRL0					0x100
#define CORE_HPM_CTRL1					0x104
#define CPU_HPM_CTRL0 					0x0
#define CPU_HPM_CTRL1					0x4
#define NPU_HPM_CTRL0					0x400
#define NPU_HPM_CTRL1					0x404
#define HPM_NPU_RVT40_REG0				0xb458
#define HPM_NPU_RVT40_REG1				0xb45c
#define HPM_CPU_HVT35_REG0				0xb048
#define HPM_CPU_HVT35_REG1				0xb04c
#define HPM_CORE_RVT40_REG0				0xb128
#define HPM_CORE_RVT40_REG1				0xb12c

#define CRG_BASE_ADDR					0x11010000
#define MISC_BASE_ADDR					0x11020000
#define CRG_CPU_PASENSOR_CK_RST_CTRL 	0x4990
#define CRG_NPU_PASENSOR_CK_RST_CTRL 	0x4994
#define CRG_PASENSOR_CLK_EN				0x111
#define CRG_PASENSOR_SRST_REQ			0x110
#define PASENSOR_CFG0_VAL0   			0x11a
#define PASENSOR_CFG0_VAL1   			0x1013a
#define PASENSOR_CFG0_VAL2   			0x101013a
#define PASENSOR_CFG1_VAL   			0x331

// cpu pasensor
#define CPU_PASENSOR_CFG0   			0x4704
#define CPU_PASENSOR_CFG1   			0x4708
#define CPU_PASENSOR_OUT0   			0x470c
#define CPU_PASENSOR_OUT1   			0x4710
// npu pasensor
#define NPU_PASENSOR_CFG0   			0x4714
#define NPU_PASENSOR_CFG1   			0x4718
#define NPU_PASENSOR_OUT0   			0x471c
#define NPU_PASENSOR_OUT1   			0x4720

#define SVB_VERSION_ADDR  				0X0134
#define SYSCTRL_REG						0x11020000
#define HPM_STORAGE_REG 				0x340
#define CORE_V_STORAGE_REG 				0x344
#define CPU_V_STORAGE_REG 				0x348
#define NPU_V_STORAGE_REG 				0x34c
#define TEMP_PA_STORAGE_REG 			0x350
#define DYN_COMP_FALG_STORAGE_REG 		0x354
#define PWM_CORE_RVT40_VOL_REG	    	(SYSCTRL_REG + 0x9000)
#define PWM_CPU_HVT35_VOL_REG			(SYSCTRL_REG + 0x9004)
#define PWM_NPU_RVT40_VOL_REG			(SYSCTRL_REG + 0x9008)

#define OTP_BASE_REG					0x101E0000
#define OTP_VERSION_ID_REG				0x0120
#define OTP_HPM_OFFSET  				0x0128
#define OTP_DELTA_CORE_OFFSET 			0x010c
#define OTP_DELTA_CPU_NPU_OFFSET 		0x0124
#define OTP_19D_VERSION_ID				0x220101
#define OTP_16D_VERSION_ID				0x220102

#define CORE_CURVE_VOLT_MAX19			1000
#define CORE_CURVE_VOLT_MIN19			866
#define CORE_CURVE_B19					144400000
#define CORE_CURVE_A19					175000

#define CPU_CURVE_VOLT_MAX19			1049
#define CPU_CURVE_VOLT_MIN19			858
#define CPU_CURVE_B19					143663157
#define CPU_CURVE_A19					180817

#define NPU_CURVE_VOLT_MAX19			1049
#define NPU_CURVE_VOLT_MIN19			904
#define NPU_CURVE_B19					156690159
#define NPU_CURVE_A19					200773

#define CORE_CURVE_VOLT_MAX16			970
#define CORE_CURVE_VOLT_MIN16			855
#define CORE_CURVE_B16					135270000
#define CORE_CURVE_A16					153100

#define CPU_CURVE_VOLT_MAX16			1049
#define CPU_CURVE_VOLT_MIN16			829
#define CPU_CURVE_B16					133912057
#define CPU_CURVE_A16					159300

#define NPU_CURVE_VOLT_MAX16			1047
#define NPU_CURVE_VOLT_MIN16			842
#define NPU_CURVE_B16					148611136
#define NPU_CURVE_A16					195110

#define CORE_CURVE_VOLT_MAX19_PT		970
#define CORE_CURVE_VOLT_MIN19_PT		845
#define CORE_CURVE_B19_PT				135270000
#define CORE_CURVE_A19_PT				153100

#define CPU_CURVE_VOLT_MAX19_PT			1049
#define CPU_CURVE_VOLT_MIN19_PT			810
#define CPU_CURVE_B19_PT				133412057
#define CPU_CURVE_A19_PT				159300

#define NPU_FIX_VOLT_VAL_PT				830
#define NPU_CURVE_VOLT_MAX19_PT			1049
#define NPU_CURVE_VOLT_MIN19_PT			810
#define NPU_CURVE_B19_PT				145611136
#define NPU_CURVE_A19_PT				195110

#define CORE_CURVE_VOLT_MAX19_BATTERY			980
#define CORE_CURVE_VOLT_MIN19_BATTERY			846
#define CORE_CURVE_B19_BATTERY					142400000
#define CORE_CURVE_A19_BATTERY					175000

#define CPU_CURVE_VOLT_MAX19_BATTERY			1037
#define CPU_CURVE_VOLT_MIN19_BATTERY			838
#define CPU_CURVE_B19_BATTERY					141663157
#define CPU_CURVE_A19_BATTERY					180817

#define NPU_CURVE_VOLT_MAX19_BATTERY			1029
#define NPU_CURVE_VOLT_MIN19_BATTERY			884
#define NPU_CURVE_B19_BATTERY					154690159
#define NPU_CURVE_A19_BATTERY					200773

#define CORE_CURVE_VOLT_MAX16_BATTERY			950
#define CORE_CURVE_VOLT_MIN16_BATTERY			835
#define CORE_CURVE_B16_BATTERY					133270000
#define CORE_CURVE_A16_BATTERY					153100

#define CPU_CURVE_VOLT_MAX16_BATTERY			1032
#define CPU_CURVE_VOLT_MIN16_BATTERY			809
#define CPU_CURVE_B16_BATTERY					131912057
#define CPU_CURVE_A16_BATTERY					159300

#define NPU_CURVE_VOLT_MAX16_BATTERY			1027
#define NPU_CURVE_VOLT_MIN16_BATTERY			822
#define NPU_CURVE_B16_BATTERY					146611136
#define NPU_CURVE_A16_BATTERY					195110

#define CORE_CURVE_VOLT_MAX19_PT_BATTERY		950
#define CORE_CURVE_VOLT_MIN19_PT_BATTERY		835
#define CORE_CURVE_B19_PT_BATTERY				133270000
#define CORE_CURVE_A19_PT_BATTERY				153100

#define CPU_CURVE_VOLT_MAX19_PT_BATTERY			1027
#define CPU_CURVE_VOLT_MIN19_PT_BATTERY			810
#define CPU_CURVE_B19_PT_BATTERY				131412057
#define CPU_CURVE_A19_PT_BATTERY				159300

#define NPU_FIX_VOLT_VAL_PT_BATTERY				810
#define NPU_CURVE_VOLT_MAX19_PT_BATTERY			1049
#define NPU_CURVE_VOLT_MIN19_PT_BATTERY			810
#define NPU_CURVE_B19_PT_BATTERY				142611136
#define NPU_CURVE_A19_PT_BATTERY				195110


#define CORE_VOLT_MAX0					1051
#define CORE_VOLT_MIN0					649

#define CPU_VOLT_MAX0					1049
#define CPU_VOLT_MIN0					649

#define NPU_VOLT_MAX0					1051
#define NPU_VOLT_MIN0					649

#define CORE_SVB_MAX0					1010
#define CORE_SVB_MIN0					825

#define CPU_SVB_MAX0					1049
#define CPU_SVB_MIN0					810

#define NPU_SVB_MAX0					1049
#define NPU_SVB_MIN0					810

#define OTP_CPU_OFFSETS     			10
#define OTP_NPU_OFFSETS     			20

#define CORE_HPM_TEM_COM_BOUNDED    	260
#define CPU_HPM_TEM_COM_BOUNDED     	265
#define NPU_HPM_TEM_COM_BOUNDED     	255

#define TEMPERATURE_115             	115
#define TEMPERATURE_110             	110
#define TEMPERATURE_90              	90
#define TEMPERATURE_75              	75
#define TEMPERATURE_70              	70
#define TEMPERATURE_35              	35
#define TEMPERATURE_0               	0
#define TEMPERATURE_NEG20           	(-20)

#define TEMPERATURE_INTERVAL_0			0
#define TEMPERATURE_INTERVAL_1			1
#define TEMPERATURE_INTERVAL_2			2
#define TEMPERATURE_INTERVAL_3			3
#define TEMPERATURE_INTERVAL_4			4
#define TEMPERATURE_INTERVAL_5			5

#define WAITING_10_US					10
#define WAITING_20_US					20
#define WAITING_200_US					200
#define WAITING_10_MS					10
#define WAITING_6_MS					6
#define PA_IS_NEGATIVE					1
#define NEG_1							(-1)

#define LOW_TEMPERATURE_V				930
#define LOW_TEMPERATURE					25
#define PWM_PERIOD						416
#define FORMULA_MULTIPLES           	100000

#define TEMPERATURE_A					132
#define TEMPERATURE_B					165
#define TEMPERATURE_C					808
#define TEMPERATURE_D					40

#define	DUTY_ONE						1
#define	DUTY_TWO						2

#define	DELTA_FORMULA_MULTIPLES			10000

#define	CORE_HPM_BOUND			270

enum hpm_record {
	HPM_RECORD0 = 0,
	HPM_RECORD1,
	HPM_RECORD2,
	HPM_RECORD3,
	HPM_RECORD_BUTT
};

enum temperature_zone {
	TEMPERATURE_ZONE_110 = 0,
	TEMPERATURE_ZONE_90,
	TEMPERATURE_ZONE_70,
	TEMPERATURE_ZONE_NEG20,
	TEMPERATURE_ZONE_BUTT
};

enum product_type {
	SVB_NONE = 0,
	SVB_19DV5,
	SVB_19DV5_PT,
	SVB_16DV5,
	SVB_59V300
};

enum ddr_type {
	LPDDR3 = 3,
	DDR3 = 6,
	DDR4 = 7,
	LPDDR4_4X = 8
};

enum batteries_product_type {
	NO_BATTERIES = 0,
	YES_BATTERIES
};

typedef struct {
	int coef_k;
	int coef_b;
} svb_coef;

typedef union {
	struct {
		unsigned int work_v	: 16;	/* [15..0] */
		unsigned int cacl_v	: 16;	/* [31..16] */
	} bits;
	unsigned int u32;
} u_storage_reg;

typedef union {
	struct {
		unsigned int svb_pwm_enable	: 1;	/* [0..0] */
		unsigned int svb_pwm_inv    : 1;	/* [1..1] */
		unsigned int svb_pwm_load	: 1;	/* [2..2] */
		unsigned int reserved0      : 1;	/* [3..3] */
		unsigned int svb_pwm_period	: 10;	/* [13..4] */
		unsigned int reserved1	    : 2;	/* [15..14] */
		unsigned int svb_pwm_duty	: 10;	/* [25..16] */
		unsigned int reserved2	    : 6;	/* [31..26] */
	} bits;
	unsigned int u32;
} u_svb_pwm_ctrl;

typedef union {
	struct {
		int core_delta_v			: 7;	/* [6..0] */
		unsigned int core_flag		: 1;	/* [7] */
		unsigned int reserved		: 24;	/* [31..8] */
	} bits;
	unsigned int u32;
} u_core_delta_v_reg;

typedef union {
	struct {
		unsigned int reserved		: 16;	/* [15..0] */
		int cpu_delta_v				: 7;	/* [22..16] */
		unsigned int cpu_flag		: 1;	/* [23] */
		int npu_delta_v				: 7;	/* [30..24] */
		unsigned int npu_flag		: 1;	/* [31] */
	} bits;
	unsigned int u32;
} u_cpu_npu_delta_v_reg;

typedef union {
	struct {
		unsigned int hpm0		: 10;	/* [9..0] */
		unsigned int reserved_0	: 6;	/* [15..10] */
		unsigned int hpm1		: 10;	/* [25..16] */
		unsigned int reserved_1	: 6;	/* [31..26] */
	} bits;
	unsigned int u32;
} u_hpm_reg;

typedef union {
	struct {
		unsigned int temp_value		    : 8;	/* [7..0] */
		unsigned int temp_negative	    : 4;	/* [11..8] */
		unsigned int temp_range		    : 4;	/* [15..12] */
		int cpu_pa_value	    		: 7;	/* [22..16] */
		unsigned int cpu_pa_negative	: 1;	/* [23..23] */
		int npu_pa_value	    		: 7;	/* [30..24] */
		unsigned int npu_pa_negative	: 1;	/* [31..31] */
	} bits;
	unsigned int u32;
} u_temp_pa_storage_reg;

typedef union {
	struct {
		unsigned int reserved0		    : 16;	/* [15..0] */
		unsigned int start_up_temp	    : 8;	/* [23..16] */
		unsigned int temp_negative      : 1;	/* [24..24] */
		unsigned int reserved1		    : 7;	/* [31..25] */
	} bits;
	unsigned int u32;
} u_dyn_comp_falg_storage_reg;

typedef union {
	struct {
		int pasensor_nbti_o_data	: 9;	/* [8..0] */
		unsigned int reserved0	    : 7;	/* [15..9] */
		int pasensor_nbti_a_data	: 9;	/* [24..16] */
		unsigned int reserved1	    : 7;	/* [31..25] */
	} bits;
	unsigned int u32;
} u_pasensor_out;

typedef union {
	struct {
		unsigned int core_hpm_value	: 10;	/* [9..0] */
		unsigned int cpu_hpm_value	: 10;	/* [19..10] */
		unsigned int npu_hpm_value	: 10;	/* [29..20] */
		unsigned int use_board_hpm	: 1;	/* [30..30] */
		unsigned int reserved	    : 1;	/* [31..31] */
	} bits;
	unsigned int u32;
} u_hpm_storage_reg;

typedef union {
	struct {
		unsigned int battery_type	: 2;	/* [1..0] */
		unsigned int svb_type		: 4;	/* [5..2] */
		unsigned int reserved	    : 26;	/* [31..6] */
	} bits;
	unsigned int u32;
} u_svb_version_reg;

typedef union {
	struct {
		unsigned int dram_type		: 4;	/* [3..0] */
		unsigned int reserved	    : 28;	/* [31..4] */
	} bits;
	unsigned int u32;
} u_ddr_phy_reg;

typedef struct {
	int curve_a;
	int curve_b;
	int volt_min;
	int volt_max;
	int volt_fix;
} svb_curve;

typedef struct {
	svb_curve core_curve;
	svb_curve cpu_curve;
	svb_curve npu_curve;
} svb_cfg;

void start_svb(void);
void end_svb(void);

#endif