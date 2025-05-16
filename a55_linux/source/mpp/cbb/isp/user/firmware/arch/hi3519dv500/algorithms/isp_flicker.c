/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_alg.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"

#define OT_ISP_FLICKER_GROUP            16
#define OT_ISP_FLICKER_MINBANDNUM       3
#define OT_ISP_FLICKER_MINVALIDBANDPCNT 6
#define OT_ISP_FLICKER_WAVEDIFF1        3
#define OT_ISP_FLICKER_WAVEDIFF2        4
#define OT_ISP_FLICKER_PERIOD           3
#define OT_ISP_FLICKER_GRCNT            5
#define OT_ISP_FLICKER_GBCNT            5
#define OT_ISP_FLICKER_OVERTH           50
#define OT_ISP_FLICKER_OVERCNTTH        520560
#define OT_ISP_FLICKER_MUTIPLE          2
#define OT_ISP_FLICKER_MINWIDTH         2

typedef struct {
    td_u8   min_band_num;
    td_u8   min_valid_band_pcnt;
    td_u8   wave_diff1;
    td_u8   wave_diff2;
    td_u8   period;
    td_u8   gr_cnt;
    td_u8   gb_cnt;
    td_s32  gb_avgcnt;
    td_s32  gr_avgcnt;
} isp_flicker_extern_reg;

typedef struct {
    /* alloc mem when initial */
    td_s32 *gr_mean;
    td_s32 *gb_mean;
    td_s32 *gr_cros0;
    td_s32 *gb_cros0;
    td_s32 *gr_cnt;
    td_s32 *gb_cnt;
} isp_flicker_point_attr;

typedef struct {
    td_s32 cur_frame_diff_sum_gr; /* s24.0, current frame gr sum */
    td_s32 cur_frame_diff_sum_gb; /* s24.0, current frame gb sum */

    td_u32 cur_frame_abs_sum_gr;  /* u23.0 */
    td_u32 cur_frame_abs_sum_gb;  /* u23.0 */
    td_u32 cur_frame_count_over; /* u26.0 */
    td_s32 *gr_mean;          /* s11.0 */
    td_s32 *gb_mean;          /* s11.0 */
} isp_flicker_stat_info;

typedef struct {
    td_bool enable;
    td_bool malloc;
    td_u16  fck_inx;
    td_u16  height;
    td_u16  over_thr;
    td_u32  over_cnt_thr;
    td_u32  frame_num;
    td_s32  pre_frame_avg_gr;  /* s15.0, previous frame gr avg */
    td_s32  pre_frame_avg_gb;  /* s15.0, previous frame gb avg */
    td_s32  cur_frame_avg_gr;  /* s15.0, previous frame gr avg */
    td_s32  cur_frame_avg_gb;  /* s15.0, previous frame gb avg */
    td_u32  cur_frame_var_gr;
    td_u32  cur_frame_var_gb;
    td_u32  cur_frame_count_over; /* u26.0 */
    td_s32  cur_flicker;        /* u1.0,  current frame flicker,1 is flicker,0 is no flicker */
    td_s32  ret_flick_result;    /* u1.0,  the result,1 is flicker,0 is no flicker */
    td_s32  cur_freq;           /* u1.0,  current frame flicker,1 is 50_hz,0 is 60_hz */
    td_s32  pre_freq;
    td_s32  ret_freq_result;
    td_s32  flicker_hist;
    td_s32  freq_hist;
    td_s32 *mem;

    isp_flicker_extern_reg fck_ext_reg;
    isp_flicker_point_attr   fck_point;
    isp_flicker_stat_info    fck_stat_info;
} isp_flicker_attr;

typedef struct {
    td_s32 grindx;
    td_s32 gbindx;
    td_s32 gr_avg;
    td_s32 gb_avg;
    td_s32 gr_pat;
    td_s32 gb_pat;
    td_s32 grflicker_thd;
    td_s32 gbflicker_thd;
    td_s32 gr_diff3cnt;
    td_s32 gb_diff3cnt;
    td_s32 gr_avgcnt;
    td_s32 gb_avgcnt;
} isp_flicker_avg;

isp_flicker_attr *g_flicker_ctx[OT_ISP_MAX_PIPE_NUM] = {TD_NULL};

