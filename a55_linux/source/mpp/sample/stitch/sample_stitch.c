/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <time.h>
#include "ss_mpi_stitch_lut_generate.h"
#include "sample_comm.h"
#include "securec.h"

#define SAMPLE_STITCH_DEFAULT_PIPE_NUM 2
#define SAMPLE_STITCH_DEFAULT_OVERLAP  270
#define SAMPLE_STITCH_LUT_0_FILE_NAME  "./stitch_lut0_sample.bin"
#define SAMPLE_STITCH_LUT_1_FILE_NAME  "./stitch_lut1_sample.bin"
#define SAMPLE_STITCH_LUT_ALIGN        16
#define SAMPLE_STITCH_LUT_DATA_LEN     8
#define SAMPLE_STITCH_LUT_HEAD_LEN     100
#ifdef OT_FPGA
#define SAMPLE_STITCH_STITCH_VB_CNT   6
#else
#define SAMPLE_STITCH_STITCH_VB_CNT   3
#endif

static volatile sig_atomic_t g_sig_flag = 0;

static ot_vi_stitch_cfg_mode g_stitch_cfg_mode = OT_VI_STITCH_CFG_MODE_NORM;
static ot_stitch_mode g_stitch_mode = OT_STITCH_BLEND;

static sample_vo_cfg g_vo_cfg = {
    .vo_dev            = SAMPLE_VO_DEV_UHD,
    .vo_intf_type      = OT_VO_INTF_BT1120,
    .intf_sync         = OT_VO_OUT_1080P60,
    .bg_color          = COLOR_RGB_BLACK,
    .pix_format        = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .disp_rect         = {0, 0, 1920, 1080},
    .image_size        = {1920, 1080},
    .vo_part_mode      = OT_VO_PARTITION_MODE_SINGLE,
    .dis_buf_len       = 3, /* 3: def buf len for single */
    .dst_dynamic_range = OT_DYNAMIC_RANGE_SDR8,
    .vo_mode           = VO_MODE_1X2,
    .compress_mode     = OT_COMPRESS_MODE_NONE,
};

static sample_comm_venc_chn_param g_venc_chn_param = {
    .frame_rate           = 30, /* 30 is a number */
    .stats_time           = 2,  /* 2 is a number */
    .gop                  = 60, /* 60 is a number */
    .venc_size            = {3300, 1080},
    .size                 = -1,
    .profile              = 0,
    .is_rcn_ref_share_buf = TD_FALSE,
    .gop_attr             = {
        .gop_mode = OT_VENC_GOP_MODE_NORMAL_P,
        .normal_p = {2},
    },
    .type                 = OT_PT_H265,
    .rc_mode              = SAMPLE_RC_CBR,
};

static td_void sample_get_char(td_void)
{
    if (g_sig_flag == 1) {
        return;
    }

    sample_pause();
}

