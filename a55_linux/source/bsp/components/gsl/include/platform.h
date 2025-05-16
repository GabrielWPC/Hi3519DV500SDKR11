/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __PLATFORM_VENDORLICON_H__
#define __PLATFORM_VENDORLICON_H__

#define reg_get(addr) 							(*(volatile unsigned int *)(uintptr_t)(addr))
#define reg_set(addr, val) 						(*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#define reg_read_val32(addr) 					(*(volatile unsigned int *)(uintptr_t)(addr))
#define reg_getbits(addr, pos, bits) 			((reg_read_val32(addr) >> (pos)) & (((unsigned int)1 << (bits)) - 1))
#define reg_setbits(addr, pos, bits, val) (reg_read_val32(addr) =           \
		(reg_read_val32(addr) & (~((((unsigned int)1 << (bits)) - 1) << (pos)))) | \
		((unsigned int)((val) & (((unsigned int)1 << (bits)) - 1)) << (pos)))
#define SRAM_BASE						CFG_RAM_BASE_ADDR
#define SRAM_SIZE						0x20000
#define SRAM_END						(SRAM_BASE + SRAM_SIZE)

#define SYSCNT_AREA_SIZE					0x200
#define SYSCNT_AREA_START_ADDR					(SRAM_END - SYSCNT_AREA_SIZE)

#define SYS_CNT_FREQ_24MHZ					24
#define SYS_CNT_RAM_UNIT					8
#define SYS_CNT_UNIT_HIGH_OFFSET				4
#define SYS_CNT_UNIT_LOW_OFFSET					0

#define SYS_CNT_HEAD_OFFSET					0 /* head info:0x55AA55AA 0x55AA55AA */
#define SYS_CNT_BOOTROM_START_OFFSET 				1
#define SYS_CNT_BOOTROM_END_OFFSET 				2
#define SYS_CNT_GSL_START_OFFSET 				3
#define SYS_CNT_UBOOT_START_OFFSET 				4

#define GSL_STACK_SIZE						0x1000
#define BOOTROM_BSS_SIZE					0xa00
#define STACK_ADDR						(SRAM_BASE + GSL_STACK_SIZE)
#define CP_STEP1_ADDR						(SRAM_BASE + GSL_STACK_SIZE + BOOTROM_BSS_SIZE)
/* GSL security reinforcement */
/* REE/TEE/TP KEY AREA */
#define FLASH_ROOT_PUB_KEY_SIZE					0x80
#define REE_FLASH_ROOT_PUB_KEY_ADDR 				CP_STEP1_ADDR
#define TEE_FLASH_ROOT_PUB_KEY_ADDR				(REE_FLASH_ROOT_PUB_KEY_ADDR + FLASH_ROOT_PUB_KEY_SIZE)
#define TP_FLASH_ROOT_PUB_KEY_ADDR				(TEE_FLASH_ROOT_PUB_KEY_ADDR + FLASH_ROOT_PUB_KEY_SIZE)

/* chipset feature */
#define CFCT_TABLE_KEY_AREA_ADDR				(TP_FLASH_ROOT_PUB_KEY_ADDR + FLASH_ROOT_PUB_KEY_SIZE)
#define CFCT_TABLE_KEY_AREA_SIZE				0x100
#define CFCT_TABLE_ADDR						(CFCT_TABLE_KEY_AREA_ADDR + CFCT_TABLE_KEY_AREA_SIZE)
#define CFCT_TABLE_SIZE						0x700

/* root public key image size + chipset feature image size = 0x1000 */
#define ROOT_PUB_KEY_AREA_SIZE					0x1000

/* GSL */
#define GSL_KEY_AREA_ADDR					(CP_STEP1_ADDR + ROOT_PUB_KEY_AREA_SIZE)
#define GSL_KEY_AREA_SIZE					0x100
#define TP_EXT_KEY_AREA_ADDR					(GSL_KEY_AREA_ADDR + GSL_KEY_AREA_SIZE)
#define TP_EXT_KEY_AREA_SIZE					0x100
#define GSL_CODE_INFO_ADDR					(TP_EXT_KEY_AREA_ADDR + TP_EXT_KEY_AREA_SIZE)
#define GSL_CODE_INFO_SIZE					0x200
#define GSL_CODE_AREA_ADDR					(GSL_CODE_INFO_ADDR + GSL_CODE_INFO_SIZE)

