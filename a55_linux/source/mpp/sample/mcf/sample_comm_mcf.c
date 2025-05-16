/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include "sample_comm_mcf.h"
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

ot_vb_pool g_pool_id_color;
ot_vb_pool g_pool_id_mono;

td_s32 sample_common_vi_bind_mcf(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, ot_mcf_grp mcf_grp, ot_mcf_pipe mcf_pipe)
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

td_s32 sample_common_vi_unbind_mcf(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, ot_mcf_grp mcf_grp, ot_mcf_pipe mcf_pipe)
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

td_s32 sample_common_mcf_bind_vpss(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn, ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn)
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

td_s32 sample_common_mcf_unbind_vpss(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn,
    ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_MCF;
    src_chn.dev_id = mcf_grp;
    src_chn.chn_id = mcf_chn;

    dest_chn.mod_id = OT_ID_VPSS;
    dest_chn.dev_id = vpss_grp;
    dest_chn.chn_id = vpss_chn;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(VI-VPSS)");

    return TD_SUCCESS;
}
td_s32 sample_common_mcf_bind_vo(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn, ot_vo_layer vo_layer, ot_vo_chn vo_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_MCF;
    src_chn.dev_id = mcf_grp;
    src_chn.chn_id = mcf_chn;

    dest_chn.mod_id = OT_ID_VO;
    dest_chn.dev_id = vo_layer;
    dest_chn.chn_id = vo_chn;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(MCF-VO)");

    return TD_SUCCESS;
}

td_s32 sample_common_mcf_unbind_vo(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn, ot_vo_layer vo_layer, ot_vo_chn vo_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_MCF;
    src_chn.dev_id = mcf_grp;
    src_chn.chn_id = mcf_chn;

    dest_chn.mod_id = OT_ID_VO;
    dest_chn.dev_id = vo_layer;
    dest_chn.chn_id = vo_chn;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(MCF-VO)");

    return TD_SUCCESS;
}

td_s32 sample_common_mcf_bind_venc(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn, ot_venc_chn venc_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_MCF;
    src_chn.dev_id = mcf_grp;
    src_chn.chn_id = mcf_chn;

    dest_chn.mod_id = OT_ID_VENC;
    dest_chn.dev_id = 0;
    dest_chn.chn_id = venc_chn;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(MCF-VENC)");

    return TD_SUCCESS;
}

td_s32 sample_common_mcf_unbind_venc(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn, ot_venc_chn venc_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_MCF;
    src_chn.dev_id = mcf_grp;
    src_chn.chn_id = mcf_chn;

    dest_chn.mod_id = OT_ID_VENC;
    dest_chn.dev_id = 0;
    dest_chn.chn_id = venc_chn;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(MCF-VENC)");

    return TD_SUCCESS;
}


td_void sample_common_mcf_get_default_grp_attr(ot_mcf_grp_attr *grp_attr, ot_size *mono_size, ot_size *color_size)
{
    grp_attr->sync_pipe_en = TD_TRUE;
    grp_attr->mono_pipe_attr.pipe_id    = 0;
    grp_attr->mono_pipe_attr.vpss_grp   = 0;
    grp_attr->mono_pipe_attr.width      = mono_size->width;
    grp_attr->mono_pipe_attr.height     = mono_size->height;

    grp_attr->color_pipe_attr.pipe_id    = 1;
    if ((color_size->width >= MCF_VPSS_MAX_WIDTH) && (color_size->height >= MCF_VPSS_MAX_HEIGHT)) {
        grp_attr->color_pipe_attr.vpss_grp   = OT_VPSS_VGS_GRP_NO;      // vgs grp: 10
    } else {
        grp_attr->color_pipe_attr.vpss_grp   = 1;
    }
    grp_attr->color_pipe_attr.width      = color_size->width;
    grp_attr->color_pipe_attr.height     = color_size->height;

    grp_attr->frame_rate.src_frame_rate = -1;
    grp_attr->frame_rate.dst_frame_rate = -1;

    grp_attr->depth = 0;
    grp_attr->mcf_path = OT_MCF_PATH_FUSION;
}

