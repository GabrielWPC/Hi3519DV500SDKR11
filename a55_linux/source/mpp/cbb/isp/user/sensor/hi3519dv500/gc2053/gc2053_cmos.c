/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include "ot_mpi_isp.h"
#include "ot_mpi_ae.h"
#include "ot_mpi_awb.h"
#include "securec.h"

#include "gc2053_cmos_ex.h"
#include "gc2053_cmos.h"

#define gc2053_ID                    2053
#define SENSOR_gc2053_WIDTH          1920
#define SENSOR_gc2053_HEIGHT         1080

#define higher_4bits(x) (((x) & 0xf0000) >> 16)
#define high_8bits(x) (((x) & 0xff00) >> 8)
#define low_8bits(x)  ((x) & 0x00ff)


#ifndef max
#define max(a, b) (((a) < (b)) ?  (b) : (a))
#endif

#ifndef min
#define min(a, b) (((a) > (b)) ?  (b) : (a))
#endif

/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/
#define gc2053_sensor_set_ctx(pipe, ctx)   ((g_gc2053_sns_state[pipe]) = (ctx))
#define gc2053_sensor_reset_ctx(pipe)      (g_gc2053_sns_state[pipe] = TD_NULL)

static ot_isp_fswdr_mode g_fswdr_mode[OT_ISP_MAX_PIPE_NUM] = {
    [0 ... OT_ISP_MAX_PIPE_NUM - 1] = OT_ISP_FSWDR_NORMAL_MODE
};
static td_u8 g_ae_stat_pos[OT_ISP_MAX_PIPE_NUM] = {0};
static td_u32 g_max_time_get_cnt[OT_ISP_MAX_PIPE_NUM] = {0};
static td_u32 g_init_exposure[OT_ISP_MAX_PIPE_NUM]  = {0};
static td_u32 g_lines_per500ms[OT_ISP_MAX_PIPE_NUM] = {0};

static td_u16 g_init_wb_gain[OT_ISP_MAX_PIPE_NUM][OT_ISP_RGB_CHN_NUM] = {{0}};
static td_u16 g_sample_r_gain[OT_ISP_MAX_PIPE_NUM] = {0};
static td_u16 g_sample_b_gain[OT_ISP_MAX_PIPE_NUM] = {0};
static td_bool g_quick_start_en[OT_ISP_MAX_PIPE_NUM] = {TD_FALSE};

static td_bool g_ae_route_ex_valid[OT_ISP_MAX_PIPE_NUM] = {0};
static ot_isp_ae_route g_init_ae_route[OT_ISP_MAX_PIPE_NUM] = {{0}};
static ot_isp_ae_route_ex g_init_ae_route_ex[OT_ISP_MAX_PIPE_NUM] = {{0}};
static ot_isp_ae_route g_init_ae_route_sf[OT_ISP_MAX_PIPE_NUM] = {{0}};
static ot_isp_ae_route_ex g_init_ae_route_sf_ex[OT_ISP_MAX_PIPE_NUM] = {{0}};

typedef struct {
    td_u8       hcg;
    td_u32      brl;
    td_u32      rhs1_max;
    td_u32      rhs2_max;
} gc2053_state;

gc2053_state g_gc2053_state[OT_ISP_MAX_PIPE_NUM] = {{0}};
ot_isp_sns_commbus g_gc2053_bus_info[OT_ISP_MAX_PIPE_NUM] = {
    [0] = { .i2c_dev = 0},
    [1 ... OT_ISP_MAX_PIPE_NUM - 1] = { .i2c_dev = -1}
};

ot_isp_sns_state *g_gc2053_sns_state[OT_ISP_MAX_PIPE_NUM] = {TD_NULL};

static td_bool blc_clamp_info[OT_ISP_MAX_PIPE_NUM] = {[0 ...(OT_ISP_MAX_PIPE_NUM - 1)] = TD_TRUE};
ot_isp_sns_commbus *gc2053_get_bus_info(ot_vi_pipe vi_pipe)
{
    return &g_gc2053_bus_info[vi_pipe];
}

ot_isp_sns_state *gc2053_get_ctx(ot_vi_pipe vi_pipe)
{
    return g_gc2053_sns_state[vi_pipe];
}

td_void gc2053_set_blc_clamp_value(ot_vi_pipe vi_pipe, td_bool clamp_en)
{
    blc_clamp_info[vi_pipe] = clamp_en;
}

const gc2053_video_mode_tbl g_gc2053_mode_tbl[gc2053_MODE_BUTT] = {
    {gc2053_VMAX_2M_LINEAR,   gc2053_FULL_LINES_MAX,          30, 0.8,
     1920, 1080, 0, OT_WDR_MODE_NONE,       "gc2053_SENSOR_2M_30FPS_LINEAR_MODE"},
};

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/
/* gc2053 Register Address */
#define GC2053_EXPTIME_ADDR_H                 0x03 /* Shutter-time[13:8] */
#define GC2053_EXPTIME_ADDR_L                 0x04 /* Shutter-time[7:0] */

#define GC2053_AGAIN_ADDR_H                   0xb4   /* Analog-gain[9:8] */
#define GC2053_AGAIN_ADDR_L                   0xb3   /* Analog-gain[7:0] */

#define GC2053_DGAIN_ADDR_H                   0xb9   /* digital-gain[11:6] */
#define GC2053_DGAIN_ADDR_L                   0xb8   /* digital-gain[5:0] */

#define GC2053_AUTO_PREGAIN_ADDR_H            0xb1   /* auto-pregain-gain[9:6] */
#define GC2053_AUTO_PREGAIN_ADDR_L            0xb2   /* auto-pregain-gain[5:0] */

#define GC2053_VMAX_ADDR_H                    0x41   /* Vmax[13:8] */
#define GC2053_VMAX_ADDR_L                    0x42    /* Vmax[7:0] */
#define GC2053_STATUS_ADDR                    0x90    /* slow shutter via framerate or exptime */

// sensor gain
#define gc2053_AGAIN_MIN    1024
#define gc2053_AGAIN_MAX    113168

#define gc2053_DGAIN_MIN    64
#define gc2053_DGAIN_MAX    512

#define gc2053_AD_GAIN_TBL_RANGE  241
#define gc2053_AGAIN_TBL_RANGE  91
#define gc2053_DGAIN_TBL_RANGE  141

#define gc2053_EXP_TIME_LMT    2


#define gc2053_err_mode_print(sns_image_mode, sns_state) \
    do { \
        isp_err_trace("Not support! Width:%u, Height:%u, Fps:%f, WDRMode:%d\n", \
                      (sns_image_mode)->width,  \
                      (sns_image_mode)->height, \
                      (sns_image_mode)->fps,    \
                      (sns_state)->wdr_mode); \
    } while (0)