#define GSL_CODE_OFFSET						(ROOT_PUB_KEY_AREA_SIZE + GSL_KEY_AREA_SIZE + \
									TP_EXT_KEY_AREA_SIZE + GSL_CODE_INFO_SIZE)

#define MALLOC_SIZE 						0x8000

/* parameter && uboot */
#define REE_BOOT_KEY_AREA_SIZE					0x100
#define PARM_AREA_INFO_SIZE					0x200
#define FILL_LEN_256BYTE					0x100
#define UBOOT_CODE_INFO_SIZE					0x200
#define TEE_KEY_SIZE						0x100
#define TEE_CODE_INFO_SIZE					0x200
#define SECONDARY_CORE_STACK					0x0403f000

/*-----------------------------------------------------------------
 * sysctrl register
 ------------------------------------------------------------------ */
/* Sys ctrl register */
#define REG_SYSCTRL_BASE					0x11020000
#define REG_SYSCTRL_BASE					0x11020000
#define REG_SC_CTRL						0x0000
#define TIMEREN0_SEL						(1 << 16)
#define REG_SC_SYSRES						0x0004
#define OS_OTP_PO_SDIO_INFO					0x0408
#define REG_SC_SYSSTAT						0x0018
#define REG_SAVE_EMMC_OCR					0x0300
#define REG_OS_SYS_CTRL1					0x304
#define REG_PERI_EMMC_STAT					0x0404
#define EMMC_NORMAL_MODE					(0x1 << 3)
#define uart_boot_flag(x)					(((x) >> 4) & 0x1)
#define EMMC_BOOT_8BIT						(0x1 << 11)
#define SEC_BOOTRAM_CTRL   					0x6080
#define BOOTRAM_RAM_SEL    					(0x1 << 0)
#define REG_SC_GEN1						0x0134
#define REG_SC_GEN3						0x013C
#define REG_SC_GEN4						0x0140

#define PCIE_SLAVE_BOOT_CTL_REG   				REG_SC_GEN1
#define DDR_INIT_EXCUTE_OK_FLAG     				0xDCEFF002 /* step2:Ddrinit Code Excute Finished Flag:    DCEFF002 */
#define UBOOT_DOWNLOAD_OK_FLAG    			  	0xBCDFF003 /* step3:Boot Code Download Finished Flag:     BCDFF003 */
#define READ_FOR_HEAD_AREA_FLAG    				0xACBFF005
#define HEAD_AREA_OK_FLAG          				0xACBFF006
#define PCIE_SLAVE_PRINT_INTERVAL				10000

#define REG_TEE_LAUNCH_FAILURE					(REG_SYSCTRL_BASE + REG_OS_SYS_CTRL1)


#define SELF_BOOT_FLAG_REG_OFFSET				0x14
#define SELF_BOOT_FLAG_REG_ADDR					(REG_SYSCTRL_BASE + SELF_BOOT_FLAG_REG_OFFSET)

/* SELF_BOOT_DEV_REG_ADDR [3:0] */
#define BURN_MODE_BIT_OFFSET					0x0
#define BURN_MODE_BIT_WIDTH					0x4

#define UART_NO_NUDE						0x1 /* [3:0] */
#define USB_DEVICE_NO_NUDE					0x2 /* [3:0] */
#define USB_HOST_NO_NUDE					0x3 /* [3:0] */
#define SD_CARD_NO_NUDE						0x4 /* [3:0] */

#define UART_DDR						0x1 /* [3:0] */
#define USB_DDR							0x1 /* [3:0] */

/* SELF_BOOT_DEV_REG_ADDR [7:0] */
#define SELF_BOOT_DEV_REG_BIT_OFFSET				0x0
#define SELF_BOOT_DEV_REG_BIT_WIDTH				0x8
#define SELF_BOOT_FLASH						0x0 /* [7:4] */
#define SELF_BOOT_UART						0x1 /* [7:4] */
#define SELF_BOOT_USB_DEVIDE					0x2 /* [7:4] */
#define SELF_BOOT_USB_HOST					0x3 /* [7:4] */ /* not support */
#define SELF_BOOT_SD_CARD					0x4 /* [7:4] */

