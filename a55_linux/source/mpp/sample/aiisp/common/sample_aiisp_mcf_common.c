/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "sample_comm.h"
#include "ot_common_ai3dnr.h"
#include "sample_aiisp_common.h"
#include "sample_aiisp_mcf_common.h"

static ot_vb_pool g_aiisp_mcf_vb_pool = OT_VB_INVALID_POOL_ID;

ot_size g_mcf_chn_out_size[OT_MCF_MAX_PHYS_CHN_NUM] = { 0 };
td_bool g_mcf_chn_enable[OT_MCF_MAX_PHYS_CHN_NUM] = {TD_TRUE, TD_FALSE, TD_FALSE};

td_bool g_vpss_chn_enable[OT_VPSS_MAX_PHYS_CHN_NUM] = {TD_TRUE, TD_TRUE, TD_FALSE};
#define MCF_VO_WIDHT_1920 1920
#define MCF_VO_HEIGHT_1080 1080
#define MCF_BIND_VPSS_GRP 2

ot_vb_pool g_pool_id_color;
ot_vb_pool g_pool_id_mono;

static td_s32 sample_aiisp_common_vi_bind_mcf(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, ot_mcf_grp mcf_grp,
    ot_mcf_pipe mcf_pipe)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dest_chn.mod_id = OT_ID_MCF;
    dest_chn.dev_id = mcf_grp;
    dest_chn.chn_id = mcf_pipe;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(VI-MCF)");

    return TD_SUCCESS;
}

static td_s32 sample_aiisp_common_vi_unbind_mcf(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, ot_mcf_grp mcf_grp,
    ot_mcf_pipe mcf_pipe)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dest_chn.mod_id = OT_ID_MCF;
    dest_chn.dev_id = mcf_grp;
    dest_chn.chn_id = mcf_pipe;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(VI-MCF)");

    return TD_SUCCESS;
}

td_s32 sample_aiisp_common_mcf_bind_vpss(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn, ot_vpss_grp vpss_grp,
    ot_vpss_chn vpss_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_MCF;
    src_chn.dev_id = mcf_grp;
    src_chn.chn_id = mcf_chn;

    dest_chn.mod_id = OT_ID_VPSS;
    dest_chn.dev_id = vpss_grp;
    dest_chn.chn_id = vpss_chn;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(VI-VPSS)");

    return TD_SUCCESS;
}

td_s32 sample_aiisp_common_mcf_unbind_vpss(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn, ot_vpss_grp vpss_grp,
    ot_vpss_chn vpss_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_MCF;
    src_chn.dev_id = mcf_grp;
    src_chn.chn_id = mcf_chn;

    dest_chn.mod_id = OT_ID_VPSS;
    dest_chn.chn_id = vpss_chn;
    dest_chn.dev_id = vpss_grp;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(VI-VPSS)");

    return TD_SUCCESS;
}

td_void sample_aiisp_mcf_get_default_grp_attr(ot_mcf_grp_attr *grp_attr, ot_size *mono_size,
    ot_size *color_size)
{
    grp_attr->sync_pipe_en              = TD_TRUE;
    grp_attr->mono_pipe_attr.pipe_id    = 0;
    if ((mono_size->width >= 3840) && (mono_size->height >= 2160)) { /* 3840:width, 2160: height */
        grp_attr->mono_pipe_attr.vpss_grp   = 10; /* vgs group num 10 */
    } else {
        grp_attr->mono_pipe_attr.vpss_grp   = 0;
    }
    grp_attr->mono_pipe_attr.width      = mono_size->width;
    grp_attr->mono_pipe_attr.height     = mono_size->height;

    grp_attr->color_pipe_attr.pipe_id    = 1;
    grp_attr->color_pipe_attr.vpss_grp   = 1;
    grp_attr->color_pipe_attr.width      = color_size->width;
    grp_attr->color_pipe_attr.height     = color_size->height;

    grp_attr->frame_rate.src_frame_rate = -1;
    grp_attr->frame_rate.dst_frame_rate = -1;

    grp_attr->depth = 0;
    grp_attr->mcf_path = OT_MCF_PATH_FUSION;
}