static td_void cmos_get_ae_comm_default(ot_vi_pipe vi_pipe, ot_isp_ae_sensor_default *ae_sns_dft,
    const ot_isp_sns_state *sns_state)
{
    ae_sns_dft->full_lines_std = sns_state->fl_std;
    ae_sns_dft->flicker_freq = 50 * 256; /* light flicker freq: 50Hz, accuracy: 256 */
    ae_sns_dft->full_lines_max = gc2053_FULL_LINES_MAX;
    ae_sns_dft->hmax_times = (1000000000) / (sns_state->fl_std * 30); /* 1000000000ns, 30fps */

    ae_sns_dft->int_time_accu.accu_type = OT_ISP_AE_ACCURACY_LINEAR;
    ae_sns_dft->int_time_accu.accuracy = 1;
    ae_sns_dft->int_time_accu.offset = 0;

    ae_sns_dft->again_accu.accu_type = OT_ISP_AE_ACCURACY_TABLE;
    ae_sns_dft->again_accu.accuracy  = 1;

    ae_sns_dft->dgain_accu.accu_type = OT_ISP_AE_ACCURACY_LINEAR;
    ae_sns_dft->dgain_accu.accuracy = 0.015625;

    ae_sns_dft->isp_dgain_shift = 8; /* accuracy: 8 */
    ae_sns_dft->min_isp_dgain_target = 1 << ae_sns_dft->isp_dgain_shift;
    ae_sns_dft->max_isp_dgain_target = 2 << ae_sns_dft->isp_dgain_shift; /* max 2 */
    if (g_lines_per500ms[vi_pipe] == 0) {
        ae_sns_dft->lines_per500ms = sns_state->fl_std * 30 / 2; /* 30fps, div 2 */
    } else {
        ae_sns_dft->lines_per500ms = g_lines_per500ms[vi_pipe];
    }

    ae_sns_dft->max_iris_fno = OT_ISP_IRIS_F_NO_1_0;
    ae_sns_dft->min_iris_fno = OT_ISP_IRIS_F_NO_32_0;

    ae_sns_dft->ae_route_ex_valid = TD_FALSE;
    ae_sns_dft->ae_route_attr.total_num = 0;
    ae_sns_dft->ae_route_attr_ex.total_num = 0;
    ae_sns_dft->quick_start.quick_start_enable = g_quick_start_en[vi_pipe];
    ae_sns_dft->quick_start.black_frame_num = 0;
    ae_sns_dft->ae_stat_pos = g_ae_stat_pos[vi_pipe]; /* 1 use be stat to AE */
    return;
}

static td_void cmos_get_ae_linear_default(ot_vi_pipe vi_pipe, ot_isp_ae_sensor_default *ae_sns_dft,
    const ot_isp_sns_state *sns_state)
{
    ae_sns_dft->max_again = 51200;//113168
    ae_sns_dft->min_again = 1024;
    ae_sns_dft->max_again_target = ae_sns_dft->max_again;
    ae_sns_dft->min_again_target = ae_sns_dft->min_again;

    ae_sns_dft->max_dgain = 64;//512
    ae_sns_dft->min_dgain = 64;
    ae_sns_dft->max_dgain_target = ae_sns_dft->max_dgain;
    ae_sns_dft->min_dgain_target = ae_sns_dft->min_dgain;

    ae_sns_dft->ae_compensation = 0x38;
    ae_sns_dft->ae_exp_mode = OT_ISP_AE_EXP_HIGHLIGHT_PRIOR;
    ae_sns_dft->init_exposure = g_init_exposure[vi_pipe] ? g_init_exposure[vi_pipe] : 148859; /* init 76151 */

    ae_sns_dft->max_int_time = sns_state->fl_std - 2; /* sub 2 */
    ae_sns_dft->min_int_time = 1; /* min 9 */
    ae_sns_dft->max_int_time_target = 65535; /* max 65535 */
    ae_sns_dft->min_int_time_target = 1; /* min 9 */
    ae_sns_dft->ae_route_ex_valid = g_ae_route_ex_valid[vi_pipe];
    (td_void)memcpy_s(&ae_sns_dft->ae_route_attr, sizeof(ot_isp_ae_route),
                      &g_init_ae_route[vi_pipe],  sizeof(ot_isp_ae_route));
    (td_void)memcpy_s(&ae_sns_dft->ae_route_attr_ex, sizeof(ot_isp_ae_route_ex),
                      &g_init_ae_route_ex[vi_pipe], sizeof(ot_isp_ae_route_ex));
    return;
}

static td_s32 cmos_get_ae_default(ot_vi_pipe vi_pipe, ot_isp_ae_sensor_default *ae_sns_dft)
{
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_check_pointer_return(ae_sns_dft);
    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_return(sns_state);

    (td_void)memset_s(&ae_sns_dft->ae_route_attr, sizeof(ot_isp_ae_route), 0, sizeof(ot_isp_ae_route));

    cmos_get_ae_comm_default(vi_pipe, ae_sns_dft, sns_state);

    switch (sns_state->wdr_mode) {
        case OT_WDR_MODE_NONE:   /* linear mode */
            cmos_get_ae_linear_default(vi_pipe, ae_sns_dft, sns_state);
            break;
        default:
            break;
    }

    return TD_SUCCESS;
}

static td_void cmos_config_vmax(ot_isp_sns_state *sns_state, td_u32 vmax)
{
    if (sns_state->wdr_mode == OT_WDR_MODE_NONE) {
        sns_state->regs_info[0].i2c_data[8].data = 0x0; /* array index 5 */
        sns_state->regs_info[0].i2c_data[9].data = (vmax & 0xFF); /* array index 6 */
        sns_state->regs_info[0].i2c_data[10].data =((vmax & 0xFF00) >> 8); /* array index 7 */
    }
    return;
}