/*-----------------------------------------------------------------
 * CRG register
 ------------------------------------------------------------------ */
#define REG_BASE_CRG						0x11010000
#define REG_PERI_CRG4192					0x4180
#define UART0_CLK_ENABLE					(0x1 << 4)
#define UART0_CLK_MASK						(0x3 << 12)
#define UART0_CLK_24M						(0x1 << 12)
#define UART0_SRST_REQ						(0x1 << 0)
#define REG_PERI_CRG4048					(0x3f40)
#define REG_PERI_CRG3376					(0x34c0)


/*-----------------------------------------------------------------
 * CA_MISC register
 *------------------------------------------------------------------ */
#define REG_BASE_CA_MISC					0x101E8000
#define SCS_CTRL						0x400
#define DISABLE                        	 			0x42

#define  REE_FLASH_ROOTKEY_STATUS_ADDR   			REG_BASE_CA_MISC + 0x150
#define  REE_FLASH_ROOTKEY_STATUS_VALID  			(0x5 << 0)
#define  REE_FLASH_ROOTKEY_STATUS_MASK   			(0xf << 0)

#define TEE_FAILURE_SOURCE      				(REG_BASE_CA_MISC + 0x170) /* b[3:0]) */
#define TEE_FAILURE_SOURCE_VMASK				0xf
#define set_tee_failure_source_type(val, type)			(((val) & (~TEE_FAILURE_SOURCE_VMASK)) | (type))
#define get_tee_failure_source_type(val)        		((val) & TEE_FAILURE_SOURCE_VMASK)

#define TEE_VER_EN_FLAG_ADDR					(REG_BASE_CA_MISC + 0x154)
#define TEE_VER_EN_FLAG_BIT_OFFSET				0
#define TEE_VER_EN_FLAG_BIT_WIDTH				8

#define REE_VER_EN_FLAG_ADDR					TEE_VER_EN_FLAG_ADDR
#define REE_VER_EN_FLAG_BIT_OFFSET				8
#define REE_VER_EN_FLAG_BIT_WIDTH				8

#define TP_VER_EN_FLAG_ADDR				TEE_VER_EN_FLAG_ADDR
#define TP_VER_EN_FLAG_BIT_OFFSET			16
#define TP_VER_EN_FLAG_BIT_WIDTH				8
/*-----------------------------------------------------------------
 * serial base address and clock
 ------------------------------------------------------------------ */
#define REG_BASE_SERIAL0					0x11040000
#define CONFIG_PL011_CLOCK					24000000	/* Serial need 24M clock input */
#define CONFIG_CONS_INDEX					0		/* select the default console */

/*-----------------------------------------------------------------
 * timer0 register
 ------------------------------------------------------------------ */
#define REG_BASE_TIMER0						0x11000000
#define TIMER_LOAD						0x0000
#define TIMER_VALUE						0x0004
#define TIMER_CONTROL						0x0008
#define TIMER_EN						(1<<7)
#define TIMER_MODE						(1<<6)
#define TIMER_PRE						(0<<2)
#define TIMER_SIZE						(1<<1)

#define CFG_TIMERBASE						REG_BASE_TIMER0
#define READ_TIMER              				(*(volatile unsigned int *)(CFG_TIMERBASE + TIMER_VALUE))
#define TIMER_FEQ						3000000
/* how many ticks per second. show the precision of timer. */
#define TIMER_DIV						1
#define CONFIG_SYS_HZ						(TIMER_FEQ / TIMER_DIV)

/*-----------------------------------------------------------------------------------
 * pmc register
 *-----------------------------------------------------------------------------------*/
#define PMC_REG_BASE						0x11120000 /* pmc base addr */

#define BOOT_IMG_ADDR_REG_ADDR					(PMC_REG_BASE + 0x014)
#define BOOT_IMG_SIZE_REG_ADDR					(PMC_REG_BASE + 0x018)