static td_void sample_common_mcf_get_default_chn_attr(ot_mcf_chn_attr *chn_attr, ot_size *out_size)
{
    chn_attr->mirror_en                 = TD_FALSE;
    chn_attr->flip_en                   = TD_FALSE;
    chn_attr->width                     = out_size->width;
    chn_attr->height                    = out_size->height;
    chn_attr->depth                     = 0;
    chn_attr->video_format              = OT_VIDEO_FORMAT_LINEAR;
    chn_attr->pixel_format              = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    chn_attr->compress_mode             = OT_COMPRESS_MODE_NONE;
    chn_attr->frame_rate.src_frame_rate = -1;
    chn_attr->frame_rate.dst_frame_rate = -1;
}

static td_void sample_common_mcf_set_default_vpss_attr(ot_vpss_grp_attr *vpss_grp_attr,
    const ot_mcf_grp_attr *mcf_grp_attr, td_bool is_mono_pipe)
{
    td_u32 mono_pipe_width, mono_pipe_height, color_pipe_width, color_pipe_height;

    mono_pipe_width = mcf_grp_attr->mono_pipe_attr.width;
    mono_pipe_height = mcf_grp_attr->mono_pipe_attr.height;
    color_pipe_width = mcf_grp_attr->color_pipe_attr.width;
    color_pipe_height = mcf_grp_attr->color_pipe_attr.height;

    (td_void)memset_s(vpss_grp_attr, sizeof(ot_vpss_grp_attr), 0, sizeof(ot_vpss_grp_attr));
    vpss_grp_attr->ie_en                     = TD_FALSE;
    vpss_grp_attr->dci_en                    = TD_FALSE;
    vpss_grp_attr->buf_share_en              = TD_FALSE;
    vpss_grp_attr->mcf_en                    = TD_TRUE;
    vpss_grp_attr->max_dei_width             = 0;
    vpss_grp_attr->max_dei_height            = 0;
    vpss_grp_attr->dynamic_range             = OT_DYNAMIC_RANGE_SDR8;
    vpss_grp_attr->dei_mode                  = OT_VPSS_DEI_MODE_OFF;
    vpss_grp_attr->buf_share_chn             = OT_VPSS_CHN0;
    vpss_grp_attr->frame_rate.src_frame_rate = -1;
    vpss_grp_attr->frame_rate.dst_frame_rate = -1;

    if (is_mono_pipe == TD_TRUE) {
        vpss_grp_attr->max_width    = mono_pipe_width;
        vpss_grp_attr->max_height   = mono_pipe_height;
        vpss_grp_attr->pixel_format = OT_PIXEL_FORMAT_YUV_400;
    } else {
        vpss_grp_attr->max_width    = color_pipe_width;
        vpss_grp_attr->max_height   = color_pipe_height;
        vpss_grp_attr->pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    }
}

td_s32 sample_aiisp_common_mcf_start_vpss(const ot_mcf_grp_attr *mcf_grp_attr)
{
    ot_vpss_grp vpss_grp[OT_MCF_PIPE_NUM];
    ot_vpss_grp_attr vpss_grp_attr[OT_MCF_PIPE_NUM];
    td_s32 i, j, k;
    td_s32 ret;
    ot_vpss_grp_attr *grp_attr = TD_NULL;
    grp_attr = &vpss_grp_attr[0];
    sample_common_mcf_set_default_vpss_attr(grp_attr, mcf_grp_attr, TD_TRUE);
    grp_attr = &vpss_grp_attr[1];
    sample_common_mcf_set_default_vpss_attr(grp_attr, mcf_grp_attr, TD_FALSE);

    vpss_grp[0] = mcf_grp_attr->mono_pipe_attr.vpss_grp;
    vpss_grp[1] = mcf_grp_attr->color_pipe_attr.vpss_grp;

    for (i = 0; i < OT_MCF_PIPE_NUM; i++) {
        ret = ss_mpi_vpss_create_grp(vpss_grp[i], &vpss_grp_attr[i]);
        if (ret != TD_SUCCESS) {
            sample_print("VPSS Create Grp %d,fail! 0x%x\n", vpss_grp[i], ret);
            goto destroy_grp;
        }
    }

    for (k = 0; k < OT_MCF_PIPE_NUM; k++) {
        ret =  ss_mpi_vpss_start_grp(vpss_grp[k]);
        if (ret != TD_SUCCESS) {
            sample_print("VPSS Set Start Grp %d fail! 0x%x\n", vpss_grp[k], ret);
            goto stop_grp;
        }
    }

    return TD_SUCCESS;
stop_grp:
    for (j = k; j > 0; j--) {
        ret = ss_mpi_vpss_stop_grp(j);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_mcf_destroy_grp failed with %#x!\n", ret);
        }
    }