/* the function of sensor set fps */
static td_void cmos_fps_set(ot_vi_pipe vi_pipe, td_float fps, ot_isp_ae_sensor_default *ae_sns_dft)
{
    td_u32 lines, lines_max, vmax;
    td_float max_fps, min_fps;
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_check_pointer_void_return(ae_sns_dft);
    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_void_return(sns_state);

    lines = g_gc2053_mode_tbl[sns_state->img_mode].ver_lines;
    lines_max = g_gc2053_mode_tbl[sns_state->img_mode].max_ver_lines;
    max_fps = g_gc2053_mode_tbl[sns_state->img_mode].max_fps;
    min_fps = g_gc2053_mode_tbl[sns_state->img_mode].min_fps;

    if ((fps > max_fps) || (fps < min_fps)) {
        isp_err_trace("Not support Fps: %f\n", fps);
        return;
    }

    vmax = (td_u32)(lines * max_fps / div_0_to_1_float(fps));
    vmax = (vmax > lines_max) ? lines_max : vmax;
    cmos_config_vmax(sns_state, vmax);

    sns_state->fl_std = vmax;
    ae_sns_dft->lines_per500ms = (td_u32)(lines * max_fps / 2); /* div 2 */

    ae_sns_dft->fps = fps;
    ae_sns_dft->full_lines_std = sns_state->fl_std;
    ae_sns_dft->max_int_time = sns_state->fl_std - gc2053_EXP_TIME_LMT;
    sns_state->fl[0] = sns_state->fl_std;
    ae_sns_dft->full_lines = sns_state->fl[0];
    ae_sns_dft->hmax_times =
        (td_u32)((1000000000) / (sns_state->fl_std * div_0_to_1_float(fps))); /* 1000000000ns */

    return;
}

static td_void cmos_slow_framerate_set(ot_vi_pipe vi_pipe, td_u32 full_lines, ot_isp_ae_sensor_default *ae_sns_dft)
{
    td_u32 lines_max;
    td_u32 vmax;
    ot_isp_sns_state *sns_state = TD_NULL;
    sns_check_pointer_void_return(ae_sns_dft);
    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_void_return(sns_state);

    lines_max = g_gc2053_mode_tbl[sns_state->img_mode].max_ver_lines;

    if (sns_state->wdr_mode == OT_WDR_MODE_NONE) {
        vmax = full_lines;
        vmax = (vmax > lines_max) ? lines_max : vmax;
        sns_state->fl[0] = vmax;
    }

    switch (sns_state->wdr_mode) {
        case OT_WDR_MODE_NONE:
            sns_state->regs_info[0].i2c_data[8].data = low_8bits(vmax); /* index 5 */
            sns_state->regs_info[0].i2c_data[9].data = high_8bits(vmax); /* index 6 */
            sns_state->regs_info[0].i2c_data[10].data = higher_4bits(vmax); /* index 7 */
            break;
        default:
            break;
    }

    ae_sns_dft->full_lines = sns_state->fl[0];
    ae_sns_dft->max_int_time = sns_state->fl[0] - gc2053_EXP_TIME_LMT;

    return;
}