#define PWR_USER_REG2                  	 			(PMC_REG_BASE + 0x01C)
#define LOW_POWER_MODE_REG_ADDR					PWR_USER_REG2
#define PWR_HASH_VALUE_LOCK_REG         			(PMC_REG_BASE + 0x02C)
#define LOW_POWER_MODE_REG_BIT_OFFSET      			0x0
#define LOW_POWER_MODE_REG_BIT_WIDTH      			0x8
#define PWR_HASH_VALUE_LOCK_ENABLE      			0x1
#define PWR_HASH_VALUE_LOCK_DISABLE      			0xfffffffe

#define LPDS_LOCK_REG_BIT_OFFSET				0x0
#define PDS_LOCK_REG_BIT_WIDTH					0x1
#define SUSPEND_TO_LPDS         				0x5A
#define SUSPEND_TO_RAM          				0x69
#define SUSPEND_TO_DDR          				0x78
#define VERIFY_BACKUP_IMG_FLAG_BIT_OFFSET			8
#define VERIFY_BACKUP_IMG_FLAG_BIT_MASK				(0x3 << VERIFY_BACKUP_IMG_FLAG_BIT_OFFSET)
#define get_verify_backup_img_flag(val)				(((val) & VERIFY_BACKUP_IMG_FLAG_BIT_MASK) >> \
								 	VERIFY_BACKUP_IMG_FLAG_BIT_OFFSET)
#define set_verify_backup_img_flag(val, flag)			(((val) & (~VERIFY_BACKUP_IMG_FLAG_BIT_MASK)) | \
									((flag) << VERIFY_BACKUP_IMG_FLAG_BIT_OFFSET))

#define BACKUP_ENABLE						1
#define BOOT_FROM_PRIME						0x0
#define BOOT_FROM_BACKUP					0x1
#define BOOT_FROM_IMG_FAIL					0x2

#define PRIME_IMG_FLASH_OFFSET					0x0
#define BACKUP_IMG_FLASH_OFFSET					(64 * 1024)

#define BACKUP_IMG_FLAG_OFFSET					0x8
#define BACKUP_IMG_FLAG_WIDTH					0x2

#define LPDS_BIT_OFFSET						4
#define LPDS_REG_BASE_REG                    			(PMC_REG_BASE + 0x030)
#define LPDS_HASH_REG_SIZE                      		256 /* 256 byte */
#define LPDS_CFCT_HASH_ADDR                  			(LPDS_REG_BASE_REG)
#define LPDS_GSL_KEY_AREA_HASH_ADDR          			(LPDS_CFCT_HASH_ADDR + 32)
#define LPDS_TP_EXT_KEY_AREA_HASH_ADDR       			(LPDS_GSL_KEY_AREA_HASH_ADDR + 32)
#define LPDS_GSL_CODE_INFO_HASH_ADDR         			(LPDS_TP_EXT_KEY_AREA_HASH_ADDR + 32)
#define LPDS_REE_KEY_AREA_HASH_ADDR          			(LPDS_GSL_CODE_INFO_HASH_ADDR + 32)
#define LPDS_PARAM_INFO_HASH_ADDR            			(LPDS_REE_KEY_AREA_HASH_ADDR + 32)
#define LPDS_UBOOT_CODE_INFO_HASH_ADDR       			(LPDS_PARAM_INFO_HASH_ADDR + 32)
#define LPDS_ALL_HASH_ADDR                   			(LPDS_UBOOT_CODE_INFO_HASH_ADDR + 32)
/*-----------------------------------------------------------------
 * boot   Configuration
 ------------------------------------------------------------------ */

#define SPI_BASE_ADDR						0x0f000000
#define DDR_BASE_ADDR						0x40000000
#define DDR_DOWNLOAD_ADDR					0x41000000
#define SELF_BOOT_DOWNLOAD_MAGIC    				0x444f574e

#define SYS_STAT_REG	        				0x18

#define BACKUP_IMG_FLG_REG					0xC8
#define BACKUP_IMG_OFF_REG					0xCC
#define BACKUP_IMG_TIMES_REG					0x314
#define BACKUP_IMG_ADDR_REG					0x318
#define DATA_CHANNEL_TYPE_REG					0x31c
#define BACKUP_IMG_ENABLE					0x424945  /* Backup Image Enable */
#define BACKUP_IMG_FLG_BIT					4
#define BACKUP_IMG_FLG_MASK					(0xffffff << BACKUP_IMG_FLG_BIT)
#define BACKUP_IMG_OFF_BIT					4
#define BACKUP_IMG_OFF_MASK					(0xfff << BACKUP_IMG_OFF_BIT)
#define get_backup_img_flag(val)				(((val) & BACKUP_IMG_FLG_MASK) >> BACKUP_IMG_FLG_BIT)

