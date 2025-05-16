/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: motionsensor_chip_proc.c
 * Author: ISP SW
 * Create: 2018-7-30
 * Version: Initial Draft
 */

#include "motionsensor_chip_proc.h"
#include <linux/kernel.h>
#include "ot_osal.h"
#include "sys_ext.h"
#include "motionsensor.h"
#ifdef IMU_ICM20690
#include "icm20690.h"
#elif defined IMU_ICM42670
#include "icm42670.h"
#endif

#define  MSENSOR_CHIP_INFO "motionsensor_chip"
#define  MSENSOR_CHIP_VERSION_INFO "motionsensor_chip debug 0.0.0.1"

static td_char *msensor_triger_mode2_str(msensor_triger_mode mode)
{
    switch (mode) {
        case TRIGER_TIMER:
            return "TIMER";
        case TRIGER_EXTERN_INTERRUPT:
            return "EXTERN_INTERRUPT";
        default:
            return "error";
    }
    return "0";
}

static td_s32 motionsensor_chip_proc_show(osal_proc_entry *s)
{
    ot_msensor_param *msensor_param = msensor_chip_get_param();
    imu_dev_info *dev_info = chip_get_dev_info();

    osal_seq_printf(s->seqfile,
        "[motionsensor] version:[" MSENSOR_CHIP_VERSION_INFO "], build time["__DATE__", "__TIME__"]\n");

    call_sys_print_proc_title(s, "common parameter");
    osal_seq_printf(s->seqfile, "%24s %24s\n", "trigle_mode", "fifo_en");
    osal_seq_printf(s->seqfile, "%24s %24d\n",
        msensor_triger_mode2_str(dev_info->triger_data.triger_mode),
        dev_info->fifo_en);

    if (msensor_param->attr.device_mask & OT_MSENSOR_DEVICE_GYRO) {
        call_sys_print_proc_title(s, "gyro parameter");
        osal_seq_printf(s->seqfile, "%24s\n", IMU_DEV_NAME);

        osal_seq_printf(s->seqfile, "%24s %24s %24s %24s %24s\n",
            "sample_rate", "full-scale-range", "datawidth", "max-chip-temperature", "min-chip-temperature");
        osal_seq_printf(s->seqfile, "%24d %24d %24d %24d %24d\n",
            msensor_param->config.gyro_config.odr,
            msensor_param->config.gyro_config.fsr,
            msensor_param->config.gyro_config.data_width,
            msensor_param->config.gyro_config.temperature_max,
            msensor_param->config.gyro_config.temperature_min);
    }
    if (msensor_param->attr.device_mask & OT_MSENSOR_DEVICE_ACC) {
        call_sys_print_proc_title(s, "accelerometer parameter");
        osal_seq_printf(s->seqfile, "%24s\n", IMU_DEV_NAME);

        osal_seq_printf(s->seqfile, "%24s %24s %24s %24s %24s\n",
            "sample_rate", "full-scale-range", "datawidth", "max-chip-temperature", "min-chip-temperature");
        osal_seq_printf(s->seqfile, "%24d %24d %24d %24d %24d\n",
            msensor_param->config.acc_config.odr,
            msensor_param->config.acc_config.fsr,
            msensor_param->config.acc_config.data_width,
            msensor_param->config.acc_config.temperature_max,
            msensor_param->config.acc_config.temperature_min);
    }
    return 0;
}

td_s32 mpu_proc_init(td_void)
{
    osal_proc_entry *msensor_chip_entry = TD_NULL;

    msensor_chip_entry = osal_create_proc_entry(MSENSOR_CHIP_INFO, TD_NULL);
    if (msensor_chip_entry == TD_NULL) {
        printk("osal_create_proc_entry failed!\n");
        return TD_FAILURE;
    }

    msensor_chip_entry->read = motionsensor_chip_proc_show;
    msensor_chip_entry->write = TD_NULL;
    return TD_SUCCESS;
}

void  mpu_proc_exit(td_void)
{
    osal_remove_proc_entry(MSENSOR_CHIP_INFO, 0);
    return;
}