static td_s32 sample_stitch_start_venc(ot_venc_chn venc_chn[], td_u32 chn_num)
{
    td_s32 i;
    td_s32 ret;
    for (i = 0; i < (td_s32)chn_num; i++) {
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

static td_void sample_stitch_stop_venc(ot_venc_chn venc_chn[], td_u32 chn_num)
{
    td_u32 i;

    sample_comm_venc_stop_get_stream(chn_num);

    for (i = 0; i < chn_num; i++) {
        sample_comm_venc_stop(venc_chn[i]);
    }
}

static td_void sample_stitch_get_vi_cfg(sample_sns_type sns_type, sample_vi_cfg *vi_cfg0, sample_vi_cfg *vi_cfg1)
{
    const ot_vi_dev vi_dev = 2; /* dev2 for sensor1 */
    const ot_vi_pipe vi_pipe = 1; /* dev2 bind pipe1 */

    sample_comm_vi_get_default_vi_cfg(sns_type, vi_cfg0);
    sample_comm_vi_get_default_vi_cfg(sns_type, vi_cfg1);

    vi_cfg0->pipe_info[0].chn_info[0].chn_attr.video_format = OT_VIDEO_FORMAT_TILE_32x4;
    vi_cfg0->pipe_info[0].chn_info[0].chn_attr.compress_mode = OT_COMPRESS_MODE_TILE;

    vi_cfg1->pipe_info[0].chn_info[0].chn_attr.video_format = OT_VIDEO_FORMAT_TILE_32x4;
    vi_cfg1->pipe_info[0].chn_info[0].chn_attr.compress_mode = OT_COMPRESS_MODE_TILE;

    vi_cfg0->mipi_info.divide_mode = LANE_DIVIDE_MODE_1;

#ifdef OT_FPGA
    vi_cfg1->sns_info.bus_id = 1; /* i2c1 */
#else
    vi_cfg1->sns_info.bus_id = 5; /* i2c5 */
#endif
    vi_cfg1->sns_info.sns_clk_src = 1;
    vi_cfg1->sns_info.sns_rst_src = 1;

    sample_comm_vi_get_mipi_info_by_dev_id(sns_type, vi_dev, &vi_cfg1->mipi_info);
    vi_cfg1->dev_info.vi_dev = vi_dev;
    vi_cfg1->bind_pipe.pipe_id[0] = vi_pipe;
    vi_cfg1->grp_info.grp_num = 1;
    vi_cfg1->grp_info.fusion_grp[0] = 1;
    vi_cfg1->grp_info.fusion_grp_attr[0].pipe_id[0] = vi_pipe;
}

static td_s32 sample_stitch_set_vi_stitch(td_void)
{
    ot_vi_stitch_grp_attr attr;

    attr.stitch_en =  TD_TRUE;
    attr.pipe_num = 2; /* 2: stitch pipe num */
    attr.pipe_id[0] = 0;
    attr.pipe_id[1] = 1;
    attr.max_pts_gap = 10000; /* 10000: 10ms */
    attr.cfg_mode = g_stitch_cfg_mode;
    return ss_mpi_vi_set_stitch_grp_attr(0, &attr);
}

static td_s32 sample_stitch_start_vi_vpss(sample_comm_cfg *comm_cfg)
{
    td_s32 ret = TD_SUCCESS;
    td_u32 pipe, pipe_r;
    ot_vpss_grp_cfg grp_cfg;
    sample_sns_type sns_type = SENSOR0_TYPE;
    sample_vi_cfg vi_cfg[2]; /* 2: stitch pipe num */
    sample_vpss_chn_attr vpss_chn_attr = {0};

    sample_stitch_set_vi_stitch();
    sample_stitch_get_vi_cfg(sns_type, &vi_cfg[0], &vi_cfg[1]);
    for (pipe = 0; pipe < comm_cfg->pipe_num; pipe++) {
        ret = sample_comm_vi_start_vi(&vi_cfg[pipe]);
        if (ret != TD_SUCCESS) {
            goto stop_vi;
        }

        sample_comm_vi_bind_vpss(pipe, 0, comm_cfg->vpss_grp[0], pipe);
    }

    if (ss_mpi_vpss_get_grp_cfg(comm_cfg->vpss_grp[0], &grp_cfg) != TD_SUCCESS) {
        goto stop_vi;
    }

    grp_cfg.max_out_rgn_num = comm_cfg->pipe_num;
    if (ss_mpi_vpss_set_grp_cfg(comm_cfg->vpss_grp[0], &grp_cfg) != TD_SUCCESS) {
        goto stop_vi;
    }

    memcpy_s(&vpss_chn_attr.chn_attr[0], sizeof(ot_vpss_chn_attr) * OT_VPSS_MAX_PHYS_CHN_NUM,
        comm_cfg->chn_attr, sizeof(ot_vpss_chn_attr) * OT_VPSS_MAX_PHYS_CHN_NUM);
    memcpy_s(&vpss_chn_attr.chn_enable[0], sizeof(vpss_chn_attr.chn_enable),
        comm_cfg->chn_en, sizeof(vpss_chn_attr.chn_enable));
    vpss_chn_attr.chn_array_size = OT_VPSS_MAX_PHYS_CHN_NUM;
    if (sample_common_vpss_start(comm_cfg->vpss_grp[0], &comm_cfg->grp_attr, &vpss_chn_attr) != TD_SUCCESS) {
        goto stop_vi;
    }

    return TD_SUCCESS;

stop_vi:
    for (pipe_r = 0; pipe_r < pipe; pipe_r++) {
        sample_comm_vi_bind_vpss(pipe_r, 0, comm_cfg->vpss_grp[0], pipe_r);
        sample_comm_vi_stop_vi(&vi_cfg[pipe_r]);
    }
    return ret;
}

static td_void sample_stitch_stop_vi_vpss(sample_comm_cfg *comm_cfg)
{
    td_u32 pipe;
    sample_sns_type sns_type = SENSOR0_TYPE;
    sample_vi_cfg   vi_cfg[2]; /* 2: stitch pipe num */

    sample_stitch_get_vi_cfg(sns_type, &vi_cfg[0], &vi_cfg[1]);

    sample_common_vpss_stop(comm_cfg->vpss_grp[0], comm_cfg->chn_en, OT_VPSS_MAX_PHYS_CHN_NUM);

    /* pipe0 must stop first for stitch */
    for (pipe = 0; pipe < comm_cfg->pipe_num; pipe++) {
        sample_comm_vi_un_bind_vpss(pipe, 0, comm_cfg->vpss_grp[0], pipe);
        sample_comm_vi_stop_vi(&vi_cfg[pipe]);
    }
}

static td_s32 sample_stitch_start_vo_venc(sample_comm_cfg *comm_cfg)
{
    td_s32 ret;
    ot_venc_chn venc_chn[SAMPLE_STITCH_DEFAULT_PIPE_NUM] = {0, 1};

    ret = sample_comm_vo_start_vo(&g_vo_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = sample_stitch_start_venc(venc_chn, 1);
    if (ret != TD_SUCCESS) {
        sample_comm_vo_stop_vo(&g_vo_cfg);
        return ret;
    }

    sample_comm_vpss_bind_vo(comm_cfg->vpss_grp[0], OT_VPSS_CHN0, 0, 0);
    sample_comm_vpss_bind_venc(comm_cfg->vpss_grp[0], OT_VPSS_CHN0, 0);
    return TD_SUCCESS;
}

static td_void sample_stitch_stop_vo_venc(sample_comm_cfg *comm_cfg)
{
    ot_venc_chn venc_chn[SAMPLE_STITCH_DEFAULT_PIPE_NUM] = {0, 1};

    sample_comm_vpss_un_bind_vo(comm_cfg->vpss_grp[0], OT_VPSS_CHN0, 0, 0);
    sample_comm_vpss_un_bind_venc(comm_cfg->vpss_grp[0], 0, 0);

    sample_stitch_stop_venc(venc_chn, 1);
    sample_comm_vo_stop_vo(&g_vo_cfg);
}

static td_s32 sample_stitch_start_route(sample_comm_cfg *comm_cfg)
{
    if (sample_comm_sys_init(comm_cfg) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if (sample_stitch_start_vi_vpss(comm_cfg) != TD_SUCCESS) {
        goto sys_exit;
    }

    if (sample_stitch_start_vo_venc(comm_cfg) != TD_SUCCESS) {
        goto stop_vi_vpss;
    }

    return TD_SUCCESS;

stop_vi_vpss:
    sample_stitch_stop_vi_vpss(comm_cfg);
sys_exit:
    sample_comm_sys_exit();
    return TD_FAILURE;
}

static td_void sample_stitch_stop_route(sample_comm_cfg *comm_cfg)
{
    sample_stitch_stop_vo_venc(comm_cfg);
    sample_stitch_stop_vi_vpss(comm_cfg);
    sample_comm_sys_exit();
}

static td_void sample_stitch_get_overlap_point(td_u32 pipe_num, td_u32 width, td_u32 height, td_u32 overlap,
    ot_stitch_attr *stitch_attr)
{
    td_u32 i;

    for (i = 0; i < pipe_num; i++) {
        if (i == 0) {
            stitch_attr->overlap_point[i][0].x = 0;
        } else {
            stitch_attr->overlap_point[i][0].x = overlap;
        }

        if (i == (pipe_num - 1)) {
            stitch_attr->overlap_point[i][1].x = width;
        } else {
            stitch_attr->overlap_point[i][1].x = width - overlap;
        }
    }
}

static td_void sample_stitch_get_cylindrical_default_attr(td_u32 pipe_num, td_u32 width, td_u32 height,
    ot_stitch_attr *stitch_attr)
{
    td_u32 pipe;

    stitch_attr->correction_mode = OT_STITCH_CORRECTION_CYLINDRICAL;
    stitch_attr->enable = TD_TRUE;
    stitch_attr->pipe_num = pipe_num;
    stitch_attr->sync_pipe_en = TD_TRUE;
    stitch_attr->stitch_mode = g_stitch_mode;
    sample_stitch_get_overlap_point(pipe_num, width, height, SAMPLE_STITCH_DEFAULT_OVERLAP, stitch_attr);

    /* The following numbers belong to the algorithm, and are typical parameters. */
    for (pipe = 0; pipe < pipe_num; pipe++) {
        stitch_attr->correction_attr[pipe].cylindrical_attr.pmf_enable = TD_TRUE;
        stitch_attr->correction_attr[pipe].cylindrical_attr.pmf_coef[0] = 524288; /* 524288: algorithm data */
        stitch_attr->correction_attr[pipe].cylindrical_attr.pmf_coef[4] = 524288; /* 524288: algorithm data */
        stitch_attr->correction_attr[pipe].cylindrical_attr.pmf_coef[5] = 524288 * 92; /* 524288:92: algorithm data */
        stitch_attr->correction_attr[pipe].cylindrical_attr.pmf_coef[8] = 524288; /* 524288: algorithm data */
        stitch_attr->correction_attr[pipe].cylindrical_attr.center_x_offset = 0;
        stitch_attr->correction_attr[pipe].cylindrical_attr.center_y_offset = 0;
        stitch_attr->correction_attr[pipe].cylindrical_attr.ratio = 210; /* 210: ratio */
        stitch_attr->correction_attr[pipe].cylindrical_attr.dst_size.width = width;
        stitch_attr->correction_attr[pipe].cylindrical_attr.dst_size.height = height;
    }
}

static td_void sample_stitch_get_default_blend_param(td_u32 pipe_num, ot_stitch_blend_param *blend_param)
{
    td_u32 pipe;
    blend_param->buf_num = 3; /* 3: blend buf num */

    for (pipe = 0; pipe < pipe_num - 1; pipe++) {
        blend_param->blend_param[pipe].blend_position = 64; /* 64: blend pos */
        blend_param->blend_param[pipe].blend_range = 32; /* 32: blend range */
    }
}

static td_s32 sample_stitch_start_stitch(sample_comm_cfg *comm_cfg, ot_stitch_attr *stitch_attr)
{
    td_s32 ret;
    ot_gdc_param gdc_param;
    ot_stitch_blend_param blend_param = { 0 };

    gdc_param.cell_size = OT_LUT_CELL_SIZE_16;
    gdc_param.in_size.width = comm_cfg->in_size.width;
    gdc_param.in_size.height = comm_cfg->in_size.height;
    ret = ss_mpi_vpss_set_grp_gdc_param(comm_cfg->vpss_grp[0], &gdc_param);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_vpss_set_grp_gdc_param(grp:%d) failed with %#x!\n", comm_cfg->vpss_grp[0], ret);
        return TD_FAILURE;
    }

    if (stitch_attr->stitch_mode == OT_STITCH_BLEND) {
        sample_stitch_get_default_blend_param(comm_cfg->pipe_num, &blend_param);
        ret = ss_mpi_vpss_set_grp_stitch_blend_param(comm_cfg->vpss_grp[0], &blend_param);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_vpss_set_grp_stitch_blend_param(grp:%d) failed with %#x!\n",
                comm_cfg->vpss_grp[0], ret);
            return TD_FAILURE;
        }
    }

    ret = ss_mpi_vpss_set_grp_stitch(comm_cfg->vpss_grp[0], stitch_attr);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_vpss_set_grp_stitch(grp:%d) failed with %#x!\n", comm_cfg->vpss_grp[0], ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void sample_stitch_start_cylindrical_stitch(sample_comm_cfg *comm_cfg)
{
    ot_stitch_attr stitch_attr = { 0 };

    sample_stitch_get_cylindrical_default_attr(comm_cfg->pipe_num, comm_cfg->in_size.width, comm_cfg->in_size.height,
        &stitch_attr);

    if (sample_stitch_start_stitch(comm_cfg, &stitch_attr) != TD_SUCCESS) {
        return;
    }

    sample_get_char();
}

static td_void sample_stitch_get_vb_cfg(td_u32 pipe_num, td_u32 width, td_u32 height, ot_vb_cfg *vb_cfg)
{
    ot_vb_calc_cfg calc_cfg;
    ot_pic_buf_attr buf_attr;

    vb_cfg->max_pool_cnt = 4; /* 4: max pool cnt */

    buf_attr.width         = width;
    buf_attr.height        = height;
    buf_attr.align         = OT_DEFAULT_ALIGN;
    buf_attr.bit_width     = OT_DATA_BIT_WIDTH_8;
    buf_attr.pixel_format  = OT_PIXEL_FORMAT_RGB_BAYER_12BPP;
    buf_attr.compress_mode = OT_COMPRESS_MODE_LINE;
    buf_attr.video_format  = OT_VIDEO_FORMAT_LINEAR;
    ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);
    vb_cfg->common_pool[0].blk_size = calc_cfg.vb_size;
    vb_cfg->common_pool[0].blk_cnt = pipe_num * SAMPLE_STITCH_STITCH_VB_CNT;

    buf_attr.pixel_format  = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    buf_attr.compress_mode = OT_COMPRESS_MODE_TILE;
    buf_attr.video_format  = OT_VIDEO_FORMAT_TILE_32x4;
    ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);
    vb_cfg->common_pool[1].blk_size = calc_cfg.vb_size;
    vb_cfg->common_pool[1].blk_cnt = pipe_num * SAMPLE_STITCH_STITCH_VB_CNT;

    buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    buf_attr.video_format  = OT_VIDEO_FORMAT_LINEAR;
    ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);
    vb_cfg->common_pool[2].blk_size = calc_cfg.vb_size; /* 2: blk index */
    vb_cfg->common_pool[2].blk_cnt = pipe_num * SAMPLE_STITCH_STITCH_VB_CNT; /* 2: blk index */

    buf_attr.width = width * pipe_num - (pipe_num - 1) * SAMPLE_STITCH_DEFAULT_OVERLAP * (1 << g_stitch_mode);
    ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);
    vb_cfg->common_pool[3].blk_size = calc_cfg.vb_size; /* 3: blk index */
    vb_cfg->common_pool[3].blk_cnt = pipe_num * SAMPLE_STITCH_STITCH_VB_CNT; /* 3: blk index */
}