/*
 * AHB_IOCFG
 */
#define REG_AHB_IO_CFG_BASE					0x102f0000

#define UART0_RXD_IOCFG_OFST					0x124
#define UART0_TXD_IOCFG_OFST					0x128

/* SFC_IOCFG */
#define REG_HP_IO_CFG_BASE					0x10230000

/* EMMC_IOCFG */
#define IO_CFG_EMMC_CLK						0x00
#define IO_CFG_EMMC_CMD						0x04
#define IO_CFG_EMMC_DATA0					0x08
#define IO_CFG_EMMC_DATA1					0x0C
#define IO_CFG_EMMC_DATA2					0x10
#define IO_CFG_EMMC_DATA3					0x14
#define IO_CFG_EMMC_DATA4					0x18
#define IO_CFG_EMMC_DATA5					0x1C
#define IO_CFG_EMMC_DATA6					0x20
#define IO_CFG_EMMC_DATA7					0x24
#define IO_CFG_EMMC_DS						0x28
#define IO_CFG_EMMC_RST						0x2C

#define REG_CRG_BASE						REG_BASE_CRG
#define REG_BASE_EMMC						0x10020000

#define PERI_CRG_EMMC						0x34c0
#define  MMC_RST						(0x1 << 16)
#define  MMC_CLK_EN						(0x1 << 0)
#define  MMC_AHB_CLK_EN						(0x1 << 1)
#define  MMC_CLK_SEL_MASK					(0x7 << 24)
#define  MMC_CLK_SEL_SHIFT					24
#define  MMC_CLK_400K						0x0
#define  MMC_CLK_25M						0x1
#define  MMC_CLK_50M						0x2
#define PERI_EMMC_DRV_DLL					0x34c8
#define  MMC_DRV_PHASE_MASK					(0x1f << 15)

#define  READ_DATA_BY_CPU					0x01
#define  READ_DATA_BY_DMA					0x02

/* emmc sdio macro */
#define mmc_clk_sel_val(reg_val)				((reg_val) & 0x7)
#define sdio_pad_pu(reg_val)            			(((reg_val) >> 0) & 0x1)

/*-----------------------------------------------------------------------------------
 * rng_gen register
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_TRNG              	 			0x10114000
#define SEC_COM_TRNG_FIFO_DATA   	 			(REG_BASE_TRNG + 0x204)
#define SEC_COM_TRNG_DATA_ST      				(REG_BASE_TRNG + 0x208)

#define SECURE_BOOT_OVER_TIME       				200	/* 200 ms */

/*-----------------------------------------------------------------------------------
 * usb register
 *-----------------------------------------------------------------------------------*/
#define USB_P0_REG_BASE						0x10320000

/*-----------------------------------------------------------------------------------
 * otp user interface register
 *-----------------------------------------------------------------------------------*/
#define SCPU_OTPC_BASE_ADDR             			0x101E0000
#define OTP_SHADOW_BASE                	 			SCPU_OTPC_BASE_ADDR

/*-----------------------------------------------------------------------------------
 * otp read only register
 *-----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 * dbc_apb register
 *-----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 * boot sel type
 *-----------------------------------------------------------------------------------*/
#define BOOT_SEL_PCIE    					0x965a4b87
#define BOOT_SEL_UART    					0x69a5b478
#define BOOT_SEL_USB     					0x965ab487
#define BOOT_SEL_SDIO   					0x69a54b87
#define BOOT_SEL_FLASH   					0x96a54b78
#define BOOT_SEL_EMMC   			 		0x695ab487
#define BOOT_SEL_UNKNOW  					0x965a4b78