td_void sample_common_mcf_get_default_chn_attr(ot_mcf_chn_attr *chn_attr, ot_size *out_size)
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

td_void sample_common_mcf_set_default_vpss_attr(ot_vpss_grp_attr *vpss_grp_attr,
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

static td_void sample_common_mcf_get_color_vpss_vb_config(ot_size *size, ot_vb_pool_cfg *vb_pool_cfg, td_u32 yuv_cnt)
{
    vb_pool_cfg->blk_size = ot_vpss_get_mcf_color_buf_size(size->width, size->height);
    vb_pool_cfg->blk_cnt  = yuv_cnt;
    vb_pool_cfg->remap_mode = OT_VB_REMAP_MODE_NONE;
}

static td_void sample_common_mcf_get_mono_vpss_vb_config(ot_size *size, ot_vb_pool_cfg *vb_pool_cfg, td_u32 yuv_cnt)
{
    vb_pool_cfg->blk_size = ot_vpss_get_mcf_color_buf_size(size->width, size->height);
    vb_pool_cfg->blk_cnt  = yuv_cnt;
    vb_pool_cfg->remap_mode = OT_VB_REMAP_MODE_NONE;
}

static td_void sample_common_mcf_vpss_destroy_user_vb(const ot_mcf_grp_attr *mcf_grp_attr)
{
    td_s32 ret;
    ot_vpss_grp vpss_grp[OT_MCF_PIPE_NUM];
    vpss_grp[0] = mcf_grp_attr->mono_pipe_attr.vpss_grp;
    vpss_grp[1] = mcf_grp_attr->color_pipe_attr.vpss_grp;

    ss_mpi_vpss_set_chn_vb_src(vpss_grp[0], 0, OT_VB_SRC_USER);
    ret = ss_mpi_vpss_detach_chn_vb_pool(vpss_grp[0], 0);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_vpss_detach_chn_vb_pool,grp(%d) fail!\n", vpss_grp[0]);
    }

    ss_mpi_vpss_set_chn_vb_src(vpss_grp[1], 0, OT_VB_SRC_USER);
    ret = ss_mpi_vpss_detach_chn_vb_pool(vpss_grp[1], 0);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_vpss_detach_chn_vb_pool,grp(%d) fail!\n", vpss_grp[1]);
    }

    ret = ss_mpi_vb_destroy_pool(g_pool_id_mono);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_vb_destroy_pool fail!\n");
    }
    g_pool_id_mono = OT_VB_INVALID_POOL_ID;

    ret = ss_mpi_vb_destroy_pool(g_pool_id_color);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_vb_destroy_pool fail!\n");
    }
    g_pool_id_color = OT_VB_INVALID_POOL_ID;
    return;
}

static td_void sample_common_mcf_get_large_size(const ot_mcf_grp_attr *mcf_grp_attr, ot_size *mcf_vpss_buf_size)
{
    if (mcf_grp_attr->mono_pipe_attr.width > mcf_grp_attr->color_pipe_attr.width) {
        mcf_vpss_buf_size->width = mcf_grp_attr->mono_pipe_attr.width;
        mcf_vpss_buf_size->height = mcf_grp_attr->mono_pipe_attr.height;
    } else {
        mcf_vpss_buf_size->width = mcf_grp_attr->color_pipe_attr.width;
        mcf_vpss_buf_size->height = mcf_grp_attr->color_pipe_attr.height;
    }
}

