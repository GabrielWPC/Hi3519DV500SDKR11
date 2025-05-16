/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <math.h>
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_config.h"
#include "isp_proc.h"
#include "isp_ext_config.h"
#include "ot_math.h"
#include "isp_param_check.h"

static const td_u32 g_pregamma_default[OT_ISP_PREGAMMA_NODE_NUM] = {
    0,      16,     32,     48,     64,     80,     96,     112,    128,    160,    192,    224,
    256,    288,    320,    352,    384,    416,    448,    480,    512,    576,    640,    704,
    768,    832,    896,    960,    1024,   1088,   1152,   1216,   1280,   1344,   1408,   1472,
    1536,   1664,   1792,   1920,   2048,   2176,   2304,   2432,   2560,   2688,   2816,   2944,
    3072,   3200,   3328,   3456,   3584,   3840,   4096,   4352,   4608,   4864,   5120,   5376,
    5632,   5888,   6144,   6400,   6656,   6912,   7168,   7424,   7680,   8192,   8704,   9216,
    9728,   10240,  10752,  11264,  11776,  12288,  12800,  13312,  13824,  14336,  14848,  15360,
    15872,  16384,  16896,  17408,  17920,  18432,  18944,  19456,  19968,  20480,  20992,  21504,
    22016,  22528,  23040,  23552,  24064,  24576,  25600,  26624,  27648,  28672,  29696,  30720,
    31744,  32768,  33792,  34816,  35840,  36864,  37888,  38912,  39936,  40960,  41984,  43008,
    44032,  45056,  46080,  47104,  48128,  49152,  50176,  51200,  52224,  53248,  54272,  55296,
    56320,  57344,  59392,  61440,  63488,  65536,  67584,  69632,  71680,  73728,  75776,  77824,
    79872,  81920,  83968,  86016,  88064,  90112,  92160,  94208,  96256,  98304,  100352, 102400,
    104448, 106496, 108544, 110592, 112640, 114688, 116736, 118784, 120832, 122880, 126976, 131072,
    135168, 139264, 143360, 147456, 151552, 155648, 159744, 163840, 167936, 172032, 176128, 180224,
    184320, 188416, 192512, 196608, 200704, 204800, 208896, 212992, 217088, 221184, 225280, 229376,
    233472, 237568, 241664, 245760, 249856, 253952, 258048, 262144, 266240, 270336, 278528, 286720,
    294912, 303104, 311296, 319488, 327680, 335872, 344064, 352256, 360448, 368640, 376832, 385024,
    393216, 401408, 409600, 417792, 425984, 434176, 442368, 450560, 458752, 466944, 475136, 483328,
    491520, 499712, 507904, 516096, 524288, 540672, 557056, 573440, 589824, 606208, 622592, 638976,
    655360, 671744, 688128, 704512, 720896, 737280, 753664, 770048, 786432, 819200, 851968, 884736,
    917504, 950272, 983040, 1015808, 1048575
};

static const td_u8 g_pregamma_seg_idx_base[PREGAMMA_SEG_NUM] = {
    0, 4, 12, 24, 38, 53, 77, 105, 135, 168, 200, 224
};

static const td_u16 g_pregamma_seg_max_val[PREGAMMA_SEG_NUM] = {
    1, 4, 12, 28, 60, 192, 448, 960, 2112, 4096, 6144, 8192
};

typedef struct {
    td_bool init;
    td_bool enable;
    td_bool lut_update;
} isp_pregamma;

isp_pregamma g_pregamma_ctx[OT_ISP_MAX_PIPE_NUM] = {{0}};
#define pregamma_get_ctx(pipe, ctx)   ((ctx) = &g_pregamma_ctx[(pipe)])

static td_void pregamma_static_regs_init(ot_vi_pipe vi_pipe, td_u8 blk_idx, isp_reg_cfg *reg_cfg)
{
    ot_unused(vi_pipe);
    isp_pregamma_static_cfg *static_reg_cfg = &(reg_cfg->alg_reg_cfg[blk_idx].pregamma_reg_cfg.static_reg_cfg);
    td_u32 idx_base_lut_size = PREGAMMA_SEG_NUM * sizeof(td_u8);
    td_u32 max_val_lut_size = PREGAMMA_SEG_NUM * sizeof(td_u16);

    (td_void)memcpy_s(static_reg_cfg->seg_idx_base, idx_base_lut_size, g_pregamma_seg_idx_base, idx_base_lut_size);
    (td_void)memcpy_s(static_reg_cfg->seg_max_val, max_val_lut_size, g_pregamma_seg_max_val,  max_val_lut_size);

    static_reg_cfg->resh   = TD_TRUE;
}