static td_void cmos_inttime_update_linear(ot_vi_pipe vi_pipe, td_u32 int_time)
{
    ot_isp_sns_state *sns_state = TD_NULL;
    td_u32 value;

    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_void_return(sns_state);
	//printf("int_time =%d\n",int_time);
    value = (int_time > gc2053_FULL_LINES_MAX) ? gc2053_FULL_LINES_MAX : int_time;
    //value = min(value, gc2053_FULL_LINES_MAX);
    //value = min(max(value, 8), (sns_state->fl[0] - 2));

    sns_state->regs_info[0].i2c_data[0].data = (value & 0xFF);
    sns_state->regs_info[0].i2c_data[1].data = ((value & 0xFF00) >> 8); /* index 1 */
    return ;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static td_void cmos_inttime_update(ot_vi_pipe vi_pipe, td_u32 int_time)
{
    ot_isp_sns_state *sns_state = TD_NULL;
    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_void_return(sns_state);

    if (sns_state->wdr_mode == OT_WDR_MODE_NONE) {
        cmos_inttime_update_linear(vi_pipe, int_time);
    }
    return;
}
static td_u32 regValTable[29][6] = {
    {0x00, 0x00, 0x01, 0x00, 0x82, 0x90}, /*0xb4 0xb3 0xb8 0xb9 [0x38 0x7c] for FSWDR */
    {0x00, 0x10, 0x01, 0x0c, 0x82, 0x90},
    {0x00, 0x20, 0x01, 0x1b, 0x82, 0x90},
    {0x00, 0x30, 0x01, 0x2c, 0x82, 0x90},
    {0x00, 0x40, 0x01, 0x3f, 0x82, 0x90},
    {0x00, 0x50, 0x02, 0x16, 0x88, 0x93},
    {0x00, 0x60, 0x02, 0x35, 0x88, 0x93},
    {0x00, 0x70, 0x03, 0x16, 0x88, 0x93},
    {0x00, 0x80, 0x04, 0x02, 0x88, 0x93},
    {0x00, 0x90, 0x04, 0x31, 0x88, 0x93},
    {0x00, 0xa0, 0x05, 0x32, 0x88, 0x93},
    {0x00, 0xb0, 0x06, 0x35, 0x88, 0x93},
    {0x00, 0xc0, 0x08, 0x04, 0x88, 0x93},
    {0x00, 0x5a, 0x09, 0x19, 0x88, 0x93},
    {0x00, 0x83, 0x0b, 0x0f, 0x88, 0x93},
    {0x00, 0x93, 0x0d, 0x12, 0x88, 0x93},
    {0x00, 0x84, 0x10, 0x00, 0x88, 0x93},
    {0x00, 0x94, 0x12, 0x3a, 0x88, 0x93},
    {0x01, 0x2c, 0x1a, 0x02, 0x88, 0x93},
    {0x01, 0x3c, 0x1b, 0x20, 0x88, 0x93},
    {0x00, 0x8c, 0x20, 0x0f, 0x88, 0x93},
    {0x00, 0x9c, 0x26, 0x07, 0x88, 0x93},
    {0x02, 0x64, 0x36, 0x21, 0x88, 0x93},
    {0x02, 0x74, 0x37, 0x3a, 0x88, 0x93},
    {0x00, 0xc6, 0x3d, 0x02, 0x88, 0x93},
    {0x00, 0xdc, 0x3f, 0x3f, 0x88, 0x93},
    {0x02, 0x85, 0x3f, 0x3f, 0x88, 0x93},
    {0x02, 0x95, 0x3f, 0x3f, 0x88, 0x93},
    {0x00, 0xce, 0x3f, 0x3f, 0x88, 0x93},
};

static td_u32 analog_gain_table[29] = {
    1024, 1230, 1440, 1730, 2032, 2380, 2880, 3460, 4080, 4800, 5776,
    6760, 8064, 9500, 11552, 13600, 16132, 18912, 22528, 27036, 32340,
    38256, 45600, 53912, 63768, 76880, 92300, 108904, 123568,
};


static td_void cmos_again_calc_table(ot_vi_pipe vi_pipe, td_u32 *again_lin, td_u32 *again_db)
{
    sns_check_pointer_void_return(again_lin);
    sns_check_pointer_void_return(again_db);
    ot_unused(vi_pipe);
	static td_u8 againmax = 28; /* adgain max 28 */
    if (*again_lin >= analog_gain_table[againmax]) {
        *again_lin = analog_gain_table[28];
        *again_db = againmax;
        return;
    } else {
        int i;
        for (i = 1; i < (29 + 1); i++) {
            if (*again_lin < analog_gain_table[i]) {
                *again_lin = analog_gain_table[i - 1];
                *again_db = i - 1;
                break;
            }
        }
    }
    return;
}
/*
static td_void cmos_dgain_calc_table(ot_vi_pipe vi_pipe, td_u32 *dgain_lin, td_u32 *dgain_db)
{
    int i;
    sns_check_pointer_void_return(dgain_lin);
    sns_check_pointer_void_return(dgain_db);
    ot_unused(vi_pipe);

    if (*dgain_lin >= g_gain_table[gc2053_DGAIN_TBL_RANGE]) {
        *dgain_lin = g_gain_table[gc2053_DGAIN_TBL_RANGE];
        *dgain_db = gc2053_DGAIN_TBL_RANGE;
        return;
    }

    for (i = 1; i < (gc2053_DGAIN_TBL_RANGE + 1); i++) {
        if (*dgain_lin < g_gain_table[i]) {
            *dgain_lin = g_gain_table[i - 1];
            *dgain_db = i - 1;
            break;
        }
    }
    return;
}*/

static td_void cmos_gains_update(ot_vi_pipe vi_pipe, td_u32 again, td_u32 dgain)
{
    ot_isp_sns_state *sns_state = TD_NULL;
    td_u32 tmp;
	td_u8 u8DgainHigh, u8DgainLow;
    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_void_return(sns_state);
	td_u32 hcg =g_gc2053_state[vi_pipe].hcg;
	u8DgainHigh = (dgain >> 6) & 0x0f; /* shift 6 */
    u8DgainLow = (dgain & 0x3f) << 2; /* shift 2 */
    sns_state->regs_info[0].i2c_data[2].data = regValTable[again][0];
	sns_state->regs_info[0].i2c_data[3].data = regValTable[again][1]; /* index 3 */
    sns_state->regs_info[0].i2c_data[4].data = regValTable[again][2]; /* index 4, index 2 */
   	sns_state->regs_info[0].i2c_data[5].data = regValTable[again][3]; /* index 5, index 3 */
   	sns_state->regs_info[0].i2c_data[7].data =  u8DgainLow; /* index 7 */
   	sns_state->regs_info[0].i2c_data[6].data = u8DgainHigh; /* index 6 */
    return;
}

static td_void cmos_get_inttime_max(ot_vi_pipe vi_pipe, td_u16 man_ratio_enable, td_u32 *ratio,
    ot_isp_ae_int_time_range *int_time, td_u32 *lf_max_int_time)
{
    ot_isp_sns_state *sns_state = TD_NULL;
    sns_check_pointer_void_return(ratio);
    sns_check_pointer_void_return(int_time);
    sns_check_pointer_void_return(lf_max_int_time);
    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_void_return(sns_state);

    ot_unused(man_ratio_enable);

    return;
}

/* Only used in LINE_WDR mode */
static td_void cmos_ae_fswdr_attr_set(ot_vi_pipe vi_pipe, ot_isp_ae_fswdr_attr *ae_fswdr_attr)
{
    sns_check_pointer_void_return(ae_fswdr_attr);

    g_fswdr_mode[vi_pipe] = ae_fswdr_attr->fswdr_mode;
    g_max_time_get_cnt[vi_pipe] = 0;
    return;
}

static td_s32 cmos_init_ae_exp_function(ot_isp_ae_sensor_exp_func *exp_func)
{
    sns_check_pointer_return(exp_func);

    (td_void)memset_s(exp_func, sizeof(ot_isp_ae_sensor_exp_func), 0, sizeof(ot_isp_ae_sensor_exp_func));

    exp_func->pfn_cmos_get_ae_default    = cmos_get_ae_default;
    exp_func->pfn_cmos_fps_set           = cmos_fps_set;
    exp_func->pfn_cmos_slow_framerate_set = cmos_slow_framerate_set;
    exp_func->pfn_cmos_inttime_update    = cmos_inttime_update;
    exp_func->pfn_cmos_gains_update      = cmos_gains_update;
    exp_func->pfn_cmos_again_calc_table  = cmos_again_calc_table;
    exp_func->pfn_cmos_dgain_calc_table  = TD_NULL;
    exp_func->pfn_cmos_get_inttime_max   = cmos_get_inttime_max;
    exp_func->pfn_cmos_ae_fswdr_attr_set = cmos_ae_fswdr_attr_set;

    return TD_SUCCESS;
}

/* awb static param */
#define CALIBRATE_STATIC_TEMP                         4908
#define CALIBRATE_STATIC_WB_R_GAIN                    388
#define CALIBRATE_STATIC_WB_GR_GAIN                   256
#define CALIBRATE_STATIC_WB_GB_GAIN                   256
#define CALIBRATE_STATIC_WB_B_GAIN                    519

/* Calibration results for Auto WB Planck */
#define CALIBRATE_AWB_P1                              39
#define CALIBRATE_AWB_P2                              152
#define CALIBRATE_AWB_Q1                              (-65)
#define CALIBRATE_AWB_A1                              201612
#define CALIBRATE_AWB_B1                              128
#define CALIBRATE_AWB_C1                              (-147924)




/* Rgain and Bgain of the golden sample */
#define GOLDEN_RGAIN                                  0
#define GOLDEN_BGAIN                                  0
static td_s32 cmos_get_awb_default(ot_vi_pipe vi_pipe, ot_isp_awb_sensor_default *awb_sns_dft)
{
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_check_pointer_return(awb_sns_dft);
    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_return(sns_state);

    (td_void)memset_s(awb_sns_dft, sizeof(ot_isp_awb_sensor_default), 0, sizeof(ot_isp_awb_sensor_default));
    awb_sns_dft->wb_ref_temp = 5082; /* wb_ref_temp 4950 */

    awb_sns_dft->gain_offset[0] = 374;
    awb_sns_dft->gain_offset[1] = 256;
    awb_sns_dft->gain_offset[2] = 256; /* index 2 */
    awb_sns_dft->gain_offset[3] = 627; /* index 3 */

    awb_sns_dft->wb_para[0] = 110;
    awb_sns_dft->wb_para[1] = 11;
    awb_sns_dft->wb_para[2] = -135; /* index 2 */
    awb_sns_dft->wb_para[3] = 224807; /* index 3 */
    awb_sns_dft->wb_para[4] = CALIBRATE_AWB_B1; /* index 4 */
    awb_sns_dft->wb_para[5] = -163011; /* index 5 */

    awb_sns_dft->golden_rgain = GOLDEN_RGAIN;
    awb_sns_dft->golden_bgain = GOLDEN_BGAIN;

    switch (sns_state->wdr_mode) {
        case OT_WDR_MODE_NONE:
            (td_void)memcpy_s(&awb_sns_dft->ccm, sizeof(ot_isp_awb_ccm), &g_awb_ccm, sizeof(ot_isp_awb_ccm));
            (td_void)memcpy_s(&awb_sns_dft->agc_tbl, sizeof(ot_isp_awb_agc_table),
                              &g_awb_agc_table, sizeof(ot_isp_awb_agc_table));
            break;
        default:
            break;
    }

    awb_sns_dft->init_rgain = g_init_wb_gain[vi_pipe][0]; /* 0: Rgain */
    awb_sns_dft->init_ggain = g_init_wb_gain[vi_pipe][1]; /* 1: Ggain */
    awb_sns_dft->init_bgain = g_init_wb_gain[vi_pipe][2]; /* 2: Bgain */
    awb_sns_dft->sample_rgain = g_sample_r_gain[vi_pipe];
    awb_sns_dft->sample_bgain = g_sample_b_gain[vi_pipe];

    return TD_SUCCESS;
}

static td_s32 cmos_init_awb_exp_function(ot_isp_awb_sensor_exp_func *exp_func)
{
    sns_check_pointer_return(exp_func);

    (td_void)memset_s(exp_func, sizeof(ot_isp_awb_sensor_exp_func), 0, sizeof(ot_isp_awb_sensor_exp_func));

    exp_func->pfn_cmos_get_awb_default = cmos_get_awb_default;

    return TD_SUCCESS;
}

static ot_isp_cmos_dng_color_param g_dng_color_param = {{ 378, 256, 430 }, { 439, 256, 439 }};

static td_void cmos_get_isp_dng_default(const ot_isp_sns_state *sns_state, ot_isp_cmos_default *isp_def)
{
    (td_void)memcpy_s(&isp_def->dng_color_param, sizeof(ot_isp_cmos_dng_color_param), &g_dng_color_param,
                      sizeof(ot_isp_cmos_dng_color_param));

    switch (sns_state->img_mode) {
        case gc2053_SENSOR_2M_30FPS_LINEAR_MODE:
            isp_def->sns_mode.dng_raw_format.bits_per_sample = 12; /* 12bit */
            isp_def->sns_mode.dng_raw_format.white_level = 4095; /* max 4095 */
            break;
        default:
            break;
    }

    isp_def->sns_mode.dng_raw_format.default_scale.default_scale_hor.denominator = 1;
    isp_def->sns_mode.dng_raw_format.default_scale.default_scale_hor.numerator = 1;
    isp_def->sns_mode.dng_raw_format.default_scale.default_scale_ver.denominator = 1;
    isp_def->sns_mode.dng_raw_format.default_scale.default_scale_ver.numerator = 1;
    isp_def->sns_mode.dng_raw_format.cfa_repeat_pattern_dim.repeat_pattern_dim_row = 2; /* pattern 2 */
    isp_def->sns_mode.dng_raw_format.cfa_repeat_pattern_dim.repeat_pattern_dim_col = 2; /* pattern 2 */
    isp_def->sns_mode.dng_raw_format.black_level_repeat_dim.repeat_row = 2; /* pattern 2 */
    isp_def->sns_mode.dng_raw_format.black_level_repeat_dim.repeat_col = 2; /* pattern 2 */
    isp_def->sns_mode.dng_raw_format.cfa_layout = OT_ISP_CFALAYOUT_TYPE_RECTANGULAR;
    isp_def->sns_mode.dng_raw_format.cfa_plane_color[0] = 0;
    isp_def->sns_mode.dng_raw_format.cfa_plane_color[1] = 1;
    isp_def->sns_mode.dng_raw_format.cfa_plane_color[2] = 2; /* index 2, cfa_plane_color 2 */
    isp_def->sns_mode.dng_raw_format.cfa_pattern[0] = 0;
    isp_def->sns_mode.dng_raw_format.cfa_pattern[1] = 1;
    isp_def->sns_mode.dng_raw_format.cfa_pattern[2] = 1; /* index 2, cfa_pattern 1 */
    isp_def->sns_mode.dng_raw_format.cfa_pattern[3] = 2; /* index 3, cfa_pattern 2 */
    isp_def->sns_mode.valid_dng_raw_format = TD_TRUE;
    return;
}

static void cmos_get_isp_linear_default(ot_isp_cmos_default *isp_def)
{
    isp_def->key.bit1_demosaic         = 1;
    isp_def->demosaic                  = &g_cmos_demosaic;
    isp_def->key.bit1_sharpen          = 1;
    isp_def->sharpen                   = &g_cmos_yuv_sharpen;
    isp_def->key.bit1_drc              = 1;
    isp_def->drc                       = &g_cmos_drc;
    isp_def->key.bit1_bayer_nr         = 1;
    isp_def->bayer_nr                  = &g_cmos_bayer_nr;
    isp_def->key.bit1_anti_false_color = 1;
    isp_def->anti_false_color          = &g_cmos_anti_false_color;
    isp_def->key.bit1_ldci             = 1;
    isp_def->ldci                      = &g_cmos_ldci;
    isp_def->key.bit1_gamma            = 1;
    isp_def->gamma                     = &g_cmos_gamma;
#ifdef CONFIG_OT_ISP_CR_SUPPORT
    isp_def->key.bit1_ge               = 1;
    isp_def->ge                        = &g_cmos_ge;
#endif
    isp_def->key.bit1_dehaze = 1;
    isp_def->dehaze = &g_cmos_dehaze;
    (td_void)memcpy_s(&isp_def->noise_calibration, sizeof(ot_isp_noise_calibration),
                      &g_cmos_noise_calibration, sizeof(ot_isp_noise_calibration));
    return;
}

static td_s32 cmos_get_isp_default(ot_vi_pipe vi_pipe, ot_isp_cmos_default *isp_def)
{
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_check_pointer_return(isp_def);
    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_return(sns_state);

    (td_void)memset_s(isp_def, sizeof(ot_isp_cmos_default), 0, sizeof(ot_isp_cmos_default));
#ifdef CONFIG_OT_ISP_CA_SUPPORT

    isp_def->key.bit1_ca      = 1;
    isp_def->ca               = &g_cmos_ca;
#endif
    isp_def->key.bit1_clut    = 1;
    isp_def->clut             = &g_cmos_clut;

    isp_def->key.bit1_dpc     = 1;
    isp_def->dpc              = &g_cmos_dpc;

    isp_def->key.bit1_wdr     = 1;
    isp_def->wdr              = &g_cmos_wdr;

    isp_def->key.bit1_lsc      = 0;
    isp_def->lsc               = &g_cmos_lsc;

    isp_def->key.bit1_acs      = 0;
    isp_def->acs               = &g_cmos_acs;

#ifdef CONFIG_OT_ISP_PREGAMMA_SUPPORT
    isp_def->key.bit1_pregamma = 0;
    isp_def->pregamma          = &g_cmos_pregamma;
#endif
    switch (sns_state->wdr_mode) {
        case OT_WDR_MODE_NONE:
            cmos_get_isp_linear_default(isp_def);
            break;
        default:
            break;
    }

    isp_def->wdr_switch_attr.exp_ratio[0] = 0x40;
    isp_def->sns_mode.sns_id= gc2053_ID;
    isp_def->sns_mode.sns_mode = sns_state->img_mode;
    cmos_get_isp_dng_default(sns_state, isp_def);

    return TD_SUCCESS;
}

static td_s32 cmos_get_isp_black_level(ot_vi_pipe vi_pipe, ot_isp_cmos_black_level *black_level)
{
    td_s32  i;
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_check_pointer_return(black_level);
    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_return(sns_state);

    (td_void)memcpy_s(black_level, sizeof(ot_isp_cmos_black_level), &g_cmos_blc, sizeof(ot_isp_cmos_black_level));

    /* Don't need to update black level when iso change */
    black_level->auto_attr.update = TD_FALSE;

    /* black level of linear mode */
    if (sns_state->wdr_mode == OT_WDR_MODE_NONE) {
        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            black_level->auto_attr.black_level[0][i] = 63;
        }
    }

    return TD_SUCCESS;
}