static td_s32 sample_common_mcf_vpss_set_user_vb(const ot_mcf_grp_attr *mcf_grp_attr, td_u32 color_cnt, td_u32 mono_cnt)
{
    ot_size mcf_vpss_buf_size;
    ot_vb_pool_cfg mcf_vpss_mono_vb_cfg = { 0 };
    ot_vb_pool_cfg mcf_vpss_color_vb_cfg = { 0 };
    ot_vpss_grp vpss_grp[OT_MCF_PIPE_NUM];
    vpss_grp[0] = mcf_grp_attr->mono_pipe_attr.vpss_grp;
    vpss_grp[1] = mcf_grp_attr->color_pipe_attr.vpss_grp;

    sample_common_mcf_get_large_size(mcf_grp_attr, &mcf_vpss_buf_size);

    sample_common_mcf_get_color_vpss_vb_config(&mcf_vpss_buf_size, &mcf_vpss_color_vb_cfg, color_cnt);
    sample_common_mcf_get_mono_vpss_vb_config(&mcf_vpss_buf_size, &mcf_vpss_mono_vb_cfg, mono_cnt);
    g_pool_id_color = ss_mpi_vb_create_pool(&mcf_vpss_color_vb_cfg);
    if (g_pool_id_color == OT_VB_INVALID_POOL_ID) {
        printf("create mcf color pre_process user vb error\n");
        return TD_FAILURE;
    }

    g_pool_id_mono = ss_mpi_vb_create_pool(&mcf_vpss_mono_vb_cfg);
    if (g_pool_id_mono == OT_VB_INVALID_POOL_ID) {
        printf("create mcf mono pre_process user vb error\n");
        goto release_color_user_vb;
    }

#ifdef SAMPLE_MEM_SHARE_ENABLE
    ss_mpi_vb_pool_share_all(g_pool_id_color);
    ss_mpi_vb_pool_share_all(g_pool_id_mono);
#endif

    ss_mpi_vpss_set_chn_vb_src(vpss_grp[0], 0, OT_VB_SRC_USER);
    if (ss_mpi_vpss_attach_chn_vb_pool(vpss_grp[0], 0, g_pool_id_mono) != TD_SUCCESS) {
        printf("create vpss(0) attach to user vb error\n");
        goto release_monor_user_vb;
    }

    ss_mpi_vpss_set_chn_vb_src(vpss_grp[1], 0, OT_VB_SRC_USER);
    if (ss_mpi_vpss_attach_chn_vb_pool(vpss_grp[1], 0, g_pool_id_color) != TD_SUCCESS) {
        printf("create vpss(1) attach to user vb error\n");
        goto release_monor_user_vb;
    }
    return TD_SUCCESS;

release_monor_user_vb:
    if (ss_mpi_vb_destroy_pool(g_pool_id_mono) != TD_SUCCESS) {
        printf("ss_mpi_vb_destroy_pool %u fail!\n", g_pool_id_mono);
    }
    g_pool_id_mono = OT_VB_INVALID_POOL_ID;
release_color_user_vb:
    if (ss_mpi_vb_destroy_pool(g_pool_id_color) != TD_SUCCESS) {
        printf("ss_mpi_vb_destroy_pool %u fail!\n", g_pool_id_color);
    }
    g_pool_id_color = OT_VB_INVALID_POOL_ID;
    return TD_FAILURE;
}

static td_void sample_common_mcf_get_gdc_param_zme(ot_gdc_param *gdc_param, ot_size *out_size,
    const ot_mcf_grp_attr *mcf_grp_attr, ot_vpss_grp *grp)
{
    td_s32 i;
    if (mcf_grp_attr->mono_pipe_attr.width > mcf_grp_attr->color_pipe_attr.width) {
        out_size->width = mcf_grp_attr->mono_pipe_attr.width;
        out_size->height = mcf_grp_attr->mono_pipe_attr.height;
    } else {
        out_size->width = mcf_grp_attr->color_pipe_attr.width;
        out_size->height = mcf_grp_attr->color_pipe_attr.height;
    }

    for (i = 0; i < OT_MCF_PIPE_NUM; i++) {
        if (grp[i] == mcf_grp_attr->mono_pipe_attr.vpss_grp) {
            gdc_param[i].in_size.height = mcf_grp_attr->mono_pipe_attr.height;
            gdc_param[i].in_size.width = mcf_grp_attr->mono_pipe_attr.width;
        } else {
            gdc_param[i].in_size.height = mcf_grp_attr->color_pipe_attr.height;
            gdc_param[i].in_size.width = mcf_grp_attr->color_pipe_attr.width;
        }
    }
}