/* SDIO_IOCFG */
#define REG_VDP_AIAO_IO_CFG_BASE        			0x102f0000
#define SDIO0_CCLK_OUT_OFST             			0x9c
#define SDIO0_CARD_DETECT_OFST          			0x80
#define SDIO0_CARD_POWEN_EN_OFST        			0x84
#define SDIO0_DATA0_OFST                			0x8c
#define SDIO0_DATA1_OFST                			0x90
#define SDIO0_DATA2_OFST                			0x94
#define SDIO0_DATA3_OFST                			0x98
#define SDIO0_CCMD_OFST                 			0x88

#define MMC_DRV_PHASE_SHIFT             			15
#define REG_BASE_SDIO0                  			0x10030000

#define PERI_CRG_SDIO0                  			0x35c0
#define PERI_SDIO0_DRV_DLL              			0x35c8
#define PERI_CRG_SDIO0_CLK              			0x210

/*-----------------------------------------------------------------------------------
 *  TEE Image
 *-----------------------------------------------------------------------------------*/
#define DDR_BASE						0x40000000
#define DDR_SIZE						0x200000000
#define DDR_END 						(DDR_BASE + DDR_SIZE)

/*-----------------------------------------------------------------------------------
 * Secure OS
 *-----------------------------------------------------------------------------------*/
#define SEC_BOOTRAM_SEC_CFG 					(REG_SYSCTRL_BASE + 0x5028)
#define SEC_BOOTRAM_RO_CFG 					(REG_SYSCTRL_BASE + 0x502c)
#define SEC_BOOTRAM_SEC_CFG_LOCK1 				(REG_SYSCTRL_BASE + 0x5030)
#define SEC_BOOTRAM_RO_CFG_LOCK2 				(REG_SYSCTRL_BASE + 0x5034)

#define FDT_BASE						DDR_BASE
#define FDT_SIZE						0xF000		/* 60KB */
#define SEC_DDR_BASE						(FDT_BASE + FDT_SIZE)
#define SECONDARY_CORE_CODE_ADDR				SEC_DDR_BASE
#define SECONDARY_CORE_CODE_SIZE				0x1000		/* 4KB */
#define BL31_BASE						(SECONDARY_CORE_CODE_ADDR + SECONDARY_CORE_CODE_SIZE)
#define BL31_SIZE						0x20000		/* 128KB */
#define BL32_BASE						(BL31_BASE + BL31_SIZE)
#define BL32_SIZE						0x1000000	/* 16MB */
#define BL32_LOAD_ADDR						(BL32_BASE - 0x1C)
#define TEE_OS_KEY_ADDR						(BL32_BASE + BL32_SIZE)
#define TEE_OS_KEY_SIZE						0x1000

/*-----------------------------------------------------------------------------------
 * TZASC
 *-----------------------------------------------------------------------------------*/
#define SEC_MODULE_BASE						0x11141000
#define TZASC_BYPASS_REG_OFFSET					0x004
#define TZASC_RGN_MAP_REG_OFFSET				0x100
#define TZASC_RGN_MAP_EXT_REG_OFFSET				0x200
#define TZASC_RGN_ATTR_REG					0x104

#define TZASC_MAX_RGN_NUMS 					16
#define TZASC_RGN_1   						1
#define TZASC_RGN_2   						2
#define TZASC_RGN_8   						8

#define tzasc_rgn_offset(rgn)					(0x10 * (rgn))

/*-----------------------------------------------------------------------------------
 *  SVB
 *-----------------------------------------------------------------------------------*/
#define SVB_VER_REG						0x11020168
#define SVB_VER							0x30303030
#define SYSCTRL_REG						0x11020000
#define OTP_SHPM_MDA_OFFSET					0x153c

#define HPM_CORE_VOL_REG					(SYSCTRL_REG + 0x9004)
#define HPM_MDA_VOL_REG						(SYSCTRL_REG + 0x9104)
#define HPM_NPU_VOL_REG						(SYSCTRL_REG + 0x9204)
#define HPM_MONITOR_CFG						0x60200001
#define HPM_CORE_OFFSET						0xb030
#define HPM_MDA_OFFSET						0xb020
#define HPM_NPU_OFFSET						0xb010