static td_s32 cmos_get_isp_blc_clamp_info(ot_vi_pipe vi_pipe, td_bool *blc_clamp_en)
{
    sns_check_pointer_return(blc_clamp_en);

    *blc_clamp_en = blc_clamp_info[vi_pipe];

    return TD_SUCCESS;
}

static td_void cmos_set_pixel_detect(ot_vi_pipe vi_pipe, td_bool enable)
{
    td_u32 full_lines_5fps, max_int_time_5fps;
    ot_isp_sns_state *sns_state = TD_NULL;

    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_void_return(sns_state);



    return;
}

static td_s32 cmos_set_wdr_mode(ot_vi_pipe vi_pipe, td_u8 mode)
{
    ot_isp_sns_state *sns_state = TD_NULL;

    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_return(sns_state);

    sns_state->sync_init    = TD_FALSE;

    switch (mode & 0x3F) {
        case OT_WDR_MODE_NONE:
            sns_state->wdr_mode = OT_WDR_MODE_NONE;
            printf("linear mode\n");
            break;

        default:
            isp_err_trace("NOT support this mode!\n");
            return TD_FAILURE;
    }

    (td_void)memset_s(sns_state->wdr_int_time, sizeof(sns_state->wdr_int_time), 0, sizeof(sns_state->wdr_int_time));

    return TD_SUCCESS;
}