static td_s32 sample_stitch_cylindrical_mode(td_void)
{
    const td_u32 pipe_num = SAMPLE_STITCH_DEFAULT_PIPE_NUM;
    sample_comm_cfg comm_cfg = { 0 };

    sample_comm_sys_get_default_cfg(pipe_num, &comm_cfg);
    comm_cfg.mode_type = OT_VI_OFFLINE_VPSS_OFFLINE;
    comm_cfg.chn_attr[OT_VPSS_CHN0].compress_mode = OT_COMPRESS_MODE_NONE;
    comm_cfg.chn_attr[OT_VPSS_CHN0].width = comm_cfg.in_size.width * pipe_num -
        (pipe_num - 1) * SAMPLE_STITCH_DEFAULT_OVERLAP * (1 << g_stitch_mode);
    g_venc_chn_param.venc_size.width = comm_cfg.chn_attr[OT_VPSS_CHN0].width;
    g_venc_chn_param.venc_size.height = comm_cfg.in_size.height;
    sample_stitch_get_vb_cfg(pipe_num, comm_cfg.in_size.width, comm_cfg.in_size.height, &comm_cfg.vb_cfg);
    if (sample_stitch_start_route(&comm_cfg) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    sample_stitch_start_cylindrical_stitch(&comm_cfg);
    sample_stitch_stop_route(&comm_cfg);
    return TD_SUCCESS;
}

static td_void sample_stitch_get_lut_default_attr(td_u32 pipe_num, td_u32 width, td_u32 height,
    ot_stitch_attr *stitch_attr)
{
    td_u32 pipe;

    stitch_attr->enable = TD_TRUE;
    stitch_attr->pipe_num = pipe_num;
    stitch_attr->sync_pipe_en = TD_TRUE;
    stitch_attr->stitch_mode = g_stitch_mode;
    stitch_attr->correction_mode = OT_STITCH_CORRECTION_LUT;
    sample_stitch_get_overlap_point(pipe_num, width, height, SAMPLE_STITCH_DEFAULT_OVERLAP, stitch_attr);

    for (pipe = 0; pipe < pipe_num; pipe++) {
        stitch_attr->correction_attr[pipe].lut.cell_size = OT_LUT_CELL_SIZE_16;
        stitch_attr->correction_attr[pipe].lut.in_size.width = width;
        stitch_attr->correction_attr[pipe].lut.in_size.height = height;
        stitch_attr->correction_attr[pipe].lut.out_size.width = width;
        stitch_attr->correction_attr[pipe].lut.out_size.height = height;
    }
}

static td_s32 sample_stitch_read_lut_file(FILE* fd, ot_size *size, td_u32 stride,
    td_phys_addr_t *phys_addr, td_void **virt_addr)
{
    td_s32 ret;
    td_void *temp_virt_addr;
    td_u32 len = stride * size->height;

    printf("read lut len : %u\n", len);
    ret = ss_mpi_sys_mmz_alloc(phys_addr, &temp_virt_addr, TD_NULL, TD_NULL, len);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_sys_mmz_alloc failed with %#x!\n", ret);
        return TD_FAILURE;
    }

    *virt_addr = temp_virt_addr;

    if (fseek(fd, SAMPLE_STITCH_LUT_HEAD_LEN, SEEK_SET) != 0) {
        sample_print("fseek failed!\n");
        goto fail;
    }

    for (td_u32 i = 0; i < size->height; i++) {
        if (fread(temp_virt_addr, stride, 1, fd) != 1) {
            sample_print("read lut line%u failed!\n", i);
            goto fail;
        }
        temp_virt_addr += stride;
    }
    return TD_SUCCESS;
fail:
    ss_mpi_sys_mmz_free(*phys_addr, *virt_addr);
    return TD_FAILURE;
}