destroy_grp:
    for (j = i; j > 0; j--) {
        ret = ss_mpi_vpss_destroy_grp(j);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_mcf_destroy_grp failed with %#x!\n", ret);
        }
    }
    return TD_FAILURE;
}

td_s32 sample_aiisp_common_mcf_stop_vpss(const ot_mcf_grp_attr *grp_attr)
{
    ot_vpss_grp grp[OT_MCF_PIPE_NUM];
    td_s32 i;
    td_s32 ret;
    grp[0] = grp_attr->mono_pipe_attr.vpss_grp;
    grp[1] = grp_attr->color_pipe_attr.vpss_grp;

    for (i = 0; i < OT_MCF_PIPE_NUM; i++) {
        ret =  ss_mpi_vpss_stop_grp(grp[i]);
        if (ret != TD_SUCCESS) {
            sample_print("VPSS Set stop Grp %d fail! 0x%x\n", grp[i], ret);
        }
    }

    for (i = 0; i < OT_MCF_PIPE_NUM; i++) {
        ret = ss_mpi_vpss_destroy_grp(grp[i]);
        if (ret != TD_SUCCESS) {
            sample_print("VPSS Destroy Grp %d fail! 0x%x\n", grp[i], ret);
        }
    }

    return TD_SUCCESS;
}

static td_s32 sample_common_mcf_enable_chn(ot_mcf_grp grp, td_bool *mcf_chn_enable,
    ot_size *chn_out_size)
{
    td_s32 i, ret;
    ot_mcf_chn mcf_chn;
    ot_mcf_chn_attr chn_attr = {0};
    for (i = 0; i < OT_MCF_MAX_PHYS_CHN_NUM; ++i) {
        if (mcf_chn_enable[i] == TD_TRUE) {
            mcf_chn = i;
            sample_common_mcf_get_default_chn_attr(&chn_attr, &chn_out_size[mcf_chn]);
            ret = ss_mpi_mcf_set_chn_attr(grp, mcf_chn, &chn_attr);
            if (ret != TD_SUCCESS) {
                sample_print("ss_mpi_mcf_set_chn_attr failed with %#x\n", ret);
                goto disable_chn;
            }

            ret = ss_mpi_mcf_enable_chn(grp, mcf_chn);
            if (ret != TD_SUCCESS) {
                sample_print("ss_mpi_mcf_enable_chn failed with %#x\n", ret);
                goto disable_chn;
            }
        }
    }
    return TD_SUCCESS;

disable_chn:
    i--;
    for (; i >= 0; i--) {
        if (mcf_chn_enable[i] == TD_TRUE) {
            mcf_chn = i;
            ret = ss_mpi_mcf_disable_chn(grp, mcf_chn);
            if (ret != TD_SUCCESS) {
                sample_print("ss_mpi_mcf_disable_chn failed with %#x!\n", ret);
            }
        }
    }
    return TD_FAILURE;
}

static td_s32 sample_aiisp_common_mcf_start(ot_mcf_grp grp, const ot_mcf_grp_attr *grp_attr,
    ot_mcf_crop_info *grp_crop_info, td_bool *mcf_chn_en, ot_size *chn_out_size)
{
    td_s32 ret;

    ret = ss_mpi_mcf_create_grp(grp, grp_attr);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_mcf_create_grp(grp:%d) failed with %#x!\n", grp, ret);
        return TD_FAILURE;
    }
    if (grp_crop_info != TD_NULL) {
        ret = ss_mpi_mcf_set_grp_crop(grp, grp_crop_info);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_mcf_set_grp_crop failed with %#x\n", ret);
            goto destroy_grp;
        }
    }
    ret = ss_mpi_mcf_start_grp(grp);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_mcf_start_grp failed with %#x\n", ret);
        goto destroy_grp;
    }

    ret = sample_common_mcf_enable_chn(grp, mcf_chn_en, chn_out_size);
    if (ret != TD_SUCCESS) {
        sample_print("sample_common_mcf_enable_chn failed with %#x\n", ret);
        goto stop_grp;
    }
    return TD_SUCCESS;