static td_void pregamma_dyna_regs_init(ot_vi_pipe vi_pipe, td_u8 blk_idx, isp_reg_cfg *reg_cfg,
                                       const td_u32 *pregamma_lut)
{
    td_s32 ret;
    isp_pregamma_dyna_cfg *dyna_reg_cfg = &(reg_cfg->alg_reg_cfg[blk_idx].pregamma_reg_cfg.dyna_reg_cfg);
    td_u32 pregamma_lut_size = OT_ISP_PREGAMMA_NODE_NUM * sizeof(td_u32);

    dyna_reg_cfg->lut_update = TD_TRUE;
    dyna_reg_cfg->update_index = 1;
    ret = memcpy_s(dyna_reg_cfg->pregamma_lut, pregamma_lut_size, pregamma_lut, pregamma_lut_size);
    if (ret != EOK) {
        isp_err_trace("ISP[%d] init pregamma lut failed!\n", vi_pipe);
    }
}

static td_s32 pregamma_regs_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_bool offline_mode;
    td_u16 i;
    td_s32 ret;
    const td_u32         *pregamma_lut  = TD_NULL;
    isp_pregamma_reg_cfg *pregamma_reg_cfg = TD_NULL;
    isp_pregamma         *pregamma_ctx = TD_NULL;
    isp_usr_ctx          *isp_ctx      = TD_NULL;
    ot_isp_cmos_default  *sns_dft      = TD_NULL;

    isp_sensor_get_default(vi_pipe, &sns_dft);
    isp_get_ctx(vi_pipe, isp_ctx);
    pregamma_get_ctx(vi_pipe, pregamma_ctx);

    pregamma_ctx->init = TD_FALSE;

    offline_mode = (is_offline_mode(isp_ctx->block_attr.running_mode) ||
                    is_striping_mode(isp_ctx->block_attr.running_mode) ||
                    is_pre_online_post_offline(isp_ctx->block_attr.running_mode));

    pregamma_ctx->lut_update = TD_FALSE;

    /* read from CMOS */
    if (sns_dft->key.bit1_pregamma) {
        isp_check_pointer_return(sns_dft->pregamma);
        ret = isp_pregamma_attr_check("cmos", sns_dft->pregamma);
        if (ret != TD_SUCCESS) {
            return ret;
        }
        pregamma_lut         = sns_dft->pregamma->table;
        pregamma_ctx->enable = sns_dft->pregamma->enable;
    } else {
        pregamma_lut         = g_pregamma_default;
        pregamma_ctx->enable =  TD_FALSE;
    }

    for (i = 0; i < OT_ISP_PREGAMMA_NODE_NUM; i++) {
        ot_ext_system_pregamma_lut_write(vi_pipe, i, pregamma_lut[i]);
    }

    for (i = 0; i < isp_ctx->block_attr.block_num; i++) {
        pregamma_reg_cfg = &reg_cfg->alg_reg_cfg[i].pregamma_reg_cfg;
        pregamma_reg_cfg->enable = offline_mode ? (pregamma_ctx->enable) : (TD_FALSE);
        pregamma_static_regs_init(vi_pipe, i, reg_cfg);
        pregamma_dyna_regs_init(vi_pipe, i, reg_cfg, pregamma_lut);
    }

    reg_cfg->cfg_key.bit1_pregamma_cfg = 1;
    pregamma_ctx->init = TD_TRUE;
    return TD_SUCCESS;
}

static td_void pregamma_ext_regs_init(ot_vi_pipe vi_pipe)
{
    isp_pregamma *pregamma_ctx = TD_NULL;

    pregamma_get_ctx(vi_pipe, pregamma_ctx);

    ot_ext_system_pregamma_en_write(vi_pipe, pregamma_ctx->enable);
    ot_ext_system_pregamma_lut_update_write(vi_pipe, TD_FALSE);
}

static td_s32 isp_pregamma_init(ot_vi_pipe vi_pipe, td_void *cfg)
{
    td_s32 ret;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)cfg;

    ot_ext_system_isp_pregamma_init_status_write(vi_pipe, TD_FALSE);
    ret = pregamma_regs_init(vi_pipe, reg_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    pregamma_ext_regs_init(vi_pipe);
    ot_ext_system_isp_pregamma_init_status_write(vi_pipe, TD_TRUE);
    return TD_SUCCESS;
}

static td_s32 isp_pregamma_wdr_mode_set(ot_vi_pipe vi_pipe, td_void *cfg)
{
    td_u8  i;
    td_u32 update_idx[OT_ISP_STRIPING_MAX_NUM] = {0};
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)cfg;
    td_s32 ret;

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        update_idx[i] = reg_cfg->alg_reg_cfg[i].pregamma_reg_cfg.dyna_reg_cfg.update_index;
        reg_cfg->alg_reg_cfg[i].pregamma_reg_cfg.enable = TD_FALSE;
    }
    reg_cfg->cfg_key.bit1_pregamma_cfg = 1;

    ret = isp_pregamma_init(vi_pipe, reg_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].pregamma_reg_cfg.dyna_reg_cfg.update_index = update_idx[i] + 1;
    }

    return TD_SUCCESS;
}