static td_void sample_stitch_free_lut(td_phys_addr_t phys_addr[], td_void *virt_addr[], td_s32 pipe_num)
{
    td_s32 pipe;
    for (pipe = 0; pipe < pipe_num; pipe++) {
        ss_mpi_sys_mmz_free(phys_addr[pipe], virt_addr[pipe]);
    }
}

static td_s32 sample_stitch_prepare_single_lut(td_char *file_name, ot_lut *lut,
    td_phys_addr_t *phys_addr, td_void **virt_addr)
{
    FILE *fp = TD_NULL;
    const td_u32 clip = 2; /* 2: muti */
    td_u32 stride;
    td_u32 cell_size;
    td_u32 width;
    ot_size size;
    td_char *path = TD_NULL;

    path = realpath(file_name, TD_NULL);
    if (path == TD_NULL) {
        sample_print("file_name is invalid!!\n");
        return TD_FAILURE;
    }

    fp = fopen(path, "rb");
    if (fp == TD_NULL) {
        return TD_FAILURE;
    }

    cell_size = SAMPLE_STITCH_LUT_ALIGN * (1 << lut->cell_size);
    size.width = (lut->out_size.width - 1) / cell_size + clip;
    size.height = (lut->out_size.height - 1) / cell_size + clip;
    width = lut->out_size.width;
    stride = ot_gdc_get_lut_buf_stride(width, lut->cell_size);
    if (sample_stitch_read_lut_file(fp, &size, stride, phys_addr, virt_addr) != TD_SUCCESS) {
        (td_void)fclose(fp);
        return TD_FAILURE;
    }
    lut->lut_phys_addr = *phys_addr;
    lut->lut_len = stride * size.height;
    (td_void)fclose(fp);
    return TD_SUCCESS;
}

