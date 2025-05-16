/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <isp_alg.h>

#define ISP_MINIMUM_SYSTEM
/* find the specified library */
td_s32 isp_find_lib(const isp_lib_node *lib_node, const ot_isp_3a_alg_lib *lib)
{
    td_s32 i;

    for (i = 0; i < OT_ISP_MAX_REGISTER_ALG_LIB_NUM; i++) {
        /* can't use memcmp, if user not fill the string. */
        if ((strncmp(lib_node[i].alg_lib.lib_name, lib->lib_name, ALG_LIB_NAME_SIZE_MAX) == 0) &&
            (lib_node[i].alg_lib.id == lib->id)) {
            return i;
        }
    }

    return -1;
}

/* search the empty pos of libs */
isp_lib_node *isp_search_lib(isp_lib_node *lib_node)
{
    td_s32 i;

    for (i = 0; i < OT_ISP_MAX_REGISTER_ALG_LIB_NUM; i++) {
        if (!lib_node[i].used) {
            return &lib_node[i];
        }
    }

    return TD_NULL;
}

/* search the empty pos of algs */
isp_alg_node *isp_search_alg(isp_alg_node *algs)
{
    td_s32 i;

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (!algs[i].used) {
            return &algs[i];
        }
    }

    return TD_NULL;
}

td_void isp_algs_init(const isp_alg_node *algs, ot_vi_pipe vi_pipe, td_void *reg_cfg)
{
    td_s32 i;

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (algs[i].used) {
            if (algs[i].alg_func.pfn_alg_init != TD_NULL) {
                algs[i].alg_func.pfn_alg_init(vi_pipe, reg_cfg);
            }
        }
    }
}

td_void isp_algs_run(const isp_alg_node *algs, ot_vi_pipe vi_pipe,
                     const td_void *stat_info, td_void *reg_cfg_info, td_s32 rsv)
{
    td_s32 i;

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (algs[i].used) {
            if (algs[i].alg_func.pfn_alg_run != TD_NULL) {
                algs[i].alg_func.pfn_alg_run(vi_pipe, stat_info, reg_cfg_info, rsv);
            }
        }
    }
}

td_void isp_algs_ctrl(const isp_alg_node *algs, ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    td_s32 i;

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (algs[i].used) {
            if (algs[i].alg_func.pfn_alg_ctrl != TD_NULL) {
                algs[i].alg_func.pfn_alg_ctrl(vi_pipe, cmd, value);
            }
        }
    }
}

td_void isp_algs_exit(ot_vi_pipe vi_pipe, const isp_alg_node *algs)
{
    td_s32 i;

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (algs[i].used) {
            if (algs[i].alg_func.pfn_alg_exit != TD_NULL) {
                algs[i].alg_func.pfn_alg_exit(vi_pipe);
            }
        }
    }
}

static td_void isp_alg_register_3a(ot_vi_pipe vi_pipe)
{
    isp_alg_register_ae(vi_pipe);
    isp_alg_register_awb(vi_pipe);
#ifdef CONFIG_OT_ISP_AF_SUPPORT
    isp_alg_register_af(vi_pipe);
#endif
}

static td_void isp_alg_register_dcg(ot_vi_pipe vi_pipe)
{
    isp_alg_register_dpc(vi_pipe);
#ifdef CONFIG_OT_ISP_CR_SUPPORT
    isp_alg_register_ge(vi_pipe);
#endif
}

static td_void isp_alg_register_wdr(ot_vi_pipe vi_pipe)
{
    isp_alg_register_frame_wdr(vi_pipe);
    isp_alg_register_expander(vi_pipe);
    isp_alg_register_flicker(vi_pipe);
}

static td_void isp_alg_register_shading(ot_vi_pipe vi_pipe)
{
    isp_alg_register_acs(vi_pipe);
    isp_alg_register_lsc(vi_pipe);
}

static td_void isp_alg_register_dem(ot_vi_pipe vi_pipe)
{
    isp_alg_register_cac(vi_pipe);
    isp_alg_register_bayershp(vi_pipe);
    isp_alg_register_demosaic(vi_pipe);
    isp_alg_register_fcr(vi_pipe);
}