stop_grp:
    ret = ss_mpi_mcf_stop_grp(grp);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_mcf_stop_grp failed with %#x\n", ret);
    }

destroy_grp:
    ret = ss_mpi_mcf_destroy_grp(grp);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_mcf_destroy_grp failed with %#x!\n", ret);
    }

    return TD_FAILURE;
}

td_s32 sample_aiisp_common_mcf_stop(ot_mcf_grp grp, const td_bool *chn_enable, td_u32 chn_array_size)
{
    td_s32 i;
    td_s32 ret;
    ot_mcf_chn mcf_chn;

    if (chn_array_size < OT_MCF_MAX_PHYS_CHN_NUM) {
        sample_print("array size(%u) of chn_enable need > %u!\n", chn_array_size, OT_MCF_MAX_PHYS_CHN_NUM);
        return TD_FAILURE;
    }

    for (i = 0; i < OT_MCF_MAX_PHYS_CHN_NUM; ++i) {
        if (chn_enable[i] == TD_TRUE) {
            mcf_chn = i;
            ret = ss_mpi_mcf_disable_chn(grp, mcf_chn);
            if (ret != TD_SUCCESS) {
                sample_print("ss_mpi_mcf_disable_chn failed with %#x!\n", ret);
            }
        }
    }
    ret = ss_mpi_mcf_stop_grp(grp);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_mcf_stop_grp failed with %#x!\n", ret);
    }
    ret = ss_mpi_mcf_destroy_grp(grp);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_mcf_destroy_grp failed with %#x!\n", ret);
    }
    return TD_SUCCESS;
}

td_void sample_aiisp_mcf_get_large_size(ot_size mono_size, ot_size color_size, ot_size *large_size)
{
    if (mono_size.width > color_size.width) {
        large_size->width = mono_size.width;
        large_size->height = mono_size.height;
    } else {
        large_size->width = color_size.width;
        large_size->height = color_size.height;
    }
}

td_void sample_aiisp_mcf_start_mcf_vi_cfg(ot_mcf_grp mcf_grp, const ot_mcf_grp_attr *mcf_grp_attr)
{
    ot_mcf_vi_attr mcf_vi_attr;
    td_s32 ret;
    mcf_vi_attr.enable     = TD_TRUE;
    mcf_vi_attr.mono_pipe  = mcf_grp_attr->mono_pipe_attr.pipe_id;
    mcf_vi_attr.color_pipe = mcf_grp_attr->color_pipe_attr.pipe_id;

    ret = ss_mpi_mcf_set_vi_attr(mcf_grp, &mcf_vi_attr);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_mcf_set_vi_attr error\n");
    }
}

td_void sample_aiisp_mcf_stop_mcf_vi_cfg(ot_mcf_grp mcf_grp)
{
    ot_mcf_vi_attr mcf_vi_attr;
    td_s32 ret;
    ret = ss_mpi_mcf_get_vi_attr(mcf_grp, &mcf_vi_attr);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_mcf_get_vi_attr err\n");
        return;
    }
    mcf_vi_attr.enable = TD_FALSE;
    ret = ss_mpi_mcf_set_vi_attr(mcf_grp, &mcf_vi_attr);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_mcf_set_vi_attr error\n");
    }
}

static td_s32 sample_aiisp_mcf_start_vpss(ot_vpss_grp grp, ot_size *in_size)
{
    ot_vpss_grp_attr grp_attr = {0};
    sample_vpss_chn_attr vpss_chn_attr = {0};
    ot_vpss_chn_attr *attr = TD_NULL;
    sample_comm_vpss_get_default_grp_attr(&grp_attr);
    grp_attr.max_width  = in_size->width;
    grp_attr.max_height = in_size->height;
    grp_attr.mcf_en     = TD_FALSE;
    attr = &vpss_chn_attr.chn_attr[0];
    sample_comm_vpss_get_default_chn_attr(attr);
    vpss_chn_attr.chn_attr[0].width  = in_size->width;
    vpss_chn_attr.chn_attr[0].height = in_size->height;
    attr = &vpss_chn_attr.chn_attr[1];
    sample_comm_vpss_get_default_chn_attr(attr);
    vpss_chn_attr.chn_attr[1].width  = MCF_VO_WIDHT_1920;
    vpss_chn_attr.chn_attr[1].height = MCF_VO_HEIGHT_1080;
    vpss_chn_attr.chn_attr[1].compress_mode = OT_COMPRESS_MODE_NONE;
    memcpy_s(vpss_chn_attr.chn_enable, sizeof(vpss_chn_attr.chn_enable),
        g_vpss_chn_enable, sizeof(vpss_chn_attr.chn_enable));
    vpss_chn_attr.chn_array_size = OT_VPSS_MAX_PHYS_CHN_NUM;
    return sample_common_vpss_start(grp, &grp_attr, &vpss_chn_attr);
}