static td_s32 sample_common_mcf_set_grp_zme(ot_vpss_grp grp, ot_size *out_size, const ot_mcf_grp_attr *mcf_grp_attr)
{
    td_s32 ret = TD_SUCCESS;

    // if same size, no zme
    if (mcf_grp_attr->mono_pipe_attr.height == mcf_grp_attr->color_pipe_attr.height &&
        mcf_grp_attr->mono_pipe_attr.width == mcf_grp_attr->color_pipe_attr.width) {
        return ret;
    }

    if (mcf_grp_attr->mono_pipe_attr.height * mcf_grp_attr->mono_pipe_attr.width >
        mcf_grp_attr->color_pipe_attr.height * mcf_grp_attr->color_pipe_attr.width) {
        if (grp == mcf_grp_attr->color_pipe_attr.vpss_grp) {
            ret = ss_mpi_vpss_set_grp_zme(grp, out_size);
            if (ret != TD_SUCCESS) {
                sample_print("VPSS set grp %d zme fail! 0x%x\n", grp, ret);
                return ret;
            }
        }
    } else {
        if (grp == mcf_grp_attr->mono_pipe_attr.vpss_grp) {
            ret = ss_mpi_vpss_set_grp_zme(grp, out_size);
            if (ret != TD_SUCCESS) {
                sample_print("VPSS set grp %d zme fail! 0x%x\n", grp, ret);
                return ret;
            }
        }
    }

    return ret;
}

static td_s32 sample_common_mcf_set_grp_fov(ot_vpss_grp grp, const ot_mcf_grp_attr *mcf_grp_attr,
    ot_fov_attr *fov_correction_attr)
{
    td_s32 ret = TD_SUCCESS;

    if (grp == mcf_grp_attr->mono_pipe_attr.vpss_grp && fov_correction_attr->enable == TD_TRUE) {
        ret = ss_mpi_vpss_set_grp_fov(grp, fov_correction_attr);
        if (ret != TD_SUCCESS) {
            sample_print("VPSS set grp %d fov fail! 0x%x\n", grp, ret);
            return ret;
        }
    }

    return ret;
}

td_s32 sample_common_mcf_stop_vpss_one_grp(ot_vpss_grp grp)
{
    td_s32 ret = TD_SUCCESS;

    ret = ss_mpi_vpss_stop_grp(grp);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_mcf_destroy_grp failed with %#x!\n", ret);
    }

    ret = ss_mpi_vpss_destroy_grp(grp);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_mcf_destroy_grp failed with %#x!\n", ret);
    }

    return ret;
}

static td_s32 sample_common_mcf_start_new_scale_vpss(ot_vpss_grp *grp, const ot_mcf_grp_attr *mcf_grp_attr,
    ot_vpss_grp_attr *grp_attr, ot_fov_attr *fov_correction_attr)
{
    td_s32 i, j;
    td_s32 ret = TD_SUCCESS;
    ot_gdc_param gdc_param[OT_MCF_PIPE_NUM];
    gdc_param[0].cell_size = OT_LUT_CELL_SIZE_16;
    gdc_param[1].cell_size = OT_LUT_CELL_SIZE_16;
    ot_size out_size;

    sample_common_mcf_get_gdc_param_zme(gdc_param, &out_size, mcf_grp_attr, grp);

    for (i = 0; i < OT_MCF_PIPE_NUM; i++) {
        ret = ss_mpi_vpss_create_grp(grp[i], &grp_attr[i]);
        if (ret != TD_SUCCESS) {
            sample_print("VPSS create grp %d fail! 0x%x\n", grp[i], ret);
            goto exit;
        }
        if ((mcf_grp_attr->color_pipe_attr.vpss_grp < OT_VPSS_VGS_GRP_NO &&
            grp[i] == mcf_grp_attr->color_pipe_attr.vpss_grp) ||
            (mcf_grp_attr->mono_pipe_attr.vpss_grp < OT_VPSS_VGS_GRP_NO &&
            grp[i] == mcf_grp_attr->mono_pipe_attr.vpss_grp)) {
            ret = ss_mpi_vpss_set_grp_gdc_param(grp[i], &gdc_param[i]);
            if (ret != TD_SUCCESS) {
                sample_print("VPSS set grp %d gdc param fail! 0x%x\n", grp[i], ret);
                return ret;
            }
        }

        ret = sample_common_mcf_set_grp_zme(grp[i], &out_size, mcf_grp_attr);
        if (ret != TD_SUCCESS) {
            sample_print("MCF set grp %d zme fail! 0x%x\n", grp[i], ret);
            goto exit;
        }

        ret = sample_common_mcf_set_grp_fov(grp[i], mcf_grp_attr, fov_correction_attr);
        if (ret != TD_SUCCESS) {
            sample_print("MCF set grp %d fov fail! 0x%x\n", grp[i], ret);
            goto exit;
        }

        ret = ss_mpi_vpss_start_grp(grp[i]);
        if (ret != TD_SUCCESS) {
            sample_print("VPSS start grp %d fail! 0x%x\n", grp[i], ret);
            goto exit;
        }
    }
    return ret;

exit:
    for (j = i; j > 0; j--) {
        sample_common_mcf_stop_vpss_one_grp(i);
    }
    return TD_FAILURE;
}

