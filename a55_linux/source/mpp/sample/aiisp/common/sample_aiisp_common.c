/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <signal.h>
#include <limits.h>
#include "sample_aiisp_common.h"


#define VO_WIDTH 1920
#define VO_HEIGTH 1080

#define VENC_WIDTH 3840
#define VENC_HEIGTH 2160

static sample_vo_cfg g_vo_cfg = {
    .vo_dev            = SAMPLE_VO_DEV_UHD,
    .vo_layer          = SAMPLE_VO_LAYER_VHD0,
    .vo_intf_type      = OT_VO_INTF_BT1120,
    .intf_sync         = OT_VO_OUT_1080P60,
    .bg_color          = COLOR_RGB_BLACK,
    .pix_format        = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .disp_rect         = {0, 0, VO_WIDTH, VO_HEIGTH},
    .image_size        = {VO_WIDTH, VO_HEIGTH},
    .vo_part_mode      = OT_VO_PARTITION_MODE_SINGLE,
    .dis_buf_len       = 3, /* 3: def buf len for single */
    .dst_dynamic_range = OT_DYNAMIC_RANGE_SDR8,
    .vo_mode           = VO_MODE_1MUX,
    .compress_mode     = OT_COMPRESS_MODE_NONE,
};

static sample_comm_venc_chn_param g_venc_chn_param = {
    .frame_rate           = 30, /* 30 is a number */
    .stats_time           = 2,  /* 2 is a number */
    .gop                  = 60, /* 60 is a number */
    .venc_size            = {VENC_WIDTH, VENC_HEIGTH},
    .size                 = -1,
    .profile              = 0,
    .is_rcn_ref_share_buf = TD_FALSE,
    .gop_attr             = {
        .gop_mode = OT_VENC_GOP_MODE_NORMAL_P,
        .normal_p = {2},
    },
    .type                 = OT_PT_H264,
    .rc_mode              = SAMPLE_RC_CBR,
};

volatile sig_atomic_t g_sig_flag = 0;

td_void sample_aiisp_handle_sig(td_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        g_sig_flag = 1;
    }
}

td_void sample_get_char(td_char *s)
{
    if (g_sig_flag == 1) {
        return;
    }

    printf("---------------press any key to %s!---------------\n", s);
    (td_void)getchar();
}

sig_atomic_t sample_aiisp_get_sig(td_void)
{
    return g_sig_flag;
}

sample_sns_type sample_aiisp_get_wdr_sns_type(sample_sns_type sns_type)
{
    if (sns_type == OV_OS08A20_MIPI_8M_30FPS_12BIT) {
        return OV_OS08A20_MIPI_8M_30FPS_10BIT_WDR2TO1;
    } else if (sns_type == OV_OS04A10_MIPI_4M_30FPS_12BIT) {
        return OV_OS04A10_MIPI_4M_30FPS_12BIT_WDR2TO1;
    } else if (sns_type == SC450AI_MIPI_4M_30FPS_10BIT) {
        return SC450AI_MIPI_4M_30FPS_10BIT_WDR2TO1;
    } else if (sns_type == SC850SL_MIPI_8M_30FPS_12BIT) {
        return SC850SL_MIPI_8M_30FPS_10BIT_WDR2TO1;
    }

    sample_print("sns_type: %d, unsupport wdr mode!\n", sns_type);
    return SNS_TYPE_BUTT;
}
static td_s32 sample_aiisp_check_fp(FILE *fp, td_char *model_file)
{
    if (fp == TD_NULL) {
        sample_print("open file err!\n");
        return TD_FAILURE;
    }
    printf("open %s success\n", model_file);
    return TD_SUCCESS;
}