static td_void cmos_comm_sns_reg_info_init(ot_vi_pipe vi_pipe, ot_isp_sns_state *sns_state)
{
    td_u32 i;
    sns_state->regs_info[0].sns_type = OT_ISP_SNS_TYPE_I2C;
    sns_state->regs_info[0].com_bus.i2c_dev = g_gc2053_bus_info[vi_pipe].i2c_dev;
    sns_state->regs_info[0].cfg2_valid_delay_max = 3; /* delay_max 2 */
    sns_state->regs_info[0].reg_num = 11; /* reg_num 8 */

    for (i = 0; i < sns_state->regs_info[0].reg_num; i++) {
        sns_state->regs_info[0].i2c_data[i].update = TD_TRUE;
        sns_state->regs_info[0].i2c_data[i].dev_addr = gc2053_I2C_ADDR;
        sns_state->regs_info[0].i2c_data[i].addr_byte_num = gc2053_ADDR_BYTE;
        sns_state->regs_info[0].i2c_data[i].data_byte_num = gc2053_DATA_BYTE;
    }

    /* shutter related */
    sns_state->regs_info[0].i2c_data[0].delay_frame_num = 1;
    sns_state->regs_info[0].i2c_data[0].reg_addr = GC2053_EXPTIME_ADDR_L;
    sns_state->regs_info[0].i2c_data[1].delay_frame_num = 1;
    sns_state->regs_info[0].i2c_data[1].reg_addr = GC2053_EXPTIME_ADDR_H;
	
    sns_state->regs_info[0].i2c_data[2].delay_frame_num = 1; /* index 2 */
    sns_state->regs_info[0].i2c_data[2].reg_addr = GC2053_AGAIN_ADDR_H; /* index 2 */
    sns_state->regs_info[0].i2c_data[3].delay_frame_num = 1; /* index 3 */
    sns_state->regs_info[0].i2c_data[3].reg_addr = GC2053_AGAIN_ADDR_L; /* index 3 */
	
    sns_state->regs_info[0].i2c_data[4].delay_frame_num = 1;  /* index 4 */
    sns_state->regs_info[0].i2c_data[4].reg_addr = GC2053_DGAIN_ADDR_L; /* index 4 */
    sns_state->regs_info[0].i2c_data[5].delay_frame_num = 1; /* index 5 */
    sns_state->regs_info[0].i2c_data[5].reg_addr = GC2053_DGAIN_ADDR_H; /* index 5 */
	
    sns_state->regs_info[0].i2c_data[7].delay_frame_num = 1; /* index 7 */
    sns_state->regs_info[0].i2c_data[7].reg_addr = GC2053_AUTO_PREGAIN_ADDR_L; /* index 7, index 2 */	
    sns_state->regs_info[0].i2c_data[6].delay_frame_num = 1; /* index 6 */
    sns_state->regs_info[0].i2c_data[6].reg_addr = GC2053_AUTO_PREGAIN_ADDR_H; /* index 6 */


	sns_state->regs_info[0].i2c_data[8].delay_frame_num = 0; /* index 7 */
    sns_state->regs_info[0].i2c_data[8].reg_addr = GC2053_STATUS_ADDR; /* index 7, index 2 */
	sns_state->regs_info[0].i2c_data[9].delay_frame_num = 0; /* index 7 */
    sns_state->regs_info[0].i2c_data[9].reg_addr = GC2053_VMAX_ADDR_L; /* index 7, index 2 */
	sns_state->regs_info[0].i2c_data[10].delay_frame_num = 0; /* index 7 */
    sns_state->regs_info[0].i2c_data[10].reg_addr = GC2053_VMAX_ADDR_H; /* index 7, index 2 */
    return;
}