#define CYCLE_NUM						8
#define HPM_NPU_REG0						0xb018
#define HPM_NPU_REG1						0xb01c
#define HPM_MDA_REG0						0xb028
#define HPM_MDA_REG1						0xb02c
#define HPM_CORE_REG0						0xb038
#define HPM_CORE_REG1						0xb03c

#define HPM_CLK_REG						0x11014a80
#define HPM_CLK_CFG						0x30

#define CPU_ISO_REG						0x1d821104

#define REG_TSENSOR_CTRL					0x1102e000
#define TSENSOR_CTRL0						0x0
#define TSENSOR_CTRL0_CFG					0xc0300000
#define TSENSOR_CTRL1						0x4
#define TSENSOR_CTRL1_CFG					0x8
#define TSENSOR_CTRL						0x70
#define TSENSOR_STATUS0						0x8

#define OTP_CPU_IF_REG						0x10120000
#define OTP_HPM_CORE_OFFSET 		 			0x1504
#define OTP_HPM_MDA_OFFSET  	 				0x1534
#define OTP_HPM_NPU_OFFSET   					0x1530

#define HPM_CORE_STORAGE_REG 					0x340
#define HPM_MDA_STORAGE_REG  					0x344
#define HPM_NPU_STORAGE_REG  					0x348
#define TEMPERATURE_STORAGE_REG  				0x34c
#define DELTA_V_STORAGE_REG 					0x350
#define CORE_DUTY_STORAGE_REG  					0x354
#define MDA_DUTY_STORAGE_REG  					0x358
#define NPU_DUTY_STORAGE_REG  					0x35c

/* physical max/min */
#define CORE_VOLT_MAX						920
#define CORE_VOLT_MIN						599

#define MDA_VOLT_MAX						1049
#define MDA_VOLT_MIN						600

#define NPU_VOLT_MAX						1049
#define NPU_VOLT_MIN						600

/* curve max/min; voltage curve:  v = (b - a * hpm) / 10 */
#define CORE_CURVE_VLOT_MAX					850
#define CORE_CURVE_VLOT_MIN					760
#define CORE_CURVE_B						16200
#define CORE_CURVE_A						20

#define MEDIA_CURVE_VLOT_MAX					770
#define MEDIA_CURVE_VLOT_MIN					690
#define MEDIA_CURVE_B						11150
#define MEDIA_CURVE_A						10


#define NPU_CURVE_VLOT_MAX					810
#define NPU_CURVE_VLOT_MIN					730
#define NPU_CURVE_B						127500
#define NPU_CURVE_A						125

#define DDR_SCRAMB_ENABLE
#ifdef DDR_SCRAMB_ENABLE
/*-----------------------------------------------------------------------------------
 *  TRNG
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_RNG_GEN					0x101EE000
#define TRNG_DSTA_FIFO_DATA_REG					(REG_BASE_RNG_GEN + 0x100)
#define TRNG_DATA_ST_REG					(REG_BASE_RNG_GEN + 0x108)
#define TRNG_FIFO_DATA_CNT_MASK					0x1f

/*-----------------------------------------------------------------------------------
 *  DDRC
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_DDRC						0x11140000

#define DMC0_DDRC_CTRL_SREF_REG					(REG_BASE_DDRC + 0x8000 + 0x0)
#define DMC1_DDRC_CTRL_SREF_REG					(REG_BASE_DDRC + 0x9000 + 0x0)

#define DMC0_DDRC_CFG_DDRMODE_REG				(REG_BASE_DDRC + 0x8000 + 0x50)
#define DMC1_DDRC_CFG_DDRMODE_REG				(REG_BASE_DDRC + 0x9000 + 0x50)

#define DMC0_DDRC_CURR_FUNC_REG					(REG_BASE_DDRC + 0x8000 + 0x294)
#define DMC1_DDRC_CURR_FUNC_REG					(REG_BASE_DDRC + 0x9000 + 0x294)

#define DDRC_SREF_DONE						(0x1 << 1)
#define DDRC_SREF_REQ						(0x1 << 0)

/*-----------------------------------------------------------------------------------
 *  MISC DDRCA
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_MISC						0x11020000
#define DDRCA_REE_RANDOM_L_REG					(REG_BASE_MISC + 0x4218)
#define DDRCA_REE_RANDOM_H_REG					(REG_BASE_MISC + 0x421C)
#define DDRCA_EN_REG						(REG_BASE_MISC + 0x4220)
#define DDRCA_REE_UPDATE_REG					(REG_BASE_MISC + 0x4224)
#define DDRCA_LOCK_REG						(REG_BASE_MISC + 0x4228)
#define SCRAMB_BYPASS_DIS					0x5
#define SCRAMB_BYPASS					        0xa
#define SCRAMB_ENABLE						0x1
#define SCRAMB_LOCK						0x1
#endif


/* MPIDR definitions */
#define MPIDR_AFFINITY_BITS					8
#define MPIDR_AFFLVL_MASK					0xff
#define MPIDR_CLUSTER_MASK					(MPIDR_AFFLVL_MASK << MPIDR_AFFINITY_BITS)

