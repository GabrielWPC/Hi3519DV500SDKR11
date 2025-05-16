/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef imx335_CMOS_H
#define imx335_CMOS_H

#include "ot_common.h"
#include "ot_common_isp.h"
#include "ot_common_video.h"
#include "ot_sns_ctrl.h"
#include "ot_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define imx335_I2C_ADDR    0x34
#define imx335_ADDR_BYTE   2
#define imx335_DATA_BYTE   1
#define imx335_sensor_get_ctx(pipe, ctx)   ((ctx) = imx335_get_ctx(pipe))

#define imx335_FULL_LINES_MAX 0xFFFFF

#define imx335_INCREASE_LINES  0  /* make real fps less than stand fps because NVR require */
#define imx335_VMAX_5M_LINEAR    (0x1194 + 0)

typedef enum {
    imx335_SENSOR_2M_30FPS_LINEAR_MODE = 0,
    imx335_MODE_BUTT
} imx335_res_mode;

typedef struct {
    td_u32      ver_lines;
    td_u32      max_ver_lines;
    td_float    max_fps;
    td_float    min_fps;
    td_u32      width;
    td_u32      height;
    td_u8       sns_mode;
    ot_wdr_mode wdr_mode;
    const char *mode_name;
} imx335_video_mode_tbl;

ot_isp_sns_state *imx335_get_ctx(ot_vi_pipe vi_pipe);
ot_isp_sns_commbus *imx335_get_bus_info(ot_vi_pipe vi_pipe);

td_void imx335_init(ot_vi_pipe vi_pipe);
td_void imx335_exit(ot_vi_pipe vi_pipe);
td_void imx335_standby(ot_vi_pipe vi_pipe);
td_void imx335_restart(ot_vi_pipe vi_pipe);
td_s32  imx335_write_register(ot_vi_pipe vi_pipe, td_u32 addr, td_u32 data);
td_s32  imx335_read_register(ot_vi_pipe vi_pipe, td_u32 addr);
td_void imx335_blc_clamp(ot_vi_pipe vi_pipe, ot_isp_sns_blc_clamp blc_clamp);
td_void imx335_set_blc_clamp_value(ot_vi_pipe vi_pipe, td_bool clamp_en);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif /* imx335_CMOS_H */