static td_s32 sample_stitch_prepare_lut(td_s32 pipe_num, td_phys_addr_t phys_addr[], td_void *virt_addr[],
    ot_stitch_attr *stitch_attr)
{
    td_s32 ret;

    ret = sample_stitch_prepare_single_lut(SAMPLE_STITCH_LUT_0_FILE_NAME, &stitch_attr->correction_attr[0].lut,
        &phys_addr[0], &virt_addr[0]);
    if (ret != TD_SUCCESS) {
        sample_print("prepare pipe%d lut failed!\n", 0);
        sample_stitch_free_lut(phys_addr, virt_addr, 0);
        return ret;
    }
    ret = sample_stitch_prepare_single_lut(SAMPLE_STITCH_LUT_1_FILE_NAME, &stitch_attr->correction_attr[1].lut,
        &phys_addr[1], &virt_addr[1]);
    if (ret != TD_SUCCESS) {
        sample_print("prepare pipe%d lut failed!\n", 1);
        sample_stitch_free_lut(phys_addr, virt_addr, 1);
        return ret;
    }
    return TD_SUCCESS;
}

static td_void sample_stitch_start_lut_stitch(sample_comm_cfg *comm_cfg)
{
    ot_stitch_attr stitch_attr = { 0 };
    td_phys_addr_t phys_addr[SAMPLE_VIO_MAX_ROUTE_NUM];
    td_void *virt_addr[SAMPLE_VIO_MAX_ROUTE_NUM];

    sample_stitch_get_lut_default_attr(comm_cfg->pipe_num, comm_cfg->in_size.width, comm_cfg->in_size.height,
        &stitch_attr);
    if (sample_stitch_prepare_lut(comm_cfg->pipe_num, phys_addr, virt_addr, &stitch_attr) != TD_SUCCESS) {
        sample_print("read lut filed!\n");
        return;
    }

    if (sample_stitch_start_stitch(comm_cfg, &stitch_attr) != TD_SUCCESS) {
        goto exit;
    }

    sample_get_char();
exit:
    sample_stitch_free_lut(phys_addr, virt_addr, comm_cfg->pipe_num);
}