static td_void pregamma_read_ext_regs(ot_vi_pipe vi_pipe)
{
    isp_pregamma *pregamma_ctx = TD_NULL;

    pregamma_get_ctx(vi_pipe, pregamma_ctx);

    pregamma_ctx->lut_update = ot_ext_system_pregamma_lut_update_read(vi_pipe);
    if (pregamma_ctx->lut_update == TD_TRUE) {
        ot_ext_system_pregamma_lut_update_write(vi_pipe, TD_FALSE);
    }
}

static td_s32 pregamma_proc_write(ot_vi_pipe vi_pipe, ot_isp_ctrl_proc_write *proc)
{
    ot_isp_ctrl_proc_write proc_tmp;
    isp_pregamma *pregamma = TD_NULL;

    pregamma_get_ctx(vi_pipe, pregamma);

    if ((proc->proc_buff == TD_NULL) || (proc->buff_len == 0)) {
        return TD_FAILURE;
    }

    proc_tmp.proc_buff = proc->proc_buff;
    proc_tmp.buff_len = proc->buff_len;

    isp_proc_print_title(&proc_tmp, &proc->write_len, "pregamma info");

    isp_proc_printf(&proc_tmp, proc->write_len, "%16s\n", "enable");

    isp_proc_printf(&proc_tmp, proc->write_len, "%16u\n", pregamma->enable);

    proc->write_len += 1;

    return TD_SUCCESS;
}

static __inline td_bool  check_pregamma_open(const isp_pregamma *pregamma)
{
    return (pregamma->enable == TD_TRUE);
}

static td_s32 isp_pregamma_run(ot_vi_pipe vi_pipe, const td_void *stat_info,
    td_void *cfg, td_s32 rsv)
{
    td_u16 i, j;
    ot_isp_alg_mod alg_mod = OT_ISP_ALG_PREGAMMA;
    isp_pregamma *pregamma_ctx = TD_NULL;
    isp_reg_cfg  *reg  = (isp_reg_cfg *)cfg;
    isp_pregamma_dyna_cfg *dyna_reg_cfg = TD_NULL;
    ot_unused(stat_info);
    ot_unused(rsv);

    pregamma_get_ctx(vi_pipe, pregamma_ctx);

    ot_ext_system_isp_pregamma_init_status_write(vi_pipe, pregamma_ctx->init);
    if (pregamma_ctx->init != TD_TRUE) {
        ioctl(isp_get_fd(vi_pipe), ISP_ALG_INIT_ERR_INFO_PRINT, &alg_mod);
        return TD_SUCCESS;
    }

    pregamma_ctx->enable = ot_ext_system_pregamma_en_read(vi_pipe);

    for (i = 0; i < reg->cfg_num; i++) {
        reg->alg_reg_cfg[i].pregamma_reg_cfg.enable = pregamma_ctx->enable;
    }

    reg->cfg_key.bit1_pregamma_cfg = 1;

    /* check hardware setting */
    if (!check_pregamma_open(pregamma_ctx)) {
        return TD_SUCCESS;
    }

    pregamma_read_ext_regs(vi_pipe);

    if (pregamma_ctx->lut_update) {
        for (i = 0; i < reg->cfg_num; i++) {
            dyna_reg_cfg = &reg->alg_reg_cfg[i].pregamma_reg_cfg.dyna_reg_cfg;
            for (j = 0; j < OT_ISP_PREGAMMA_NODE_NUM; j++) {
                dyna_reg_cfg->pregamma_lut[j] = ot_ext_system_pregamma_lut_read(vi_pipe, j);
            }

            dyna_reg_cfg->lut_update = TD_TRUE;
            dyna_reg_cfg->update_index += 1;
        }
    }

    return TD_SUCCESS;
}

static td_s32 isp_pregamma_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    isp_reg_cfg_attr *reg_cfg = TD_NULL;
    td_s32 ret = TD_SUCCESS;

    switch (cmd) {
        case OT_ISP_WDR_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, reg_cfg);
            isp_check_pointer_return(reg_cfg);
            ret = isp_pregamma_wdr_mode_set(vi_pipe, (td_void *)&reg_cfg->reg_cfg);
            break;
        case OT_ISP_PROC_WRITE:
            ret = pregamma_proc_write(vi_pipe, (ot_isp_ctrl_proc_write *)value);
            break;
        default:
            break;
    }

    return ret;
}

static td_s32 isp_pregamma_exit(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, reg_cfg);
    ot_ext_system_isp_pregamma_init_status_write(vi_pipe, TD_FALSE);
    for (i = 0; i < reg_cfg->reg_cfg.cfg_num; i++) {
        reg_cfg->reg_cfg.alg_reg_cfg[i].pregamma_reg_cfg.enable = TD_FALSE;
    }

    reg_cfg->reg_cfg.cfg_key.bit1_pregamma_cfg = 1;
    return TD_SUCCESS;
}

td_s32 isp_alg_register_pregamma(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_pregamma);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_PREGAMMA;
    algs->alg_func.pfn_alg_init = isp_pregamma_init;
    algs->alg_func.pfn_alg_run  = isp_pregamma_run;
    algs->alg_func.pfn_alg_ctrl = isp_pregamma_ctrl;
    algs->alg_func.pfn_alg_exit = isp_pregamma_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