static td_void isp_alg_register_all_gamma(ot_vi_pipe vi_pipe)
{
    isp_alg_register_gamma(vi_pipe);
#ifdef CONFIG_OT_ISP_PREGAMMA_SUPPORT
    isp_alg_register_pregamma(vi_pipe);
#endif
}

static td_void isp_alg_register_color(ot_vi_pipe vi_pipe)
{
    ot_unused(vi_pipe);
#ifdef CONFIG_OT_ISP_CA_SUPPORT
    isp_alg_register_ca(vi_pipe);
#endif
    isp_alg_register_clut(vi_pipe);
}

td_void isp_algs_register(ot_vi_pipe vi_pipe)
{
    isp_alg_register_3a(vi_pipe);
    isp_alg_register_dcg(vi_pipe);
    isp_alg_register_wdr(vi_pipe);

#ifdef CONFIG_OT_VI_PIPE_FPN
    isp_alg_register_fpn(vi_pipe);
#endif
    isp_alg_register_blc(vi_pipe);
    isp_alg_register_bayer_nr(vi_pipe);
    isp_alg_register_dg(vi_pipe);
    isp_alg_register_hrs(vi_pipe);
    isp_alg_register_shading(vi_pipe);
    isp_alg_register_rc(vi_pipe);
    isp_alg_register_drc(vi_pipe);
    isp_alg_register_dehaze(vi_pipe);
    isp_alg_register_dem(vi_pipe);
    isp_alg_register_all_gamma(vi_pipe);
    isp_alg_register_csc(vi_pipe);

    isp_alg_register_sharpen(vi_pipe);
    isp_alg_register_mcds(vi_pipe);
    isp_alg_register_ldci(vi_pipe);

    isp_alg_register_color(vi_pipe);
#ifdef CONFIG_OT_ISP_RGBIR_SUPPORT
    isp_alg_register_rgbir(vi_pipe);
#endif
    isp_alg_register_lblc(vi_pipe);
}

td_void isp_fe_algs_register(ot_vi_pipe vi_pipe)
{
    isp_alg_register_ae(vi_pipe);
#ifdef CONFIG_OT_ISP_AF_SUPPORT
    isp_alg_register_af(vi_pipe);
#endif
    isp_alg_register_blc(vi_pipe);
    isp_alg_register_dg(vi_pipe);
    isp_alg_register_hrs(vi_pipe);
    isp_alg_register_lblc(vi_pipe);
}

td_void isp_yuv_algs_register(ot_vi_pipe vi_pipe)
{
#ifdef CONFIG_OT_ISP_AF_SUPPORT
    isp_alg_register_af(vi_pipe);
#endif
    isp_alg_register_rc(vi_pipe);
    isp_alg_register_sharpen(vi_pipe);
    isp_alg_register_mcds(vi_pipe);
    isp_alg_register_ldci(vi_pipe);

#ifdef CONFIG_OT_ISP_CA_SUPPORT
    isp_alg_register_ca(vi_pipe);
#endif

    return;
}

/* resolve problem: isp error: null pointer in isp_alg_registerxxx when return isp_init without app exit */
td_void isp_algs_unregister(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_s32 i;

    isp_get_ctx(vi_pipe, isp_ctx);

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (isp_ctx->algs[i].used) {
            isp_ctx->algs[i].used = TD_FALSE;
        }
    }
}

/* resolve problem: isp error: OT_MPI_XX_Register failed when return 3a lib register without app exit */
td_void isp_libs_unregister(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_s32 i;

    isp_get_ctx(vi_pipe, isp_ctx);

    for (i = 0; i < OT_ISP_MAX_REGISTER_ALG_LIB_NUM; i++) {
        isp_ctx->ae_lib_info.libs[i].used  = TD_FALSE;
        isp_ctx->awb_lib_info.libs[i].used = TD_FALSE;
        isp_ctx->af_lib_info.libs[i].used  = TD_FALSE;
    }
}