static td_void cmos_sns_reg_info_update(ot_vi_pipe vi_pipe, ot_isp_sns_state *sns_state)
{
    td_u32 i;
    ot_unused(vi_pipe);

    for (i = 0; i < sns_state->regs_info[0].reg_num; i++) {
        if (sns_state->regs_info[0].i2c_data[i].data ==
            sns_state->regs_info[1].i2c_data[i].data) {
            sns_state->regs_info[0].i2c_data[i].update = TD_FALSE;
        } else {
            sns_state->regs_info[0].i2c_data[i].update = TD_TRUE;
        }
    }

    return;
}

static td_s32 cmos_get_sns_regs_info(ot_vi_pipe vi_pipe, ot_isp_sns_regs_info *sns_regs_info)
{
    ot_isp_sns_state *sns_state = TD_NULL;

    sns_check_pointer_return(sns_regs_info);
    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_return(sns_state);

    if ((sns_state->sync_init == TD_FALSE) || (sns_regs_info->config == TD_FALSE)) {
        cmos_comm_sns_reg_info_init(vi_pipe, sns_state);
        sns_state->sync_init = TD_TRUE;
    } else {
        cmos_sns_reg_info_update(vi_pipe, sns_state);
    }

    sns_regs_info->config = TD_FALSE;
    (td_void)memcpy_s(sns_regs_info, sizeof(ot_isp_sns_regs_info),
                      &sns_state->regs_info[0], sizeof(ot_isp_sns_regs_info));
    (td_void)memcpy_s(&sns_state->regs_info[1], sizeof(ot_isp_sns_regs_info),
                      &sns_state->regs_info[0], sizeof(ot_isp_sns_regs_info));
    sns_state->fl[1] = sns_state->fl[0];

    return TD_SUCCESS;
}

static td_void cmos_config_image_mode_param(ot_vi_pipe vi_pipe, td_u8 sns_image_mode,
    ot_isp_sns_state *sns_state)
{
    switch (sns_image_mode) {
        case gc2053_SENSOR_2M_30FPS_LINEAR_MODE:
            sns_state->fl_std         = gc2053_VMAX_2M_LINEAR;
            break;
        default:
            break;
    }
}

static td_s32 cmos_set_image_mode(ot_vi_pipe vi_pipe, const ot_isp_cmos_sns_image_mode *sns_image_mode)
{
    td_u32 i;
    td_u8 image_mode;
    ot_isp_sns_state *sns_state = TD_NULL;
    sns_check_pointer_return(sns_image_mode);
    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_return(sns_state);

    image_mode = sns_state->img_mode;

    for (i = 0; i < gc2053_MODE_BUTT; i++) {
        if (sns_image_mode->fps <= g_gc2053_mode_tbl[i].max_fps &&
            sns_image_mode->width <= g_gc2053_mode_tbl[i].width &&
            sns_image_mode->height <= g_gc2053_mode_tbl[i].height &&
            sns_state->wdr_mode == g_gc2053_mode_tbl[i].wdr_mode) {
            image_mode = (gc2053_res_mode)i;
            break;
        }
    }
	g_gc2053_state[vi_pipe].hcg = 0x2;
    if (i >= gc2053_MODE_BUTT) {
        gc2053_err_mode_print(sns_image_mode, sns_state);
        return TD_FAILURE;
    }

    cmos_config_image_mode_param(vi_pipe, image_mode, sns_state);

    if ((sns_state->init == TD_TRUE) && (image_mode == sns_state->img_mode)) {
        return OT_ISP_DO_NOT_NEED_SWITCH_IMAGEMODE; /* Don't need to switch image_mode */
    }

    sns_state->sync_init = TD_FALSE;
    sns_state->img_mode = image_mode;
    sns_state->fl[0] = sns_state->fl_std;
    sns_state->fl[1] = sns_state->fl[0];
    return TD_SUCCESS;
}

static td_void sensor_global_init(ot_vi_pipe vi_pipe)
{
    ot_isp_sns_state *sns_state = TD_NULL;

    gc2053_sensor_get_ctx(vi_pipe, sns_state);
    sns_check_pointer_void_return(sns_state);

    sns_state->init = TD_FALSE;
    sns_state->sync_init = TD_FALSE;
    sns_state->img_mode = gc2053_SENSOR_2M_30FPS_LINEAR_MODE;
    sns_state->wdr_mode = OT_WDR_MODE_NONE;
    sns_state->fl_std  = gc2053_VMAX_2M_LINEAR;
    sns_state->fl[0] = gc2053_VMAX_2M_LINEAR;
    sns_state->fl[1] = gc2053_VMAX_2M_LINEAR;

    (td_void)memset_s(&sns_state->regs_info[0], sizeof(ot_isp_sns_regs_info), 0, sizeof(ot_isp_sns_regs_info));
    (td_void)memset_s(&sns_state->regs_info[1], sizeof(ot_isp_sns_regs_info), 0, sizeof(ot_isp_sns_regs_info));
    return;
}

static td_s32 cmos_init_sensor_exp_function(ot_isp_sns_exp_func *sensor_exp_func)
{
    sns_check_pointer_return(sensor_exp_func);

    (td_void)memset_s(sensor_exp_func, sizeof(ot_isp_sns_exp_func), 0, sizeof(ot_isp_sns_exp_func));

    sensor_exp_func->pfn_cmos_sns_init         = gc2053_init;
    sensor_exp_func->pfn_cmos_sns_exit         = gc2053_exit;
    sensor_exp_func->pfn_cmos_sns_global_init  = sensor_global_init;
    sensor_exp_func->pfn_cmos_set_image_mode      = cmos_set_image_mode;
    sensor_exp_func->pfn_cmos_set_wdr_mode        = cmos_set_wdr_mode;
    sensor_exp_func->pfn_cmos_get_isp_default     = cmos_get_isp_default;
    sensor_exp_func->pfn_cmos_get_isp_black_level = cmos_get_isp_black_level;
    sensor_exp_func->pfn_cmos_get_blc_clamp_info  = cmos_get_isp_blc_clamp_info;
    sensor_exp_func->pfn_cmos_set_pixel_detect    = cmos_set_pixel_detect;
    sensor_exp_func->pfn_cmos_get_sns_reg_info    = cmos_get_sns_regs_info;

    return TD_SUCCESS;
}

static td_s32 gc2053_set_bus_info(ot_vi_pipe vi_pipe, ot_isp_sns_commbus sns_bus_info)
{
    g_gc2053_bus_info[vi_pipe].i2c_dev = sns_bus_info.i2c_dev;

    return TD_SUCCESS;
}