static td_s32 sample_stitch_lut_mode(td_void)
{
    const td_u32 pipe_num = SAMPLE_STITCH_DEFAULT_PIPE_NUM;
    sample_comm_cfg comm_cfg = { 0 };

    sample_comm_sys_get_default_cfg(pipe_num, &comm_cfg);
    comm_cfg.mode_type = OT_VI_OFFLINE_VPSS_OFFLINE;
    comm_cfg.chn_attr[OT_VPSS_CHN0].compress_mode = OT_COMPRESS_MODE_NONE;
    comm_cfg.chn_attr[OT_VPSS_CHN0].width = comm_cfg.in_size.width * pipe_num -
        (pipe_num - 1) * SAMPLE_STITCH_DEFAULT_OVERLAP * (1 << g_stitch_mode);
    g_venc_chn_param.venc_size.width = comm_cfg.chn_attr[OT_VPSS_CHN0].width;
    g_venc_chn_param.venc_size.height = comm_cfg.in_size.height;
    sample_stitch_get_vb_cfg(pipe_num, comm_cfg.in_size.width, comm_cfg.in_size.height, &comm_cfg.vb_cfg);
    if (sample_stitch_start_route(&comm_cfg) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    sample_stitch_start_lut_stitch(&comm_cfg);
    sample_stitch_stop_route(&comm_cfg);
    return TD_SUCCESS;
}

static td_void sample_stitch_lut_generate_cfg(ot_stitch_config *stitch_cfg)
{
    ot_stitch_avs_config *avs_stitch_config = &stitch_cfg->avs_stitch_config;
    avs_stitch_config->camera_num = 2; /* 2 cameras */
    avs_stitch_config->src_size.width = 2688; /* src width 2688 */
    avs_stitch_config->src_size.height = 1520; /* src height 1520 */
    avs_stitch_config->dst_size.width = 5084; /* dst width 5084 */
    avs_stitch_config->dst_size.height = 1520; /* dst height 1520 */
    avs_stitch_config->prj_mode = PROJECTION_CYLINDRICAL;
    avs_stitch_config->center.x = 1290; /* center x coordinate 1290 */
    avs_stitch_config->center.y = 750; /* center y coordinate 750 */
    avs_stitch_config->fov.fov_x = 17000; /* fov x 17000 */
    avs_stitch_config->fov.fov_y = 4550; /* fov y 4550 */
    avs_stitch_config->ori_rotation.yaw = 0;
    avs_stitch_config->ori_rotation.pitch = 0;
    avs_stitch_config->ori_rotation.roll = 0;
    avs_stitch_config->rotation.yaw = 0;
    avs_stitch_config->rotation.pitch = 0;
    avs_stitch_config->rotation.roll = 0;

    ot_stitch_lut_generate_input *lut_input = &stitch_cfg->lut_input;
    lut_input->type = TYPE_AVSP;
    lut_input->fine_tuning_cfg.fine_tuning_en = TD_FALSE;
    lut_input->fine_tuning_cfg.adjust[0].adjust_en = TD_TRUE;
    lut_input->fine_tuning_cfg.adjust[0].yaw = 0;
    lut_input->fine_tuning_cfg.adjust[0].roll = 0;
    lut_input->fine_tuning_cfg.adjust[0].pitch = 0;
    lut_input->fine_tuning_cfg.adjust[0].offset_x = 0;
    lut_input->fine_tuning_cfg.adjust[0].offset_y = 0;
    lut_input->fine_tuning_cfg.adjust[1].adjust_en = TD_TRUE;
    lut_input->fine_tuning_cfg.adjust[1].yaw = 0;
    lut_input->fine_tuning_cfg.adjust[1].roll = 0;
    lut_input->fine_tuning_cfg.adjust[1].pitch = 0;
    lut_input->fine_tuning_cfg.adjust[1].offset_x = 0;
    lut_input->fine_tuning_cfg.adjust[1].offset_y = 0;
    lut_input->stitch_distance = 5.0; /* stitch distance 5.0 meters */
}

static td_s32 sample_stitch_lut_generate(td_void)
{
    clock_t t1, t2;
    t1 = clock();
    td_s32 ret;

    ot_stitch_config stitch_cfg;
    sample_stitch_lut_generate_cfg(&stitch_cfg);

    td_char cal_file[255] = "./lut/stitch.cal";
    td_char mesh_file_prefix[255] = "./lut";
    stitch_cfg.lut_input.cal_file_name = cal_file;

    ot_stitch_img_size_cfg size_cfg;
    size_cfg.left_point[0].x = 0;
    size_cfg.right_point[0].x = 2750; /* 2750: ending point of overlap in final stitch image */
    size_cfg.left_point[1].x = 2334; /* 2334: starting point of overlap in final stitch image */
    size_cfg.right_point[1].x = stitch_cfg.avs_stitch_config.dst_size.width;

    ot_stitch_out_param out_param;

    td_phys_addr_t phys_addr[SAMPLE_VIO_MAX_ROUTE_NUM];
    td_void *lut_virt_addr[SAMPLE_VIO_MAX_ROUTE_NUM];

    for (int i = 0; i < stitch_cfg.avs_stitch_config.camera_num; i++) {
        int out_width = size_cfg.right_point[i].x - size_cfg.left_point[i].x;
        int out_height = stitch_cfg.avs_stitch_config.dst_size.height;
        int cell_width = 16;
        int cell_height = 16;
        int mesh_width = (out_width + cell_width - 1) / cell_width + 1;
        int mesh_height = (out_height + cell_height - 1) / cell_height + 1;
        int lut_len = mesh_width * mesh_height * sizeof(ot_point);

        ret = ss_mpi_sys_mmz_alloc(&phys_addr[i], &lut_virt_addr[i], TD_NULL, TD_NULL, lut_len);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_sys_mmz_alloc failed with %#x!\n", ret);
            return TD_FAILURE;
        }

        out_param.lut[i].mesh_points = (ot_point *)lut_virt_addr[i];
    }

    ret = ss_mpi_stitch_lut_generate(&stitch_cfg, &size_cfg, &out_param, mesh_file_prefix);

    sample_stitch_free_lut(phys_addr, lut_virt_addr, stitch_cfg.avs_stitch_config.camera_num);
    printf("ss_mpi_stitch_lut_generate finished!\n");

    t2 = clock();
    double duration;
    duration = (double)(t2 - t1) / CLOCKS_PER_SEC;
    printf("****** Total time for refreshing lut = %f s ******\n", duration);

    return ret;
}

