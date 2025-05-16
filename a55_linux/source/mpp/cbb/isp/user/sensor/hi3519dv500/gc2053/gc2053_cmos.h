/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef gc2053_CMOS_H
#define gc2053_CMOS_H

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

#define gc2053_I2C_ADDR    0x6e
#define gc2053_ADDR_BYTE   1
#define gc2053_DATA_BYTE   1
#define gc2053_sensor_get_ctx(pipe, ctx)   ((ctx) = gc2053_get_ctx(pipe))

#define gc2053_FULL_LINES_MAX 0x3FFF

#define gc2053_INCREASE_LINES  0  /* make real fps less than stand fps because NVR require */
#define gc2053_VMAX_2M_LINEAR    (1125 + 0)

typedef enum {
    gc2053_SENSOR_2M_30FPS_LINEAR_MODE = 0,
    gc2053_MODE_BUTT
} gc2053_res_mode;

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
} gc2053_video_mode_tbl;

ot_isp_sns_state *gc2053_get_ctx(ot_vi_pipe vi_pipe);
ot_isp_sns_commbus *gc2053_get_bus_info(ot_vi_pipe vi_pipe);

td_void gc2053_init(ot_vi_pipe vi_pipe);
td_void gc2053_exit(ot_vi_pipe vi_pipe);
td_void gc2053_standby(ot_vi_pipe vi_pipe);
td_void gc2053_restart(ot_vi_pipe vi_pipe);
td_s32  gc2053_write_register(ot_vi_pipe vi_pipe, td_u32 addr, td_u32 data);
td_s32  gc2053_read_register(ot_vi_pipe vi_pipe, td_u32 addr);
td_void gc2053_blc_clamp(ot_vi_pipe vi_pipe, ot_isp_sns_blc_clamp blc_clamp);
td_void gc2053_set_blc_clamp_value(ot_vi_pipe vi_pipe, td_bool clamp_en);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif /* gc2053_CMOS_H */