/* p channel control */
#define CPU_CTRL6						0x4128
#define CPU_CTRL7						0x422C
#define CPU_CTRL8						0x4230
#define CPU_CTRL9						0x4234
#define MISC_REG_CPU_CTRL6					(REG_SYSCTRL_BASE + CPU_CTRL6)
#define MISC_REG_CPU_CTRL7					(REG_SYSCTRL_BASE + CPU_CTRL7)
#define MISC_REG_CPU_CTRL8					(REG_SYSCTRL_BASE + CPU_CTRL8)
#define MISC_REG_CPU_CTRL9					(REG_SYSCTRL_BASE + CPU_CTRL9)

#define CPU_HW_STATE_MACHINE					(1 << 6)
#define CPU_PSTATE_MASK						((1 << 6) - 1)
#define CPU_CORE_POWER_DOWN					(1 << 16)

#define CORTEX_A55_CPUPWRCTLR_EL1      				S3_0_C15_C2_7
#define CORTEX_A55_CORE_PWRDN_EN_MASK				0x1

#define CURRENT_EL_MASK						0x03
#define CURRENT_EL_SHIFT					0x02

/* */
#define CORE_1							1
#define CORE_2							2
#define CORE_3							3

#define ENTRY_POINT_OFFSET					2
#define REG_PERI_OFFSET						4


#ifndef REG_PERI_CPU_RVBARADDR
#define REG_PERI_CPU_RVBARADDR					0x11024204
#endif

#ifndef REG_PERI_CRG2066_CORE0
#define REG_PERI_CRG2066_CORE0					0x11012048
#endif

#ifndef REG_PERI_CRG2067_CORE1
#define REG_PERI_CRG2067_CORE1					0x1101204c
#endif

#ifndef REG_PERI_CRG2068_CORE2
#define REG_PERI_CRG2068_CORE2					0x11012050
#endif

#ifndef REG_PERI_CRG2069_CORE3
#define REG_PERI_CRG2069_CORE3					0x11012054
#endif


/*-----------------------------------------------------------------------------------
 * ipc register
 *-----------------------------------------------------------------------------------*/
#define IPC_REG_BASE						0x1103E000U
#define IPC_SEC_REG_BASE					0x1103F000U
#define IPC_SET_REG						(IPC_REG_BASE + 0x000)
#define IPC_CLEAR_REG						(IPC_REG_BASE + 0x004)
#define IPC_STATUS_REG						(IPC_REG_BASE + 0x008)
#define IPC_INT_MASK_REG					(IPC_REG_BASE + 0x00C)
#define IPC_SHARE_REG_BASE					(IPC_REG_BASE + 0x020)
#define IPC_SHARE_MAX_REG					16
#define IPC_INT_MAX						9

#define IPC_CMD_ACK						0xA0
#define IPC_CMD_START_BL31					0xA1
#define IPC_CMD_START_TEEIMG					0xA2
#define IPC_CMD_NEED_BL31					0xB1
#define IPC_CMD_NEED_TEEIMG					0xB2

#define IPC_NODE_CORE0						0
#define IPC_NODE_CORE1						1

/* security hardening */
#define	REDUN_CHECK_CNT						2

#define FW_ADDR_OFFSET						32

#define SYSBOOT14                                               0x11020168
#define BOOT_REG_VERSION                                        0x0101
#include "serial_reg.h"
#endif  /* __PLATFORM_VENDORLICON_H__ */