td_s32 sample_common_mcf_start_vpss(const ot_mcf_grp_attr *mcf_grp_attr, td_u32 color_cnt, td_u32 mono_cnt,
    ot_fov_attr *fov_correction_attr)
{
    ot_vpss_grp vpss_grp[OT_MCF_PIPE_NUM];
    ot_vpss_grp_attr vpss_grp_attr[OT_MCF_PIPE_NUM];
    td_s32 i;
    td_s32 ret;
    ot_vpss_grp_attr *grp_attr = TD_NULL;
    grp_attr = &vpss_grp_attr[0];
    sample_common_mcf_set_default_vpss_attr(grp_attr, mcf_grp_attr, TD_TRUE);
    grp_attr = &vpss_grp_attr[1];
    sample_common_mcf_set_default_vpss_attr(grp_attr, mcf_grp_attr, TD_FALSE);

    vpss_grp[0] = mcf_grp_attr->mono_pipe_attr.vpss_grp;
    vpss_grp[1] = mcf_grp_attr->color_pipe_attr.vpss_grp;

    ret = sample_common_mcf_start_new_scale_vpss(vpss_grp, mcf_grp_attr, vpss_grp_attr, fov_correction_attr);
    if (ret != TD_SUCCESS) {
        sample_print("New Scale VPSS Create Grp fail! 0x%x\n", ret);
        return ret;
    }

    ret = sample_common_mcf_vpss_set_user_vb(mcf_grp_attr, color_cnt, mono_cnt);
    if (ret != TD_SUCCESS) {
        sample_print("VPSS Set user vb fail!");
        goto stop_grp;
    }
    return TD_SUCCESS;

stop_grp:
    for (i = 0; i < OT_MCF_PIPE_NUM; i++) {
        sample_common_mcf_stop_vpss_one_grp(i);
    }
    return TD_FAILURE;
}

td_s32 sample_common_mcf_stop_vpss(const ot_mcf_grp_attr *grp_attr)
{
    ot_vpss_grp grp[OT_MCF_PIPE_NUM];
    td_s32 i;
    grp[0] = grp_attr->mono_pipe_attr.vpss_grp;
    grp[1] = grp_attr->color_pipe_attr.vpss_grp;
    sample_common_mcf_vpss_destroy_user_vb(grp_attr);

    for (i = 0; i < OT_MCF_PIPE_NUM; i++) {
        sample_common_mcf_stop_vpss_one_grp(grp[i]);
    }
    return TD_SUCCESS;
}

td_s32 sample_common_mcf_enable_chn(ot_mcf_grp grp, td_bool *mcf_chn_enable,
                                    ot_size *chn_out_size)
{
    td_s32 i, ret;
    ot_mcf_chn mcf_chn;
    ot_mcf_chn_attr chn_attr;
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

td_s32 sample_common_mcf_start(ot_mcf_grp grp, const ot_mcf_grp_attr *grp_attr, ot_mcf_crop_info *grp_crop_info,
                               td_bool *mcf_chn_en, ot_size *chn_out_size)
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

td_s32 sample_common_mcf_stop(ot_mcf_grp grp, const td_bool *chn_enable, td_u32 chn_array_size)
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