td_s32 sample_aiisp_load_mem(ot_aiisp_mem_info *mem, td_char *model_file)
{
    td_s32 ret;
    FILE *fp = TD_NULL;
    td_char path[PATH_MAX + 1] = {0};

    /* Get model file size */
    sample_aiisp_check_exps_return((strlen(model_file) > PATH_MAX) || realpath(model_file, path) == TD_NULL);
    fp = fopen(path, "rb");
    if (sample_aiisp_check_fp(fp, model_file) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    ret = fseek(fp, 0L, SEEK_END);
    if (ret != TD_SUCCESS) {
        sample_print("fseek end failed!\n");
        goto fail_0;
    }

    mem->size = ftell(fp);
    if (mem->size <= 0) {
        sample_print("ftell failed!\n");
        goto fail_0;
    }

    ret = fseek(fp, 0L, SEEK_SET);
    if (ret != TD_SUCCESS) {
        sample_print("fseek set failed!\n");
        goto fail_0;
    }

    /* malloc model file mem */
    ret = ss_mpi_sys_mmz_alloc(&(mem->phys_addr), &(mem->virt_addr), "aibnr_cfg", TD_NULL, mem->size);
    if (ret != TD_SUCCESS) {
        sample_print("malloc mmz failed!\n");
        goto fail_0;
    }

    ret = fread(mem->virt_addr, mem->size, 1, fp);
    if (ret != 1) {
        sample_print("read model file failed!\n");
        goto fail_1;
    }

    ret = fclose(fp);
    if (ret != 0) {
        sample_print("close file error\n");
    }
    return TD_SUCCESS;

fail_1:
    ss_mpi_sys_mmz_free(mem->phys_addr, mem->virt_addr);
    mem->phys_addr = 0;
    mem->virt_addr = TD_NULL;
fail_0:
    (td_void)fclose(fp);
    return TD_FAILURE;
}

td_void sample_aiisp_unload_mem(ot_aiisp_mem_info *param_mem)
{
    if ((param_mem->phys_addr != 0) && (param_mem->virt_addr != TD_NULL)) {
        (td_void)ss_mpi_sys_mmz_free(param_mem->phys_addr, param_mem->virt_addr);
    }
}

static td_void sample_aiisp_get_default_vb_config(const ot_size *size, ot_vb_cfg *vb_cfg, td_u32 vb_cnt)
{
    ot_vb_calc_cfg calc_cfg = {0};
    ot_pic_buf_attr buf_attr = {0};

    (td_void)memset_s(vb_cfg, sizeof(ot_vb_cfg), 0, sizeof(ot_vb_cfg));
    vb_cfg->max_pool_cnt = 128; /* 128 blks */

    buf_attr.width         = size->width;
    buf_attr.height        = size->height;
    buf_attr.align         = OT_DEFAULT_ALIGN;
    buf_attr.bit_width     = OT_DATA_BIT_WIDTH_8;
    buf_attr.pixel_format  = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422;
    buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    buf_attr.video_format  = OT_VIDEO_FORMAT_LINEAR;
    ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);

    vb_cfg->common_pool[0].blk_size = calc_cfg.vb_size;
    vb_cfg->common_pool[0].blk_cnt  = vb_cnt;
}

#ifdef SAMPLE_MEM_SHARE_ENABLE
static td_void sample_aiisp_init_mem_share(td_void)
{
    td_u32 i;
    ot_vb_common_pools_id pools_id = {0};

    if (ss_mpi_vb_get_common_pool_id(&pools_id) != TD_SUCCESS) {
        sample_print("get common pool_id failed!\n");
        return;
    }
    for (i = 0; i < pools_id.pool_cnt; ++i) {
        ss_mpi_vb_pool_share_all(pools_id.pool[i]);
    }
}
#endif