td_void sample_aiisp_mcf_stop_vpss(ot_vpss_grp grp)
{
    sample_common_vpss_stop(grp, g_vpss_chn_enable, OT_VPSS_MAX_PHYS_CHN_NUM);
}

td_s32 sample_aiisp_mcf_start_vpss_venc_and_vo(ot_size *out_size)
{
    td_s32 ret;
    const sample_vo_mode vo_mode = VO_MODE_1MUX;
    ot_venc_chn venc_chn[3] = {0, 1, 2}; /* 3: max chn num, 0/1/2 chn id */
    const ot_vpss_grp vpss_grp = MCF_BIND_VPSS_GRP;
    const td_s32 grp_num = 1;
    ret = sample_aiisp_mcf_start_vpss(vpss_grp, out_size);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    ret = sample_aiisp_start_vo(vo_mode);
    if (ret != TD_SUCCESS) {
        goto start_vo_failed;
    }

    ret = sample_aiisp_start_venc(venc_chn, grp_num, 1, out_size); /* 3: max chn num, 0/1/2 chn id */
    if (ret != TD_SUCCESS) {
        goto start_venc_failed;
    }
    return TD_SUCCESS;

start_vo_failed:
    sample_aiisp_mcf_stop_vpss(vpss_grp);

start_venc_failed:
    sample_aiisp_stop_vo();
    return TD_FAILURE;
}

td_void sample_aiisp_mcf_stop_vpss_venc_and_vo(td_void)
{
    const td_s32 grp_num = 1;
    ot_venc_chn venc_chn[3] = {0, 1, 2}; /* 3: chn num, 0/1/2 chn id */
    const ot_vpss_grp vpss_grp = MCF_BIND_VPSS_GRP;
    sample_aiisp_stop_venc(venc_chn, grp_num, 1); /* 3: chn num, 0/1/2 chn id */
    sample_aiisp_stop_vo();
    sample_aiisp_mcf_stop_vpss(vpss_grp);
}

/* bind */
td_void sample_aiisp_mcf_bind_vi(ot_mcf_grp mcf_grp, const ot_mcf_grp_attr *mcf_grp_attr, ot_vi_chn vi_chn)
{
    ot_mcf_pipe mcf_color_pipe, mcf_mono_pipe;
    ot_vi_pipe vi_mono_pipe, vi_color_pipe;

    vi_mono_pipe = mcf_grp_attr->mono_pipe_attr.pipe_id;
    vi_color_pipe = mcf_grp_attr->color_pipe_attr.pipe_id;

    mcf_mono_pipe = mcf_grp_attr->mono_pipe_attr.pipe_id;
    mcf_color_pipe = mcf_grp_attr->color_pipe_attr.pipe_id;

    sample_aiisp_common_vi_bind_mcf(vi_mono_pipe, vi_chn, mcf_grp, mcf_mono_pipe);
    sample_aiisp_common_vi_bind_mcf(vi_color_pipe, vi_chn, mcf_grp, mcf_color_pipe);
}

td_void sample_aiisp_mcf_unbind_vi(ot_mcf_grp mcf_grp, const ot_mcf_grp_attr *mcf_grp_attr, ot_vi_chn vi_chn)
{
    ot_mcf_pipe mcf_color_pipe, mcf_mono_pipe;
    ot_vi_pipe vi_mono_pipe, vi_color_pipe;

    vi_mono_pipe = mcf_grp_attr->mono_pipe_attr.pipe_id;
    vi_color_pipe = mcf_grp_attr->color_pipe_attr.pipe_id;

    mcf_mono_pipe = mcf_grp_attr->mono_pipe_attr.pipe_id;
    mcf_color_pipe = mcf_grp_attr->color_pipe_attr.pipe_id;

    sample_aiisp_common_vi_unbind_mcf(vi_mono_pipe, vi_chn, mcf_grp, mcf_mono_pipe);
    sample_aiisp_common_vi_unbind_mcf(vi_color_pipe, vi_chn, mcf_grp, mcf_color_pipe);
}