static td_s32 sensor_ctx_init(ot_vi_pipe vi_pipe)
{
    ot_isp_sns_state *sns_state_ctx = TD_NULL;

    gc2053_sensor_get_ctx(vi_pipe, sns_state_ctx);

    if (sns_state_ctx == TD_NULL) {
        sns_state_ctx = (ot_isp_sns_state *)malloc(sizeof(ot_isp_sns_state));
        if (sns_state_ctx == TD_NULL) {
            isp_err_trace("Isp[%d] SnsCtx malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }

    (td_void)memset_s(sns_state_ctx, sizeof(ot_isp_sns_state), 0, sizeof(ot_isp_sns_state));

    gc2053_sensor_set_ctx(vi_pipe, sns_state_ctx);

    return TD_SUCCESS;
}

static td_void sensor_ctx_exit(ot_vi_pipe vi_pipe)
{
    ot_isp_sns_state *sns_state_ctx = TD_NULL;

    gc2053_sensor_get_ctx(vi_pipe, sns_state_ctx);
    sns_free(sns_state_ctx);
    gc2053_sensor_reset_ctx(vi_pipe);
    return;
}

static td_s32 sensor_register_callback(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *ae_lib, ot_isp_3a_alg_lib *awb_lib)
{
    td_s32 ret;
    ot_isp_sns_register isp_register;
    ot_isp_ae_sensor_register ae_register;
    ot_isp_awb_sensor_register awb_register;
    ot_isp_sns_attr_info sns_attr_info;

    sns_check_pointer_return(ae_lib);
    sns_check_pointer_return(awb_lib);

    ret = sensor_ctx_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sns_attr_info.sns_id = gc2053_ID;
    ret = cmos_init_sensor_exp_function(&isp_register.sns_exp);
    if (ret != TD_SUCCESS) {
        isp_err_trace("cmos init exp function failed!\n");
        return TD_FAILURE;
    }
    ret = ot_mpi_isp_sensor_reg_callback(vi_pipe, &sns_attr_info, &isp_register);
    if (ret != TD_SUCCESS) {
        isp_err_trace("sensor register callback function failed!\n");
        return ret;
    }

    ret = cmos_init_ae_exp_function(&ae_register.sns_exp);
    if (ret != TD_SUCCESS) {
        isp_err_trace("cmos init ae exp function failed!\n");
        return TD_FAILURE;
    }
    ret = ot_mpi_ae_sensor_reg_callback(vi_pipe, ae_lib, &sns_attr_info, &ae_register);
    if (ret != TD_SUCCESS) {
        isp_err_trace("sensor register callback function to ae lib failed!\n");
        return ret;
    }

    ret = cmos_init_awb_exp_function(&awb_register.sns_exp);
    if (ret != TD_SUCCESS) {
        isp_err_trace("cmos init awb exp function failed!\n");
        return TD_FAILURE;
    }
    ret = ot_mpi_awb_sensor_reg_callback(vi_pipe, awb_lib, &sns_attr_info, &awb_register);
    if (ret != TD_SUCCESS) {
        isp_err_trace("sensor register callback function to awb lib failed!\n");
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 sensor_unregister_callback(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *ae_lib, ot_isp_3a_alg_lib *awb_lib)
{
    td_s32 ret;

    sns_check_pointer_return(ae_lib);
    sns_check_pointer_return(awb_lib);

    ret = ot_mpi_isp_sensor_unreg_callback(vi_pipe, gc2053_ID);
    if (ret != TD_SUCCESS) {
        isp_err_trace("sensor unregister callback function failed!\n");
        return ret;
    }

    ret = ot_mpi_ae_sensor_unreg_callback(vi_pipe, ae_lib, gc2053_ID);
    if (ret != TD_SUCCESS) {
        isp_err_trace("sensor unregister callback function to ae lib failed!\n");
        return ret;
    }

    ret = ot_mpi_awb_sensor_unreg_callback(vi_pipe, awb_lib, gc2053_ID);
    if (ret != TD_SUCCESS) {
        isp_err_trace("sensor unregister callback function to awb lib failed!\n");
        return ret;
    }

    sensor_ctx_exit(vi_pipe);
    return TD_SUCCESS;
}

static td_s32 sensor_set_init(ot_vi_pipe vi_pipe, ot_isp_init_attr *init_attr)
{
    sns_check_pointer_return(init_attr);

    g_init_exposure[vi_pipe]  = init_attr->exposure;
    g_lines_per500ms[vi_pipe] = init_attr->lines_per500ms;
    g_init_wb_gain[vi_pipe][0] = init_attr->wb_r_gain; /* 0: rgain */
    g_init_wb_gain[vi_pipe][1] = init_attr->wb_g_gain; /* 1: ggain */
    g_init_wb_gain[vi_pipe][2] = init_attr->wb_b_gain; /* 2: bgain */
    g_sample_r_gain[vi_pipe] = init_attr->sample_r_gain;
    g_sample_b_gain[vi_pipe] = init_attr->sample_b_gain;
    g_quick_start_en[vi_pipe] = init_attr->quick_start_en;
    g_ae_stat_pos[vi_pipe]       = init_attr->ae_stat_pos;
    g_ae_route_ex_valid[vi_pipe] = init_attr->ae_route_ex_valid;
    (td_void)memcpy_s(&g_init_ae_route[vi_pipe], sizeof(ot_isp_ae_route),
                      &init_attr->ae_route, sizeof(ot_isp_ae_route));
    (td_void)memcpy_s(&g_init_ae_route_ex[vi_pipe], sizeof(ot_isp_ae_route_ex),
                      &init_attr->ae_route_ex, sizeof(ot_isp_ae_route_ex));
    (td_void)memcpy_s(&g_init_ae_route_sf[vi_pipe], sizeof(ot_isp_ae_route),
                      &init_attr->ae_route_sf, sizeof(ot_isp_ae_route));
    (td_void)memcpy_s(&g_init_ae_route_sf_ex[vi_pipe], sizeof(ot_isp_ae_route_ex),
                      &init_attr->ae_route_sf_ex, sizeof(ot_isp_ae_route_ex));

    return TD_SUCCESS;
}

ot_isp_sns_obj g_sns_gc2053_obj = {
    .pfn_register_callback     = sensor_register_callback,
    .pfn_un_register_callback  = sensor_unregister_callback,
    .pfn_standby               = gc2053_standby,
    .pfn_restart               = gc2053_restart,
    .pfn_mirror_flip           = TD_NULL,
    .pfn_set_blc_clamp         = gc2053_blc_clamp,
    .pfn_write_reg             = gc2053_write_register,
    .pfn_read_reg              = gc2053_read_register,
    .pfn_set_bus_info          = gc2053_set_bus_info,
    .pfn_set_init              = sensor_set_init
};