static td_void sample_stitch_usage(const char *prg_name)
{
    printf("usage : %s <index> \n", prg_name);
    printf("index:\n");
    printf("    (0) cylindrical stitch : 2 pipe vi linear(Offline, normal mode) -> vpss(blend) -> venc && vo.\n");
    printf("    (1) cylindrical stitch : 2 pipe vi linear(Offline, sync mode) -> vpss(blend) -> venc && vo.\n");
    printf("    (2) lut stitch : 2 pipe vi linear(Offline, normal mode) -> vpss(blend) -> venc && vo.\n");
    printf("    (3) lut stitch : 2 pipe vi linear(Offline, sync mode) -> vpss(blend) -> venc && vo.\n");
    printf("    (4) stitch lut generate.\n");
}

static td_void sample_stitch_handle_sig(td_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        g_sig_flag = 1;
    }
}

static td_void sample_register_sig_handler(td_void (*sig_handle)(td_s32))
{
    struct sigaction sa;

    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sig_handle;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, TD_NULL);
    sigaction(SIGTERM, &sa, TD_NULL);
}

static td_s32 sample_stitch_execute_case(td_u32 case_index)
{
    td_s32 ret;

    switch (case_index) {
        case 0: /* 0: cylindrical blend stitch with normal mode */
            ret = sample_stitch_cylindrical_mode();
            break;
        case 1: /* 1: cylindrical blend stitch with sync mode */
            g_stitch_cfg_mode = OT_VI_STITCH_CFG_MODE_SYNC;
            ret = sample_stitch_cylindrical_mode();
            break;
        case 2: /* 2: lut blend stitch with normal mode */
            ret = sample_stitch_lut_mode();
            break;
        case 3: /* 3: lut blend stitch with sync mode */
            g_stitch_cfg_mode = OT_VI_STITCH_CFG_MODE_SYNC;
            ret = sample_stitch_lut_mode();
            break;
        case 4: /* 4: stitch lut generate */
            ret = sample_stitch_lut_generate();
            break;
        default:
            ret = TD_FAILURE;
            break;
    }

    return ret;
}