td_void sample_aiisp_mcf_set_chn_defualt_outsize(ot_size out_size)
{
    td_s32 i;
    for (i = 0; i < OT_MCF_MAX_PHYS_CHN_NUM; i++) {
        (td_void)memcpy_s(&g_mcf_chn_out_size[i], sizeof(ot_size), &out_size, sizeof(ot_size));
    }
}

static td_void sample_aiisp_mcf_get_vi_config(sample_sns_type sns_type, sample_vi_cfg *vi_cfg, ot_vi_pipe pipe_id)
{
    ot_vi_bind_pipe *bind_pipe = TD_NULL;
    sample_vi_grp_info  *grp_info = TD_NULL;

    (td_void)memset_s(vi_cfg, sizeof(sample_vi_cfg), 0, sizeof(sample_vi_cfg));
    /* sensor info */
    sample_comm_vi_get_default_sns_info(sns_type, &vi_cfg->sns_info);
    /* mipi info */
    sample_comm_vi_get_default_mipi_info(sns_type, &vi_cfg->mipi_info);
    /* dev info */
    sample_comm_vi_get_default_dev_info(sns_type, &vi_cfg->dev_info);

    /* bind info */
    bind_pipe = &vi_cfg->bind_pipe;
    bind_pipe->pipe_num = 1;
    bind_pipe->pipe_id[0] = pipe_id;

    /* grp info */
    grp_info = &vi_cfg->grp_info;
    grp_info->grp_num = 1;
    grp_info->fusion_grp[0] = 0;
    grp_info->fusion_grp[1] = 1;
    grp_info->fusion_grp_attr[0].wdr_mode = OT_WDR_MODE_NONE;
    grp_info->fusion_grp_attr[0].pipe_id[0] = pipe_id;
    grp_info->fusion_grp_attr[0].cache_line = 100; /* 100 line value */

    sample_comm_vi_get_default_pipe_info(sns_type, &vi_cfg->bind_pipe, vi_cfg->pipe_info);
    vi_cfg->pipe_info->chn_info[0].chn_attr.depth = 1;
    vi_cfg->pipe_info[0].bnr_bnf_num = 3;  /* 3: ai3dnr should set bnr buf_num 3 */
}

static td_s32 sample_aiisp_mcf_start_vi0(const ot_mcf_grp_attr *mcf_grp_attr, sample_vi_cfg *vi_cfg0)
{
    td_s32 ret;
    ot_vi_pipe vi_mono_pipe;
    vi_mono_pipe = mcf_grp_attr->mono_pipe_attr.pipe_id;
    /* start mono pipe */
    sample_aiisp_mcf_get_vi_config(SENSOR0_TYPE, vi_cfg0, vi_mono_pipe);
    vi_cfg0->mipi_info.divide_mode = LANE_DIVIDE_MODE_1;
    vi_cfg0->dev_info.vi_dev = 0;
    vi_cfg0->pipe_info->chn_info[0].chn_attr.pixel_format = OT_PIXEL_FORMAT_YUV_400;
    vi_cfg0->pipe_info[0].chn_info[0].chn_attr.compress_mode = OT_COMPRESS_MODE_NONE;
#ifdef OT_FPGA
    vi_cfg0->pipe_info[0].isp_info.isp_pub_attr.frame_rate = 5; /* 5 fpga */
#endif
    vi_cfg0->pipe_info[0].bnr_bnf_num = 0;
    ret = sample_comm_vi_start_vi(vi_cfg0);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 sample_aiisp_mcf_attach_vb(ot_vi_pipe vi_pipe, ot_size *in_size)
{
    td_s32 ret;
    td_s32 blk_size;
    ot_vb_pool vb_pool;
    ot_aiisp_pool pool_attr = {0};
    ot_vb_pool_cfg vb_pool_cfg = {0};

    blk_size = ot_ai3dnr_get_pic_buf_size(in_size->width, in_size->height);
    vb_pool_cfg.blk_size = blk_size;
    vb_pool_cfg.blk_cnt = 16; /* 16: vb cnt */
    vb_pool_cfg.remap_mode = OT_VB_REMAP_MODE_NONE;
    vb_pool = ss_mpi_vb_create_pool(&vb_pool_cfg);
    if (vb_pool == OT_VB_INVALID_POOL_ID) {
        sample_print("create user pool failed!\n");
        return TD_FAILURE;
    }

#ifdef SAMPLE_MEM_SHARE_ENABLE
    ss_mpi_vb_pool_share_all(vb_pool);
#endif

    pool_attr.aiisp_type = OT_AIISP_TYPE_AI3DNR;
    pool_attr.ai3dnr_pool.vb_pool = vb_pool;
    pool_attr.ai3dnr_pool.net_type = OT_AI3DNR_NET_TYPE_UV;

    ret = ss_mpi_vi_attach_aiisp_vb_pool(vi_pipe, &pool_attr);
    if (ret != TD_SUCCESS) {
        sample_print("vi aiisp attach vb pool failed.\n");
        return ret;
    }

    g_aiisp_mcf_vb_pool = vb_pool;

    return TD_SUCCESS;
}

static td_s32 sample_aiisp_mcf_detach_vb(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    ret = ss_mpi_vi_detach_aiisp_vb_pool(vi_pipe, OT_AIISP_TYPE_AI3DNR);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_vi_detach_aiisp_vb_pool failed.\n");
    }

    if (g_aiisp_mcf_vb_pool != OT_VB_INVALID_POOL_ID) {
        ss_mpi_vb_destroy_pool(g_aiisp_mcf_vb_pool);
        g_aiisp_mcf_vb_pool = OT_VB_INVALID_POOL_ID;
    }

    return TD_SUCCESS;
}

