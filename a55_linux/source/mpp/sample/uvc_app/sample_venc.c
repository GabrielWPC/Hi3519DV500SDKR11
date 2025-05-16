/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "sample_venc.h"
#include <stdio.h>
#include "sample_comm.h"
#include "ot_stream.h"
#include "uvc_media.h"

/* Stream Control Operation Functions End */
static struct processing_unit_ops g_venc_pu_ops = {
    .brightness_get = venc_brightness_get,
    .contrast_get = venc_contrast_get,
    .hue_get = venc_hue_get,
    .power_line_frequency_get = venc_power_line_frequency_get,
    .saturation_get = venc_saturation_get,
    .white_balance_temperature_auto_get = venc_white_balance_temperature_auto_get,
    .white_balance_temperature_get = venc_white_balance_temperature_get,

    .brightness_set = venc_brightness_set,
    .contrast_set = venc_contrast_set,
    .hue_set = venc_hue_set,
    .power_line_frequency_set = venc_power_line_frequency_set,
    .saturation_set = venc_saturation_set,
    .white_balance_temperature_auto_set = venc_white_balance_temperature_auto_set,
    .white_balance_temperature_set = venc_white_balance_temperature_set,
};

static struct input_terminal_ops g_venc_it_ops = {
    .exposure_ansolute_time_get = venc_exposure_ansolute_time_get,
    .exposure_auto_mode_get = venc_exposure_auto_mode_get,
    .exposure_ansolute_time_set = venc_exposure_ansolute_time_set,
    .exposure_auto_mode_set = venc_exposure_auto_mode_set,
};

static struct stream_control_ops g_venc_sc_ops = {
    .init = sample_venc_init,
    .deinit = sample_venc_deinit,
    .startup = sample_venc_startup,
    .shutdown = sample_venc_shutdown,
    .set_idr = sample_venc_set_idr,
    .set_property = sample_venc_set_property,
    .get_send = sample_venc_get_uvc_send,
};

void sample_venc_config(void)
{
    printf("\n@@@@@ UVC App Sample @@@@@\n\n");

    ot_stream_register_mpi_ops(&g_venc_sc_ops, &g_venc_pu_ops, &g_venc_it_ops, TD_NULL);
}