#define flicker_get_ctx(pipe, ctx)   ((ctx) = g_flicker_ctx[pipe])
#define flicker_set_ctx(pipe, ctx)   (g_flicker_ctx[pipe] = (ctx))
#define flicker_reset_ctx(pipe)      (g_flicker_ctx[pipe] = TD_NULL)

static td_s32 flicker_ctx_init(ot_vi_pipe vi_pipe)
{
    isp_flicker_attr *flicker_ctx = TD_NULL;

    flicker_get_ctx(vi_pipe, flicker_ctx);

    if (flicker_ctx == TD_NULL) {
        flicker_ctx = (isp_flicker_attr *)isp_malloc(sizeof(isp_flicker_attr));
        if (flicker_ctx == TD_NULL) {
            isp_err_trace("Isp[%d] flicker_ctx malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }

        (td_void)memset_s(flicker_ctx, sizeof(isp_flicker_attr), 0, sizeof(isp_flicker_attr));
    }

    flicker_set_ctx(vi_pipe, flicker_ctx);

    return TD_SUCCESS;
}

static td_void flicker_ctx_exit(ot_vi_pipe vi_pipe)
{
    isp_flicker_attr *flicker_ctx = TD_NULL;

    flicker_get_ctx(vi_pipe, flicker_ctx);
    isp_free(flicker_ctx);
    flicker_reset_ctx(vi_pipe);
}

static td_void flicker_free_mem(ot_vi_pipe vi_pipe)
{
    isp_flicker_attr *flicker_ctx = TD_NULL;

    flicker_get_ctx(vi_pipe, flicker_ctx);
    isp_check_pointer_void_return(flicker_ctx);

    if ((flicker_ctx->malloc == TD_TRUE) && (flicker_ctx->mem != TD_NULL)) {
        isp_free(flicker_ctx->mem);
        flicker_ctx->fck_stat_info.gr_mean = TD_NULL;
        flicker_ctx->fck_stat_info.gb_mean = TD_NULL;
        flicker_ctx->fck_point.gr_mean    = TD_NULL;
        flicker_ctx->fck_point.gb_mean    = TD_NULL;
        flicker_ctx->fck_point.gr_cros0   = TD_NULL;
        flicker_ctx->fck_point.gb_cros0   = TD_NULL;
        flicker_ctx->fck_point.gr_cnt     = TD_NULL;
        flicker_ctx->fck_point.gb_cnt     = TD_NULL;
        flicker_ctx->malloc                  = TD_FALSE;
    }
}

static td_void set_read_flicker_stat_key(ot_vi_pipe vi_pipe, td_bool read_en)
{
    td_u32 isr_access;

    isr_access = ot_ext_system_statistics_ctrl_highbit_read(vi_pipe);

    if (read_en == TD_TRUE) {
        isr_access |= (1 << FLICKER_STAT_KEY_BIT);
    } else {
        isr_access &= (~(1 << FLICKER_STAT_KEY_BIT));
    }

    ot_ext_system_statistics_ctrl_highbit_write(vi_pipe, isr_access);
}

static td_void flicker_ext_regs_initialize(ot_vi_pipe vi_pipe)
{
    ot_ext_system_flicker_result_write(vi_pipe, 0);
    ot_ext_system_freq_result_write(vi_pipe, 0x2);
    ot_ext_system_flicker_min_band_num_write(vi_pipe, OT_ISP_FLICKER_MINBANDNUM);
    ot_ext_system_flicker_min_valid_band_pcnt_write(vi_pipe, OT_ISP_FLICKER_MINVALIDBANDPCNT);
    ot_ext_system_flicker_wave_diff1_write(vi_pipe, OT_ISP_FLICKER_WAVEDIFF1);
    ot_ext_system_flicker_wave_diff2_write(vi_pipe, OT_ISP_FLICKER_WAVEDIFF2);
    ot_ext_system_flicker_period_write(vi_pipe, OT_ISP_FLICKER_PERIOD);
    ot_ext_system_flicker_gr_cnt_write(vi_pipe, OT_ISP_FLICKER_GRCNT);
    ot_ext_system_flicker_gb_cnt_write(vi_pipe, OT_ISP_FLICKER_GBCNT);
    ot_ext_system_flicker_over_cnt_thr_write(vi_pipe, OT_ISP_FLICKER_OVERCNTTH);
    ot_ext_system_flicker_over_thr_write(vi_pipe, OT_ISP_FLICKER_OVERTH);
}

static td_bool get_flick_enable(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (is_wdr_mode(isp_ctx->sns_wdr_mode) && \
        ((isp_ctx->block_attr.running_mode == ISP_MODE_RUNNING_OFFLINE)  || \
         (isp_ctx->block_attr.running_mode == ISP_MODE_RUNNING_ONLINE))) {
        return TD_TRUE;
    } else {
        return TD_FALSE;
    }
}

static td_void flicker_dyna_reg_init(td_u8 wdr_mode, isp_flicker_dyna_cfg *dyna_reg_cfg)
{
    ot_unused(wdr_mode);
    dyna_reg_cfg->gr_avg_pre   = 0;
    dyna_reg_cfg->gb_avg_pre   = 0;
    dyna_reg_cfg->over_thr     = OT_ISP_FLICKER_OVERTH;
    dyna_reg_cfg->resh         = TD_TRUE;
}

static td_void flicker_regs_initialize(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_bool enable;
    td_u8 i;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    enable = get_flick_enable(vi_pipe);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        flicker_dyna_reg_init(isp_ctx->sns_wdr_mode, &reg_cfg->alg_reg_cfg[i].flicker_reg_cfg.dyna_reg_cfg);
        reg_cfg->alg_reg_cfg[i].flicker_reg_cfg.enable = enable;
    }

    reg_cfg->cfg_key.bit1_flicker_cfg = 1;
}

static td_void flicker_read_hw_stat_info(ot_vi_pipe vi_pipe, isp_stat *stat_info)
{
    td_u32 i;
    td_u32 val, tmp;
    td_s32 hw_val, hw_tmp;
    const td_u32 sft = 11;
    isp_flicker_attr *flicker_ctx = TD_NULL;

    flicker_get_ctx(vi_pipe, flicker_ctx);

    val = stat_info->flicker_stat.cur_diff_gr[0]; /* diff */
    hw_val = (val & 0x400000) ? (-((td_s32)(((~val) & 0x3FFFFF) + 1))) : (td_s32)(val & 0x3FFFFF);
    flicker_ctx->fck_stat_info.cur_frame_diff_sum_gr = hw_val;

    val = stat_info->flicker_stat.cur_diff_gb[0]; /* diff */
    hw_val = (val & 0x400000) ? (-((td_s32)(((~val) & 0x3FFFFF) + 1))) : (td_s32)(val & 0x3FFFFF);
    flicker_ctx->fck_stat_info.cur_frame_diff_sum_gb = hw_val;

    flicker_ctx->fck_stat_info.cur_frame_abs_sum_gr = stat_info->flicker_stat.cur_abs_gr[0]; /* ABS */
    flicker_ctx->fck_stat_info.cur_frame_abs_sum_gb = stat_info->flicker_stat.cur_abs_gb[0]; /* ABS */

    flicker_ctx->cur_frame_avg_gr = (OT_ISP_FLICKER_MUTIPLE * flicker_ctx->fck_stat_info.cur_frame_diff_sum_gr +
                                     (td_s32)(flicker_ctx->height >> 1)) / div_0_to_1((td_s32)flicker_ctx->height);
    flicker_ctx->cur_frame_avg_gb = (OT_ISP_FLICKER_MUTIPLE * flicker_ctx->fck_stat_info.cur_frame_diff_sum_gb +
                                     (td_s32)(flicker_ctx->height >> 1)) / div_0_to_1((td_s32)flicker_ctx->height);

    for (i = 0; i < flicker_ctx->fck_inx; i++) {
        val = stat_info->flicker_stat.g_mean[0][i];
        tmp = (val & 0x3FF800) >> sft;
        hw_tmp = (tmp & 0x400) ? (-((td_s32)(((~tmp) & 0x3FF) + 1))) : (td_s32)(tmp & 0x3FF);
        flicker_ctx->fck_stat_info.gr_mean[i] = hw_tmp;

        tmp = val & 0X7FF;
        hw_tmp = (tmp & 0x400) ? (-((td_s32)(((~tmp) & 0x3FF) + 1))) : (td_s32)(tmp & 0x3FF);
        flicker_ctx->fck_stat_info.gb_mean[i] = hw_tmp;
    }

    flicker_ctx->cur_frame_var_gr  = (OT_ISP_FLICKER_MUTIPLE * flicker_ctx->fck_stat_info.cur_frame_abs_sum_gr) /
                                     div_0_to_1(flicker_ctx->height);
    flicker_ctx->cur_frame_var_gb  = (OT_ISP_FLICKER_MUTIPLE * flicker_ctx->fck_stat_info.cur_frame_abs_sum_gb) /
                                     div_0_to_1(flicker_ctx->height);
}

static td_void flicker_result(isp_flicker_attr *flicker_ctx, const isp_flicker_extern_reg *ext_reg,
                              td_s32 gb_avgcnt, td_s32 gr_avgcnt)
{
    /* 50_hz / 60_hz */
    if (flicker_ctx->cur_flicker == 1) {
        if ((gr_avgcnt <= ext_reg->gr_cnt) && (gb_avgcnt <= ext_reg->gb_cnt)) {
            flicker_ctx->cur_freq = 1; /* 50_hz */
        } else {
            flicker_ctx->cur_freq = 0; /* 60_hz */
        }
    } else {
        flicker_ctx->cur_freq     = 2; /* 2 is the state to light off */
    }

    if (flicker_ctx->frame_num == 1) {
        if (flicker_ctx->cur_freq == 1) {
            flicker_ctx->ret_freq_result = 1;
        } else if (flicker_ctx->cur_freq == 0) {
            flicker_ctx->ret_freq_result = 0;
        } else {
            flicker_ctx->ret_freq_result = 2; /* 2 is the state to light off */
        }
        flicker_ctx->freq_hist = 0;
    } else {
        if (flicker_ctx->freq_hist == ext_reg->period) {
            flicker_ctx->ret_freq_result = flicker_ctx->pre_freq;
            flicker_ctx->freq_hist = 0;
        }
        if (flicker_ctx->cur_freq == flicker_ctx->ret_freq_result) {
            flicker_ctx->freq_hist = 0;
        } else {
            flicker_ctx->freq_hist++;
        }
    }
    flicker_ctx->pre_freq = flicker_ctx->cur_freq;
}

static td_void flicker_detect_result(ot_vi_pipe vi_pipe)
{
    td_s32 gb_avgcnt;
    td_s32 gr_avgcnt;
    isp_flicker_extern_reg *ext_reg = TD_NULL;
    isp_flicker_attr *flicker_ctx = TD_NULL;

    flicker_get_ctx(vi_pipe, flicker_ctx);

    ext_reg = &flicker_ctx->fck_ext_reg;

    gb_avgcnt = ext_reg->gb_avgcnt;
    gr_avgcnt = ext_reg->gr_avgcnt;

    flicker_ctx->pre_frame_avg_gr = flicker_ctx->cur_frame_avg_gr;
    flicker_ctx->pre_frame_avg_gb = flicker_ctx->cur_frame_avg_gb;

    /* flicker result */
    if (flicker_ctx->frame_num == 1) {
        if (flicker_ctx->cur_flicker == 1) {
            flicker_ctx->ret_flick_result = 1;
        } else {
            flicker_ctx->ret_flick_result = 0;
        }
        flicker_ctx->flicker_hist = 0;
    } else {
        if (flicker_ctx->flicker_hist == ext_reg->period) {
            flicker_ctx->ret_flick_result = !(flicker_ctx->ret_flick_result);
            flicker_ctx->flicker_hist = 0;
        }
        if (flicker_ctx->cur_flicker == flicker_ctx->ret_flick_result) {
            flicker_ctx->flicker_hist = 0;
        } else {
            flicker_ctx->flicker_hist++;
        }
    }

    flicker_result(flicker_ctx, ext_reg, gb_avgcnt, gr_avgcnt);
}

static td_void filcker_get_index(const td_s32 *cros0, td_s32 *s32indx, td_s32 *hw_cnt, td_u16 inx)
{
    td_u32 i, j;
    td_s32 cnt = 1;
    td_s32 tmp1, tmp2;

    *s32indx = 0;
    j = 1;

    for (i = 0; i < inx;) {
        tmp1 = *(cros0 + i);

        for (; j < inx;) {
            tmp2 = *(cros0 + j);

            if (tmp1 == tmp2) {
                cnt++;
                j++;
            } else {
                break;
            }
        }

        if (i == (td_u32)(inx - 1)) {
            *(hw_cnt + *s32indx) = cnt;
            break;
        } else if (j == inx) {
            *(hw_cnt + *s32indx) = cnt;
            i = inx;
        } else {
            *(hw_cnt + *s32indx) = cnt;
            cnt = 1;
            i = j;
            j++;
            (*s32indx)++;
        }
    }
}

static td_void flicker_avg_count_start(isp_flicker_avg *flicker_avg, const td_s32 *gr_cnt, const td_s32 *gb_cnt)
{
    td_s32 i;
    td_u32 gr_avgcnt;
    td_u32 gb_avgcnt;

    gr_avgcnt = flicker_avg->gr_avgcnt;
    gb_avgcnt = flicker_avg->gb_avgcnt;
    /* gr avg */
    flicker_avg->gr_avg = 0;
    for (i = 1; i < flicker_avg->grindx; i++) {
        if (*(gr_cnt + i) > OT_ISP_FLICKER_MINWIDTH) {
            flicker_avg->gr_avg += *(gr_cnt + i);
            gr_avgcnt++;
        }
    }
    if (gr_avgcnt) {
        flicker_avg->gr_avg = (flicker_avg->gr_avg + (gr_avgcnt >> 1)) / div_0_to_1(gr_avgcnt);
    } else {
        flicker_avg->gr_avg = -1;
    }
    /* gb avg */
    flicker_avg->gb_avg = 0;
    for (i = 1; i < flicker_avg->gbindx; i++) {
        if (*(gb_cnt + i) > OT_ISP_FLICKER_MINWIDTH) {
            flicker_avg->gb_avg += *(gb_cnt + i);
            flicker_avg->gb_avgcnt++;
        }
    }
    if (flicker_avg->gb_avgcnt) {
        flicker_avg->gb_avg = (flicker_avg->gb_avg + (gb_avgcnt >> 1)) / div_0_to_1(gb_avgcnt);
    } else {
        flicker_avg->gb_avg = -1;
    }
}

static td_void flicker_avg_count_end(isp_flicker_avg *flicker_avg, const td_s32 *gr_cnt, const td_s32 *gb_cnt,
    const isp_flicker_extern_reg *ext_reg)
{
    td_s32 i;

    flicker_avg->gr_pat = 0;
    for (i = 1; i < flicker_avg->grindx; i++) {
        if (abs(flicker_avg->gr_avg - *(gr_cnt + i)) <= ext_reg->wave_diff1) {
            flicker_avg->gr_pat++;
        } else if (abs(flicker_avg->gr_avg - *(gr_cnt + i)) == ext_reg->wave_diff2) {
            flicker_avg->gr_diff3cnt++;
        }
    }

    flicker_avg->gb_pat = 0;
    for (i = 1; i < flicker_avg->gbindx; i++) {
        if (abs(flicker_avg->gb_avg - *(gb_cnt + i)) <= ext_reg->wave_diff1) {
            flicker_avg->gb_pat++;
        } else if (abs(flicker_avg->gb_avg - *(gb_cnt + i)) == ext_reg->wave_diff2) {
            flicker_avg->gb_diff3cnt++;
        }
    }
    if (flicker_avg->gr_diff3cnt == 1) {
        flicker_avg->gr_pat++;
    }

    if (flicker_avg->gb_diff3cnt == 1) {
        flicker_avg->gb_pat++;
    }

    if (flicker_avg->gr_avg == -1 || flicker_avg->gb_avg == -1) {
        flicker_avg->gr_pat = -1;
        flicker_avg->gb_pat = -1;
    }
}

static td_void flicker_avg_count(isp_flicker_extern_reg *ext_reg, isp_flicker_attr *flicker_ctx,
                                 isp_flicker_avg *flicker_avg, td_s32 *gr_cnt, td_s32 *gb_cnt)
{
    const td_s32 rnd_adder = 5;
    const td_s32 rnd_divisor = 10;

    /* remove first and last wave */
    flicker_avg->gr_diff3cnt = 0;
    flicker_avg->gb_diff3cnt = 0;
    flicker_avg->gr_avgcnt   = 0;
    flicker_avg->gb_avgcnt   = 0;

    if (flicker_avg->grindx > ext_reg->min_band_num && flicker_avg->gbindx > ext_reg->min_band_num) {
        flicker_avg_count_start(flicker_avg, gr_cnt, gb_cnt);
        flicker_avg_count_end(flicker_avg, gr_cnt, gb_cnt, ext_reg);
    } else {
        flicker_avg->gr_pat = -1;
        flicker_avg->gb_pat = -1;
    }
    flicker_avg->grflicker_thd = ((flicker_avg->grindx * ext_reg->min_valid_band_pcnt) + rnd_adder) / rnd_divisor;
    flicker_avg->gbflicker_thd = ((flicker_avg->gbindx * ext_reg->min_valid_band_pcnt) + rnd_adder) / rnd_divisor;
    /* result */
    flicker_ctx->cur_flicker = 0;
    if (flicker_avg->gr_pat >= flicker_avg->grflicker_thd && flicker_avg->gb_pat >= flicker_avg->gbflicker_thd) {
        flicker_ctx->cur_flicker = 1;
    }

    ext_reg->gb_avgcnt = flicker_avg->gb_avgcnt;
    ext_reg->gr_avgcnt = flicker_avg->gr_avgcnt;
}

static td_void flicker_detect(ot_vi_pipe vi_pipe)
{
    td_u16 inx;
    td_u32 i;
    td_s32 grindx, gbindx;
    td_s32 *gr_cros0 = TD_NULL;
    td_s32 *gb_cros0 = TD_NULL;
    td_s32 *gr_cnt = TD_NULL;
    td_s32 *gb_cnt = TD_NULL;

    isp_flicker_extern_reg *ext_reg = TD_NULL;
    isp_flicker_attr *flicker_ctx = TD_NULL;
    isp_flicker_avg flicker_avg;

    flicker_get_ctx(vi_pipe, flicker_ctx);

    ext_reg = &flicker_ctx->fck_ext_reg;

    inx = flicker_ctx->fck_inx;

    gr_cros0 = flicker_ctx->fck_point.gr_cros0;
    gb_cros0 = flicker_ctx->fck_point.gb_cros0;

    gr_cnt   = flicker_ctx->fck_point.gr_cnt;
    gb_cnt   = flicker_ctx->fck_point.gb_cnt;

    /* previous frame-based mean is the zero-value axis.
     * if group mean value is above this zero-value, it is marked as 1;
     * otherwise, it is marked as 0.
     */
    for (i = 0; i < inx; i++) {
        *(gr_cros0 + i) = 0;
        *(gb_cros0 + i) = 0;

        if (flicker_ctx->fck_stat_info.gr_mean[i] >= flicker_ctx->pre_frame_avg_gr) {
            *(gr_cros0 + i) = 1;
        }

        if (flicker_ctx->fck_stat_info.gb_mean[i] >= flicker_ctx->pre_frame_avg_gb) {
            *(gb_cros0 + i) = 1;
        }
    }

    /* gr_cnt, gb_cnt: the width of each wave
     * gr_avg, gb_avg: the average width of wave
     * gr_pat, gb_pat: the total number of valid wave
     * gr/gbflicker_thd: min. number of valid wave to indicate flickering
     * comparing the wave's avg. width with each wave's width,
     * if the difference is less than 2, this wave is valid flicker wave.
     * we allow one time of difference being 3 (from the video we have had.)
     */
    grindx  = 0;
    gbindx  = 0;
    filcker_get_index(gr_cros0, &grindx, gr_cnt, inx);
    filcker_get_index(gb_cros0, &gbindx, gb_cnt, inx);

    flicker_avg.grindx = grindx;

    flicker_avg.gbindx = gbindx;

    flicker_avg_count(ext_reg, flicker_ctx, &flicker_avg, gr_cnt, gb_cnt);
    flicker_detect_result(vi_pipe);
}

static td_s32 flicker_read_extregs(ot_vi_pipe vi_pipe)
{
    isp_flicker_extern_reg *ext_reg = TD_NULL;
    isp_flicker_attr  *flicker_ctx = TD_NULL;

    flicker_get_ctx(vi_pipe, flicker_ctx);

    ext_reg = &flicker_ctx->fck_ext_reg;

    ext_reg->min_band_num        = ot_ext_system_flicker_min_band_num_read(vi_pipe);
    ext_reg->min_valid_band_pcnt = ot_ext_system_flicker_min_valid_band_pcnt_read(vi_pipe);
    ext_reg->wave_diff1          = ot_ext_system_flicker_wave_diff1_read(vi_pipe);
    ext_reg->wave_diff2          = ot_ext_system_flicker_wave_diff2_read(vi_pipe);
    ext_reg->period              = ot_ext_system_flicker_period_read(vi_pipe);
    ext_reg->gr_cnt              = ot_ext_system_flicker_gr_cnt_read(vi_pipe);
    ext_reg->gb_cnt              = ot_ext_system_flicker_gr_cnt_read(vi_pipe);
    flicker_ctx->over_thr        = ot_ext_system_flicker_over_thr_read(vi_pipe);
    flicker_ctx->over_cnt_thr    = ot_ext_system_flicker_over_cnt_thr_read(vi_pipe);
    return TD_SUCCESS;
}

static td_void flicker_initialize(ot_vi_pipe vi_pipe)
{
    size_t mem_size, mem_cnt;
    td_u16 height;
    const td_u16 inx_sft = 4;
    isp_flicker_attr *flicker_ctx = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    flicker_get_ctx(vi_pipe, flicker_ctx);

    height = isp_ctx->block_attr.frame_rect.height;

    flicker_ctx->frame_num = 0;
    flicker_ctx->height    = height;
    flicker_ctx->fck_inx   = height >> inx_sft;

    flicker_ctx->pre_frame_avg_gb = 0;
    flicker_ctx->pre_frame_avg_gr = 0;
    flicker_ctx->enable           = get_flick_enable(vi_pipe);

    flicker_free_mem(vi_pipe);

    mem_size = (height / OT_ISP_FLICKER_GROUP) * sizeof(td_s32);
    mem_cnt = sizeof(isp_flicker_point_attr) / sizeof(td_s32);

    if (mem_size == 0 || mem_cnt == 0) {
        return;
    }
    flicker_ctx->mem = (td_s32 *)isp_malloc(mem_size * mem_cnt);

    if (flicker_ctx->mem == TD_NULL) {
        isp_err_trace("ISP[%d]:Alloc Mem for Flicker Failure!\n", vi_pipe);
        flicker_ctx->enable = TD_FALSE;
        flicker_ctx->malloc = TD_FALSE;
        return;
    }

    flicker_ctx->fck_point.gr_mean  = flicker_ctx->mem;
    flicker_ctx->fck_point.gb_mean  = (td_s32 *)((td_u8 *)flicker_ctx->fck_point.gr_mean  + mem_size);
    flicker_ctx->fck_point.gr_cros0 = (td_s32 *)((td_u8 *)flicker_ctx->fck_point.gb_mean  + mem_size);
    flicker_ctx->fck_point.gb_cros0 = (td_s32 *)((td_u8 *)flicker_ctx->fck_point.gr_cros0 + mem_size);
    flicker_ctx->fck_point.gr_cnt   = (td_s32 *)((td_u8 *)flicker_ctx->fck_point.gb_cros0 + mem_size);
    flicker_ctx->fck_point.gb_cnt   = (td_s32 *)((td_u8 *)flicker_ctx->fck_point.gr_cnt   + mem_size);

    (td_void)memset_s(flicker_ctx->fck_point.gr_mean, mem_size, 0, mem_size);
    (td_void)memset_s(flicker_ctx->fck_point.gb_mean, mem_size, 0, mem_size);
    (td_void)memset_s(flicker_ctx->fck_point.gr_cros0, mem_size, 0, mem_size);
    (td_void)memset_s(flicker_ctx->fck_point.gb_cros0, mem_size, 0, mem_size);
    (td_void)memset_s(flicker_ctx->fck_point.gr_cnt,  mem_size, -1, mem_size);
    (td_void)memset_s(flicker_ctx->fck_point.gb_cnt,  mem_size, -1, mem_size);

    flicker_ctx->fck_stat_info.gr_mean = flicker_ctx->fck_point.gr_mean;
    flicker_ctx->fck_stat_info.gb_mean = flicker_ctx->fck_point.gb_mean;

    flicker_ctx->malloc = TD_TRUE;
}

static td_s32 isp_flicker_init(ot_vi_pipe vi_pipe, td_void *cfg)
{
    td_s32 ret;
    isp_flicker_attr *flicker_ctx = TD_NULL;
    isp_reg_cfg    *reg_cfg     = (isp_reg_cfg *)cfg;

    ret = flicker_ctx_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    flicker_get_ctx(vi_pipe, flicker_ctx);
    isp_check_pointer_return(flicker_ctx);

    flicker_ext_regs_initialize(vi_pipe);
    flicker_regs_initialize(vi_pipe, reg_cfg);
    flicker_initialize(vi_pipe);

    set_read_flicker_stat_key(vi_pipe, flicker_ctx->enable);

    return TD_SUCCESS;
}

static td_s32 isp_flicker_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *cfg, td_s32 rsv)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_flicker_attr *flicker_ctx = TD_NULL;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)cfg;
    isp_flicker_dyna_cfg *dyna_reg_cfg = TD_NULL;
    ot_unused(rsv);

    isp_get_ctx(vi_pipe, isp_ctx);
    flicker_get_ctx(vi_pipe, flicker_ctx);
    isp_check_pointer_return(flicker_ctx);

    flicker_ctx->frame_num = isp_ctx->frame_cnt;

    if (!flicker_ctx->enable) {
        return TD_SUCCESS;
    }

    if (isp_ctx->linkage.stat_ready  == TD_FALSE) {
        return TD_SUCCESS;
    }

    isp_check_pointer_return(flicker_ctx->mem);
    ret = flicker_read_extregs(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    flicker_read_hw_stat_info(vi_pipe, (isp_stat *)stat_info);

    if (flicker_ctx->frame_num) {
        flicker_detect(vi_pipe);
    }

    isp_ctx->linkage.flicker     = (td_bool)flicker_ctx->ret_flick_result;
    isp_ctx->linkage.freq_result = (td_u8)flicker_ctx->ret_freq_result;

    ot_ext_system_flicker_result_write(vi_pipe, flicker_ctx->ret_flick_result);
    ot_ext_system_freq_result_write(vi_pipe, flicker_ctx->ret_freq_result);

    dyna_reg_cfg = &reg_cfg->alg_reg_cfg[0].flicker_reg_cfg.dyna_reg_cfg;

    dyna_reg_cfg->over_thr     = flicker_ctx->over_thr;
    dyna_reg_cfg->gr_avg_pre   = flicker_ctx->pre_frame_avg_gr;
    dyna_reg_cfg->gb_avg_pre   = flicker_ctx->pre_frame_avg_gb;

    reg_cfg->cfg_key.bit1_flicker_cfg = 1;

    return TD_SUCCESS;
}

static td_s32 isp_flicker_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    isp_reg_cfg_attr  *reg_cfg   = TD_NULL;
    ot_unused(value);

    switch (cmd) {
        case OT_ISP_WDR_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, reg_cfg);
            isp_check_pointer_return(reg_cfg);
            isp_flicker_init(vi_pipe, (td_void *)&reg_cfg->reg_cfg);
            break;
        case OT_ISP_CHANGE_IMAGE_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, reg_cfg);
            isp_check_pointer_return(reg_cfg);
            isp_flicker_init(vi_pipe, (td_void *)&reg_cfg->reg_cfg);
            break;
        default:
            break;
    }

    return TD_SUCCESS;
}

static td_s32 isp_flicker_exit(ot_vi_pipe vi_pipe)
{
    flicker_free_mem(vi_pipe);
    flicker_ctx_exit(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_alg_register_flicker(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_flicker);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_FLICKER;
    algs->alg_func.pfn_alg_init = isp_flicker_init;
    algs->alg_func.pfn_alg_run  = isp_flicker_run;
    algs->alg_func.pfn_alg_ctrl = isp_flicker_ctrl;
    algs->alg_func.pfn_alg_exit = isp_flicker_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