static td_s32 sample_aiisp_mcf_start_vi1(const ot_mcf_grp_attr *mcf_grp_attr, sample_vi_cfg *vi_cfg1, ot_size *in_size)
{
    td_s32 ret;
    ot_vi_pipe vi_color_pipe;
    const ot_vi_dev vi_dev = 2; /* dev 2 for sensor1 */

    vi_color_pipe = mcf_grp_attr->color_pipe_attr.pipe_id;
    /* start color pipe */
    sample_aiisp_mcf_get_vi_config(SENSOR1_TYPE, vi_cfg1, vi_color_pipe);
#ifdef OT_FPGA
    vi_cfg1->sns_info.bus_id = 1; /* i2c1 */
    vi_cfg1->pipe_info[0].isp_info.isp_pub_attr.frame_rate = 5; /* 5 fpga */
#else
    vi_cfg1->sns_info.bus_id = 5; /* i2c5 */
#endif

    vi_cfg1->sns_info.sns_clk_src = 1;
    vi_cfg1->sns_info.sns_rst_src = 1;
    sample_comm_vi_get_mipi_info_by_dev_id(SENSOR1_TYPE, vi_dev, &vi_cfg1->mipi_info);
    vi_cfg1->mipi_info.divide_mode = LANE_DIVIDE_MODE_1;
    vi_cfg1->dev_info.vi_dev = vi_dev;
    vi_cfg1->pipe_info[0].chn_info[0].chn_attr.compress_mode = OT_COMPRESS_MODE_NONE;

    ret = sample_comm_vi_start_vi(vi_cfg1);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    ret = sample_aiisp_mcf_attach_vb(vi_color_pipe, in_size);
    if (ret != TD_SUCCESS) {
        sample_print("sample_aiisp_mcf_attach_vb failed.\n");
        goto stop_vi;
    }

    return TD_SUCCESS;

stop_vi:
    sample_comm_vi_stop_vi(vi_cfg1);
    return TD_FAILURE;
}

td_s32 sample_aiisp_mcf_start_vi(const ot_mcf_grp_attr *mcf_grp_attr, sample_vi_cfg *vi_cfg1,
    sample_vi_cfg *vi_cfg0, ot_size *in_size)
{
    td_s32 ret;
    ret = sample_aiisp_mcf_start_vi0(mcf_grp_attr, vi_cfg0);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ret = sample_aiisp_mcf_start_vi1(mcf_grp_attr, vi_cfg1, in_size);
    if (ret != TD_SUCCESS) {
        sample_comm_vi_stop_vi(vi_cfg0);
        return ret;
    }

    return TD_SUCCESS;
}