td_s32 sample_aiisp_sys_init(ot_size *in_size, td_u32 vb_cnt)
{
    td_s32 ret;
    ot_vb_cfg vb_cfg = {0};
    td_u32 supplement_config;
    ot_vi_vpss_mode_type mode_type = OT_VI_OFFLINE_VPSS_OFFLINE;

    sample_aiisp_get_default_vb_config(in_size, &vb_cfg, vb_cnt);

    supplement_config = OT_VB_SUPPLEMENT_BNR_MOT_MASK;
    ret = sample_comm_sys_init_with_vb_supplement(&vb_cfg, supplement_config);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

#ifdef SAMPLE_MEM_SHARE_ENABLE
    sample_aiisp_init_mem_share();
#endif

    ret = sample_comm_vi_set_vi_vpss_mode(mode_type, OT_VI_AIISP_MODE_DEFAULT);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 sample_aiisp_start_vpss(ot_vpss_grp grp, ot_size *in_size)
{
    td_s32 ret;
    ot_vpss_grp_attr grp_attr = {0};
    sample_vpss_chn_attr vpss_chn_attr = {0};

    vpss_chn_attr.chn_enable[0] = TD_TRUE;
    vpss_chn_attr.chn_array_size = OT_VPSS_MAX_PHYS_CHN_NUM;
    sample_comm_vpss_get_default_grp_attr(&grp_attr);
    grp_attr.max_width  = in_size->width;
    grp_attr.max_height = in_size->height;
    sample_comm_vpss_get_default_chn_attr(&vpss_chn_attr.chn_attr[0]);
    vpss_chn_attr.chn_attr[0].width  = in_size->width;
    vpss_chn_attr.chn_attr[0].height = in_size->height;

    ret = sample_common_vpss_start(grp, &grp_attr, &vpss_chn_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 sample_aiisp_start_vo(sample_vo_mode vo_mode)
{
    g_vo_cfg.vo_mode = vo_mode;

    return sample_comm_vo_start_vo(&g_vo_cfg);
}

td_void sample_aiisp_stop_vo(td_void)
{
    sample_comm_vo_stop_vo(&g_vo_cfg);
}

td_s32 sample_aiisp_start_venc(ot_venc_chn venc_chn[], td_u32 venc_chn_len, td_u32 chn_num, ot_size *size)
{
    td_s32 i;
    td_s32 ret;

    g_venc_chn_param.venc_size.width        = size->width;
    g_venc_chn_param.venc_size.height       = size->height;
    g_venc_chn_param.size = sample_comm_sys_get_pic_enum(size);

    for (i = 0; i < (td_s32)chn_num && i < (td_s32)venc_chn_len; i++) {
        ret = sample_comm_venc_start(venc_chn[i], &g_venc_chn_param);
        if (ret != TD_SUCCESS) {
            goto exit;
        }
    }

    ret = sample_comm_venc_start_get_stream(venc_chn, chn_num);
    if (ret != TD_SUCCESS) {
        goto exit;
    }

    return TD_SUCCESS;

exit:
    for (i = i - 1; i >= 0; i--) {
        sample_comm_venc_stop(venc_chn[i]);
    }
    return TD_FAILURE;
}

td_s32 sample_aiisp_start_venc_and_vo(ot_vpss_grp vpss_grp[], td_u32 grp_num, ot_size *in_size)
{
    td_u32 i;
    td_s32 ret;
    sample_vo_mode vo_mode = VO_MODE_1MUX;
    const ot_vpss_chn vpss_chn = 0;
    const ot_vo_layer vo_layer = 0;
    ot_vo_chn vo_chn[4] = {0, 1, 2, 3};     /* 4: max chn num, 0/1/2/3 chn id */
    ot_venc_chn venc_chn[4] = {0, 1, 2, 3}; /* 4: max chn num, 0/1/2/3 chn id */

    if (grp_num > 1) {
        vo_mode = VO_MODE_4MUX;
    }

    ret = sample_aiisp_start_vo(vo_mode);
    if (ret != TD_SUCCESS) {
        goto start_vo_failed;
    }

    ret = sample_aiisp_start_venc(venc_chn, sizeof(venc_chn) / sizeof(ot_venc_chn), grp_num, in_size);
    if (ret != TD_SUCCESS) {
        goto start_venc_failed;
    }

    for (i = 0; i < grp_num; i++) {
        sample_comm_vpss_bind_vo(vpss_grp[i], vpss_chn, vo_layer, vo_chn[i]);
        sample_comm_vpss_bind_venc(vpss_grp[i], vpss_chn, venc_chn[i]);
    }

    return TD_SUCCESS;

start_venc_failed:
    sample_aiisp_stop_vo();
start_vo_failed:
    return TD_FAILURE;
}

td_void sample_aiisp_stop_vpss(ot_vpss_grp grp)
{
    td_bool chn_enable[OT_VPSS_MAX_PHYS_CHN_NUM] = {TD_TRUE, TD_FALSE, TD_FALSE, TD_FALSE};

    sample_common_vpss_stop(grp, chn_enable, OT_VPSS_MAX_PHYS_CHN_NUM);
}

td_void sample_aiisp_stop_venc(ot_venc_chn venc_chn[], td_u32 venc_chn_len, td_u32 chn_num)
{
    td_u32 i;

    sample_comm_venc_stop_get_stream(chn_num);

    for (i = 0; i < chn_num && i < venc_chn_len; i++) {
        sample_comm_venc_stop(venc_chn[i]);
    }
}

td_void sample_aiisp_stop_venc_and_vo(ot_vpss_grp vpss_grp[], td_u32 grp_num)
{
    td_u32 i;
    const ot_vpss_chn vpss_chn = 0;
    const ot_vo_layer vo_layer = 0;
    ot_vo_chn vo_chn[4] = {0, 1, 2, 3};     /* 4: max chn num, 0/1/2/3 chn id */
    ot_venc_chn venc_chn[4] = {0, 1, 2, 3}; /* 4: max chn num, 0/1/2/3 chn id */

    for (i = 0; i < grp_num; i++) {
        sample_comm_vpss_un_bind_vo(vpss_grp[i], vpss_chn, vo_layer, vo_chn[i]);
        sample_comm_vpss_un_bind_venc(vpss_grp[i], vpss_chn, venc_chn[i]);
    }

    sample_aiisp_stop_venc(venc_chn, sizeof(venc_chn) / sizeof(ot_venc_chn), grp_num);
    sample_aiisp_stop_vo();
}

td_s32 sample_aiisp_set_long_frame_mode(ot_vi_pipe vi_pipe, td_bool is_wdr_mode)
{
    td_s32 ret;
    ot_isp_exposure_attr exp_attr = {0};
    ot_isp_wdr_fs_attr fswdr_attr = {0};

    if (is_wdr_mode == TD_FALSE) {
        return TD_SUCCESS;
    }

    sample_get_char("set long frame mode");

    ret = ss_mpi_isp_get_exposure_attr(vi_pipe, &exp_attr);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_isp_get_exposure_attr error[0x%x]\n", ret);
        return TD_FAILURE;
    }

    exp_attr.auto_attr.fswdr_mode = OT_ISP_FSWDR_LONG_FRAME_MODE;
    ret = ss_mpi_isp_set_exposure_attr(vi_pipe, &exp_attr);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_isp_set_exposure_attr error[0x%x]\n", ret);
        return TD_FAILURE;
    }

    ret = ss_mpi_isp_get_fswdr_attr(vi_pipe, &fswdr_attr);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_isp_get_fswdr_attr error[0x%x]\n", ret);
        return TD_FAILURE;
    }

    fswdr_attr.wdr_merge_mode = OT_ISP_MERGE_WDR_MODE;
    ret = ss_mpi_isp_set_fswdr_attr(vi_pipe, &fswdr_attr);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_isp_set_fswdr_attr error[0x%x]\n", ret);
        return TD_FAILURE;
    }

    return ret;
}

td_void sample_aiisp_get_default_cfg(sample_sns_type sns_type, ot_vi_pipe vi_pipe,
    ot_size *size, sample_vi_cfg *vi_cfg)
{
    sample_comm_vi_get_size_by_sns_type(sns_type, size);
    sample_comm_vi_get_default_vi_cfg(sns_type, vi_cfg);

#ifdef OT_FPGA
    vi_cfg->pipe_info[vi_pipe].pipe_attr.frame_rate_ctrl.src_frame_rate = 30; /* 30fps */
    vi_cfg->pipe_info[vi_pipe].pipe_attr.frame_rate_ctrl.dst_frame_rate = 5; /* 5fps */
#endif
}

td_void sample_aiisp_sensor_cfg(ot_vi_pipe vi_pipe, sample_sns_type sns_type)
{
    switch (sns_type) {
        case SONY_IMX347_SLAVE_MIPI_4M_30FPS_12BIT:
            g_sns_imx347_slave_obj.pfn_mirror_flip(vi_pipe, ISP_SNS_MIRROR);
            break;

        default:
            break;
    }

    return;
}