#ifdef __LITEOS__
td_s32 app_main(td_s32 argc, td_char *argv[])
#else
td_s32 main(td_s32 argc, td_char *argv[])
#endif
{
    td_s32 ret;
    td_u32 index;
    td_char *end_ptr = TD_NULL;

    if (argc != 2) { /* 2:arg num */
        sample_stitch_usage(argv[0]);
        return TD_FAILURE;
    }

    if (!strncmp(argv[1], "-h", 2)) { /* 2:arg num */
        sample_stitch_usage(argv[0]);
        return TD_FAILURE;
    }

    if (strlen(argv[1]) != 1 || argv[1][0] < '0' || argv[1][0] > '4') {
        sample_stitch_usage(argv[0]);
        return TD_FAILURE;
    }

#ifndef __LITEOS__
    sample_register_sig_handler(sample_stitch_handle_sig);
#endif
    errno = 0;
    index = (td_u32)strtol(argv[1], &end_ptr, 10); /* base 10 */
    if ((end_ptr == argv[1]) || (*end_ptr) != '\0' || errno == ERANGE) {
        printf("input index error!\n");
        return TD_FAILURE;
    }

    ret = sample_stitch_execute_case(index);
    if ((ret == TD_SUCCESS) && (g_sig_flag == 0)) {
        printf("\033[0;32mprogram exit normally!\033[0;39m\n");
    } else {
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

#ifdef __LITEOS__
    return ret;
#else
    exit(ret);
#endif
}