td_void sample_aiisp_mcf_stop_vi(ot_vi_pipe vi_color_pipe, sample_vi_cfg *vi_cfg1, sample_vi_cfg *vi_cfg0)
{
    sample_comm_vi_stop_vi(vi_cfg0);
    sample_aiisp_mcf_detach_vb(vi_color_pipe);
    sample_comm_vi_stop_vi(vi_cfg1);
}

static td_void sample_aiisp_mcf_bind(ot_mcf_grp mcf_grp, const ot_mcf_grp_attr *mcf_grp_attr)
{
    const ot_vi_chn vi_chn = 0;
    const ot_mcf_chn mcf_chn = 0;
    const ot_vpss_grp vpss_grp = MCF_BIND_VPSS_GRP;
    ot_vpss_chn vpss_chn[2] = {0, 1};
    const ot_vo_layer vo_layer = 0;
    const ot_vo_chn vo_chn = 0;
    const ot_venc_chn venc_chn = 0;

    sample_aiisp_mcf_bind_vi(mcf_grp, mcf_grp_attr, vi_chn);
    sample_aiisp_common_mcf_bind_vpss(mcf_grp, mcf_chn, vpss_grp, vpss_chn[0]);
    sample_comm_vpss_bind_venc(vpss_grp, vpss_chn[0], venc_chn);
    sample_comm_vpss_bind_vo(vpss_grp, vpss_chn[1], vo_layer, vo_chn);
}

static td_void sample_aiisp_mcf_unbind(ot_mcf_grp mcf_grp, const ot_mcf_grp_attr *mcf_grp_attr)
{
    const ot_vi_chn vi_chn = 0;
    const ot_mcf_chn mcf_chn = 0;
    const ot_vpss_grp vpss_grp = MCF_BIND_VPSS_GRP;
    ot_vpss_chn vpss_chn[2] = {0, 1};
    const ot_vo_layer vo_layer = 0;
    const ot_vo_chn vo_chn = 0;
    const ot_venc_chn venc_chn = 0;

    sample_comm_vpss_un_bind_venc(vpss_grp, vpss_chn[0], venc_chn);
    sample_comm_vpss_un_bind_vo(vpss_grp, vpss_chn[1], vo_layer, vo_chn);
    sample_aiisp_common_mcf_unbind_vpss(mcf_grp, mcf_chn, vpss_grp, vpss_chn[0]);
    sample_aiisp_mcf_unbind_vi(mcf_grp, mcf_grp_attr, vi_chn);
}

td_s32 sample_aiisp_start_mcf_data_flow(ot_mcf_grp mcf_grp, ot_mcf_grp_attr *mcf_grp_attr,
    ot_size *out_size, ot_mcf_crop_info *grp_crop_info)
{
    td_s32 ret;

    ret = sample_aiisp_common_mcf_start_vpss(mcf_grp_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = sample_aiisp_common_mcf_start(mcf_grp, mcf_grp_attr, grp_crop_info,
        &g_mcf_chn_enable[0], &g_mcf_chn_out_size[0]);
    if (ret != TD_SUCCESS) {
        goto start_mcf_failed;
    }

    ret = sample_aiisp_mcf_start_vpss_venc_and_vo(out_size);
    if (ret != TD_SUCCESS) {
        goto start_vpss_venc_and_vo_failed;
    }

    sample_aiisp_mcf_bind(mcf_grp, mcf_grp_attr);

    sample_aiisp_mcf_start_mcf_vi_cfg(mcf_grp, mcf_grp_attr);

    return ret;

start_vpss_venc_and_vo_failed:
    sample_aiisp_common_mcf_stop(mcf_grp, g_mcf_chn_enable, OT_MCF_MAX_PHYS_CHN_NUM);
start_mcf_failed:
    sample_aiisp_common_mcf_stop_vpss(mcf_grp_attr);

    return ret;
}

td_void sample_aiisp_stop_mcf_data_flow(ot_mcf_grp mcf_grp, ot_mcf_grp_attr *mcf_grp_attr)
{
    sample_aiisp_mcf_stop_mcf_vi_cfg(mcf_grp);
    sample_aiisp_mcf_unbind(mcf_grp, mcf_grp_attr);
    sample_aiisp_mcf_stop_vpss_venc_and_vo();
    sample_aiisp_common_mcf_stop(mcf_grp, g_mcf_chn_enable, OT_MCF_MAX_PHYS_CHN_NUM);
    sample_aiisp_common_mcf_stop_vpss(mcf_grp_attr);
}

