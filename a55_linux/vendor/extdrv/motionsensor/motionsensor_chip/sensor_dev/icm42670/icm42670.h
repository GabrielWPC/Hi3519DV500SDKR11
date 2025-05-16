/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: icm42670.h
 * Author: ISP SW
 * Create: 2023-2-23
 * Version: Initial Draft
 */

#ifndef ICM42670_H
#define ICM42670_H

#include "motionsensor.h"
#include "ot_osal.h"
#include "linux/workqueue.h"
#include "linux/hrtimer.h"

#ifdef __LITEOS__
#include "gpio.h"
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#define IMU_DEV_NAME      "##ICM42670##"
#define IMU_DEV_ADDR      0x69
#define I2C_DEV_NUM       0x1

#define ICM42670_VALID_DATA_BIT 16

#define div_0_to_1(a) (((a) == 0) ? 1 : (a))

#define TRUE_REGISTER_VALUE  0x01
#define RESET_OFFSET    0x04
#define CLKSET_VALUE    0x01

// MREGs
#define MREG1 0
#define MREG2 1
#define MREG3 2

#define ICM42670_FIFO_MAX_RECORD 140
#define ICM42670_FIFO_R_MAX_SIZE 1152

#define ROOMTEMP_OFFSET  (25 * (0x1 << 10))
#define GRADIENT_TEMP    (0x1 << 10)
#define TEMP_SENSITIVITY    128

/* icm42670 interrupt pin reg */
#define INT_GPIO_CHIP       7
#define INT_GPIO_OFFSET     2

#define TIME_RECORD_CNT     10
#define DATA_RECORD_CNT (TIME_RECORD_CNT)

typedef enum {
    GYRO_BAND_WIDTH_16HZ,
    GYRO_BAND_WIDTH_25HZ,
    GYRO_BAND_WIDTH_34HZ,
    GYRO_BAND_WIDTH_53HZ,
    GYRO_BAND_WIDTH_73HZ,
    GYRO_BAND_WIDTH_121HZ,
    GYRO_BAND_WIDTH_180HZ,
    GYRO_BAND_WIDTH_BUTT
} msensor_gyro_band_width;

typedef enum {
    ACCEL_BAND_WIDTH_16HZ,
    ACCEL_BAND_WIDTH_25HZ,
    ACCEL_BAND_WIDTH_34HZ,
    ACCEL_BAND_WIDTH_53HZ,
    ACCEL_BAND_WIDTH_73HZ,
    ACCEL_BAND_WIDTH_121HZ,
    ACCEL_BAND_WIDTH_180HZ,
    ACCEL_BAND_WIDTH_BUTT
} msensor_accel_band_width;

typedef enum {
    ICM_42670_GYRO_DATA_RATE_1600HZ = (1600 << 10),
    ICM_42670_GYRO_DATA_RATE_800HZ = (800 << 10),
    ICM_42670_GYRO_DATA_RATE_400HZ = (400 << 10),
    ICM_42670_GYRO_DATA_RATE_200HZ = (200 << 10),
    ICM_42670_GYRO_DATA_RATE_100HZ = (100 << 10),
    ICM_42670_GYRO_DATA_RATE_50HZ = (50 << 10),
    ICM_42670_GYRO_DATA_RATE_BUTT
} icm_42670_gyro_msensor_gyro_data_rate;

typedef enum {
    ICM_42670_ACC_DATA_RATE_1600HZ = (1600 << 10),
    ICM_42670_ACC_DATA_RATE_800HZ = (800 << 10),
    ICM_42670_ACC_DATA_RATE_400HZ = (400 << 10),
    ICM_42670_ACC_DATA_RATE_200HZ = (200 << 10),
    ICM_42670_ACC_DATA_RATE_100HZ = (100 << 10),
    ICM_42670_ACC_DATA_RATE_50HZ = (50 << 10),
    ICM_42670_ACC_DATA_RATE_BUTT
} icm_42670_acc_msensor_gyro_data_rate;


imu_dev_info *chip_get_dev_info(td_void);
td_s32 chip_fifo_data_reset(td_void);
td_s32 chip_get_triger_config(triger_config *triger_data);

td_s32 chip_dev_init(ot_msensor_param *msensor_param);
td_s32 chip_dev_mng_init(ot_msensor_param *msensor_param);
td_void chip_dev_deinit(ot_msensor_param *msensor_param);

td_s32 chip_sensor_init(td_void);
td_void chip_sensor_deinit(ot_msensor_param *msensor_param);
td_s32 chip_timer_run(td_void);
td_s32 chip_interrupt_run(td_void);
td_s32 chip_timer_stop(td_void);
td_s32 chip_interrupt_stop(td_void);
td_s32 chip_get_data_for_one_frm(td_void);

/*
 * otp_pwr_down
 * 0: to power up OTP for read/write operation.
 * 1: to power down OTP to save power.
 * This bit is automatically set to 1 when OTP copy operation is complete.
 */
#define OTP_CTRL7_OTP_PWR_DOWN_POS      0x01
#define OTP_CTRL7_OTP_PWR_DOWN_MASK     (0x01 << OTP_CTRL7_OTP_PWR_DOWN_POS)
/* ---------------------------------------------------------------------------
 * register MREG_FPGA
 * ---------------------------------------------------------------------------*/
/* ---------------------------------------------------------------------------
 * register ROM
 * ---------------------------------------------------------------------------*/
#define ACCEL_DATA_SIZE               6
#define GYRO_DATA_SIZE                6
#define TEMP_DATA_SIZE                2

#define FIFO_HEADER_SIZE              1
#define FIFO_ACCEL_DATA_SIZE          ACCEL_DATA_SIZE
#define FIFO_GYRO_DATA_SIZE           GYRO_DATA_SIZE
#define FIFO_TEMP_DATA_SIZE           1
#define FIFO_TS_FSYNC_SIZE            2
#define FIFO_TEMP_HIGH_RES_SIZE       1
#define FIFO_ACCEL_GYRO_HIGH_RES_SIZE 3

 /*
 * Hysteresis high peak threshold (mg) added to the threshold after the initial threshold is met.
 * Use type APEX_CONFIG5_HIGHG_PEAK_TH_HYST_t to define highG peak hysteresis
 * These types are defined in inv_imu_defs.h.
 */
#define HIGHG_PEAK_HYSTERESIS          APEX_CONFIG5_HIGHG_PEAK_TH_HYST_156MG

/* Initial WOM threshold to be applied to IMU in mg */
#define WOM_THRESHOLD_INITIAL_MG 200

/*
 * HighG frequencies
 * Use type APEX_CONFIG1_DMP_ODR_t to define DMP frequency
 * These types are defined in inv_imu_defs.h.
 *
 * \note The frequency modes to run the HighG are :
 * APEX_CONFIG1_DMP_ODR_25Hz  (Low Power mode),
 * APEX_CONFIG1_DMP_ODR_50Hz  (Performance mode)
 */
#define HIGHG_FREQUENCY_MODE APEX_CONFIG1_DMP_ODR_50Hz


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif

