/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>

#include "ss_mpi_sys_mem.h"
#ifdef CONFIG_OT_VI_ALL_SUPPORT
#include "ss_mpi_vi.h"
#endif
#include "ss_mpi_vpss.h"
#include "ss_mpi_vgs.h"
#include "ot_common_vpss.h"
#include "ss_mpi_vb.h"
#include "ot_common_vb.h"
#include "ot_common_gdc.h"
#include "ot_buffer.h"
#include "securec.h"

#include "ss_mpi_fisheye_calibration.h"

#define MAX_DIGIT_LEN 2
#define FISHEYE_CALIBRATE_LEVEL OT_FISHEYE_CALIBRATION_LEVEL_1
typedef struct {
    ot_vb_blk vb_blk;
    ot_vb_pool vb_pool;
    td_u32 pool_id;

    td_phys_addr_t phys_addr;
    td_void *virt_addr;
} fisheye_dump_mem;

td_u32  g_orig_depth = 0;
static volatile sig_atomic_t g_quit = TD_FALSE;   /* g_quit may be set in the signal handler */
static FILE *g_fd_out = TD_NULL;
ot_fisheye_correction_attr g_correction_attr = {0};
td_bool g_fisheye_orig_en = TD_FALSE;
static ot_vgs_handle g_handle = -1;
static ot_vb_pool g_pool = OT_VB_INVALID_POOL_ID;

#ifdef CONFIG_OT_VI_ALL_SUPPORT
typedef ot_vi_chn chn_type;
typedef ot_vi_chn_attr chn_attr_type;
typedef ot_vi_pipe parameter_type;
static ot_vi_chn g_chn = 0;
static ot_vi_pipe g_parameter = 0;
#else
typedef ot_vpss_chn chn_type;
typedef ot_vpss_chn_attr chn_attr_type;
typedef ot_vpss_grp parameter_type;
static td_u32 g_vpss_chn_mode = 0;
static ot_vpss_chn g_chn = 0;
static ot_vpss_grp g_parameter = 0;
#endif

#ifndef __LITEOS__
#define SAVE_PATH "."
#else
#define SAVE_PATH "/sharefs/output"
#endif

#define fisheye_clip3(low, high, x) ((x) < (low) ? (low) : ((x) > (high) ? (high) : (x)))

static void usage(void)
{
    printf(
        "\n"
        "*************************************************\n"
#ifdef CONFIG_OT_VI_ALL_SUPPORT
#ifndef __LITEOS__
        "Usage: ./fisheye_calibrate [ViPipe] [ViPhyChn].\n"
#else
        "Usage: fisheye_cal [ViPipe] [ViPhyChn].\n"
#endif
        "1)ViPipe: \n"
        "   the source ViPipe\n"
        "2)ViPhyChn: \n"
        "   the source physic channel bind the exetend channel\n"
        "*)Example:\n"
#else
#ifndef __LITEOS__
        "Usage: ./fisheye_calibrate [VpssGrp] [VpssChn].\n"
#else
        "Usage: fisheye_cal [VpssGrp] [VpssChn].\n"
#endif
        "1)VpssGrp: \n"
        "   the source ViPipe\n"
        "2)VpssPhyChn: \n"
        "   the source physic channel bind the exetend channel\n"
        "*)Example:\n"
#endif

#ifndef __LITEOS__
        "   e.g : ./fisheye_calibration 0 0\n"
#else
        "   e.g : fisheye_cal 0 0\n"
#endif
        "*************************************************\n\n");
}

static inline td_void safe_close_file(td_void)
{
    if (g_fd_out != TD_NULL) {
        fclose(g_fd_out);
        g_fd_out = TD_NULL;
    }
}

static void sig_handler(td_s32 signo)
{
    if (g_quit == TD_TRUE) {
        return;
    }

    if ((SIGINT == signo) || (SIGTERM == signo)) {
        g_quit = TD_TRUE;
    }
}

static td_s32 fisheye_calibrate_map_virt_addr(ot_video_frame_info *frame)
{
    td_s32 size = 0;
    td_phys_addr_t phy_addr;
    ot_pixel_format pixel_format;
    td_u32 y_size;
    /* td_u32 uv_interval = 0 */

    pixel_format = frame->video_frame.pixel_format;
    phy_addr = frame->video_frame.phys_addr[0];
    if (pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) {
        size = (frame->video_frame.stride[0]) * (frame->video_frame.height) * 3 / 2; /* 3 / 2 for size */
        y_size = frame->video_frame.stride[0] * frame->video_frame.height;
        /* uv_interval = 0 */
    } else if (pixel_format == OT_PIXEL_FORMAT_YVU_PLANAR_420) {
        size = (frame->video_frame.stride[0]) * (frame->video_frame.height) * 3 / 2; /* 3 / 2 for size */
        y_size = frame->video_frame.stride[0] * frame->video_frame.height;
        /* uv_interval = frame->video_frame.width * frame->video_frame.height / 2 */
    } else {
        printf("not support pixelformat: %d\n", pixel_format);
        return TD_FAILURE;
    }

    frame->video_frame.virt_addr[0] = ss_mpi_sys_mmap_cached(phy_addr, size);
    if (frame->video_frame.virt_addr[0] == TD_NULL) {
        printf("Map Virt Addr Failed!\n");
        return TD_FAILURE;
    }

    frame->video_frame.virt_addr[1] = frame->video_frame.virt_addr[0] + y_size;
    /* frame->video_frame.virt_addr[2] = frame->video_frame.virt_addr[1] + uv_interval */
    return TD_SUCCESS;
}


static td_void fisheye_calibrate_save_sp42x_to_planar(const ot_video_frame *v_buf)
{
    unsigned int w, h;
    char *v_buf_virt_y = TD_NULL;
    char *v_buf_virt_c = TD_NULL;
    char *mem_content = TD_NULL;
    unsigned char *tmp_buff = TD_NULL;
    ot_pixel_format pixel_format = v_buf->pixel_format;
    td_u32 uv_height;
    td_u32 line_size = 2408;

    if (pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) {
        uv_height = v_buf->height / 2; /* 2 half */
    } else {
        uv_height = v_buf->height;
    }

    if (v_buf->width / 2 > line_size) { /* 2 half */
        return;
    }

    v_buf_virt_y = (char *)v_buf->virt_addr[0];
    v_buf_virt_c = (char *)(v_buf_virt_y + (v_buf->stride[0]) * (v_buf->height));
    tmp_buff = (unsigned char *)malloc(line_size);
    if (tmp_buff == TD_NULL) {
        printf("Func:%s line:%d -- unable alloc %uB memory for tmp buffer\n",
               __FUNCTION__, __LINE__, line_size);
        return;
    }

    /* save Y */
    for (h = 0; h < v_buf->height; h++) {
        mem_content = v_buf_virt_y + h * v_buf->stride[0];
        (td_void)fwrite(mem_content, 1, v_buf->width, g_fd_out);
    }
    /* save U */
    for (h = 0; h < uv_height; h++) {
        mem_content = v_buf_virt_c + h * v_buf->stride[1];
        mem_content += 1;
        for (w = 0; w < v_buf->width / 2; w++) { /* 2 half */
            tmp_buff[w] = *mem_content;
            mem_content += 2; /* 2 for content */
        }
        (td_void)fwrite(tmp_buff, 1, v_buf->width / 2, g_fd_out); /* 2 half */
    }
    /* save V */
    for (h = 0; h < uv_height; h++) {
        mem_content = v_buf_virt_c + h * v_buf->stride[1];
        for (w = 0; w < v_buf->width / 2; w++) { /* 2 half */
            tmp_buff[w] = *mem_content;
            mem_content += 2; /* 2 for content */
        }
        (td_void)fwrite(tmp_buff, 1, v_buf->width / 2, g_fd_out); /* 2 half */
    }
    free(tmp_buff);
    return;
}

static td_s32 fisheye_calibrate_misc_buf_attr_get(ot_pic_buf_attr *pic_buf_attr, ot_vb_calc_cfg *cal_config,
    const ot_video_frame_info *in_frame)
{
    pic_buf_attr->align = 0;
    pic_buf_attr->bit_width = OT_DATA_BIT_WIDTH_8;
    pic_buf_attr->compress_mode = OT_COMPRESS_MODE_NONE;
    pic_buf_attr->width = in_frame->video_frame.width;
    pic_buf_attr->height = in_frame->video_frame.height;
    pic_buf_attr->pixel_format = in_frame->video_frame.pixel_format;
    pic_buf_attr->video_format = in_frame->video_frame.video_format;

    ot_common_get_pic_buf_cfg(pic_buf_attr, cal_config);
    if (in_frame->video_frame.pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) {
        /* uv_interval = 0 */
    } else if (in_frame->video_frame.pixel_format == OT_PIXEL_FORMAT_YVU_PLANAR_420) {
        /* uv_interval = cal_config->main_y_size >> 2 */
    } else {
        printf("Error!!!, not support pixel_format: %d\n", in_frame->video_frame.pixel_format);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 fisheye_calibrate_misc_get_vb(ot_video_frame_info *out_frame, const ot_video_frame_info *in_frame,
    ot_vb_blk *out_vb_blk, ot_vb_pool pool)
{
    td_phys_addr_t phys_addr;
    td_void *virt_addr = TD_NULL;
    ot_vb_calc_cfg cal_config;
    /* td_u32 uv_interval */
    ot_pic_buf_attr pic_buf_attr;

    if (fisheye_calibrate_misc_buf_attr_get(&pic_buf_attr, &cal_config, in_frame) != TD_SUCCESS) {
        goto end0;
    }

    *out_vb_blk = ss_mpi_vb_get_blk(pool, cal_config.vb_size, TD_NULL);
    if (*out_vb_blk == OT_VB_INVALID_HANDLE) {
        printf("ss_mpi_vb_get_blk err! size:%u\n", cal_config.vb_size);
        goto end0;
    }

    phys_addr = ss_mpi_vb_handle_to_phys_addr(*out_vb_blk);
    if (phys_addr == 0) {
        printf("ss_mpi_vb_handle_to_phys_addr err!\n");
        goto end1;
    }

    virt_addr = ss_mpi_sys_mmap(phys_addr, cal_config.vb_size);
    if (virt_addr == TD_NULL) {
        printf("ss_mpi_sys_mmap err!\n");
        goto end1;
    }

    out_frame->pool_id = ss_mpi_vb_handle_to_pool_id(*out_vb_blk);
    if (out_frame->pool_id == OT_VB_INVALID_POOL_ID) {
        printf("pool_id err!\n");
        goto end2;
    }

    out_frame->video_frame.phys_addr[0] = phys_addr;
    /* printf("\nuser phys_addr = 0x%x\n", out_frame->video_frame.phys_addr[0]) */
    out_frame->video_frame.phys_addr[1] = out_frame->video_frame.phys_addr[0] + cal_config.main_y_size;
    /* out_frame->video_frame.phys_addr[2] = out_frame->video_frame.phys_addr[1] + uv_interval */
    out_frame->video_frame.virt_addr[0] = virt_addr;
    out_frame->video_frame.virt_addr[1] = out_frame->video_frame.virt_addr[0] + cal_config.main_y_size;
    /* out_frame->video_frame.virt_addr[2] = out_frame->video_frame.virt_addr[1] + uv_interval */
    out_frame->video_frame.width  = pic_buf_attr.width;
    out_frame->video_frame.height = pic_buf_attr.height;
    out_frame->video_frame.stride[0] = cal_config.main_stride;
    out_frame->video_frame.stride[1] = cal_config.main_stride;
    /* out_frame->video_frame.stride[2] = cal_config.main_stride */
    out_frame->video_frame.field = OT_VIDEO_FIELD_FRAME;
    out_frame->video_frame.compress_mode = OT_COMPRESS_MODE_NONE;
    out_frame->video_frame.pixel_format = pic_buf_attr.pixel_format;

    return TD_SUCCESS;

end2:
    ss_mpi_sys_munmap(virt_addr, cal_config.vb_size);
end1:
    ss_mpi_vb_release_blk(*out_vb_blk);
end0:
    return TD_FAILURE;
}

static td_s32 fisheye_calibrate_get_chn_fisheye(ot_fisheye_correction_attr *fisheye_correction_attr)
{
    if (ss_mpi_vpss_get_grp_fisheye(g_parameter, fisheye_correction_attr) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 fisheye_calibrate_set_chn_fisheye(const ot_fisheye_correction_attr *fisheye_correction_attr)
{
    if (ss_mpi_vpss_set_grp_fisheye(g_parameter, fisheye_correction_attr) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_bool fisheye_calibrate_is_valid_digit_str(const td_char *str)
{
    td_s32 i;
    td_s32 str_len;

    str_len = strlen(str);
    if (str_len > MAX_DIGIT_LEN) {
        return TD_FALSE;
    }

    for (i = 0; i < str_len; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return TD_FALSE;
        }
    }
    return TD_TRUE;
}

static td_s32 fisheye_calibrate_check_param(td_void)
{
#ifdef CONFIG_OT_VI_ALL_SUPPORT
    if ((g_parameter < 0) || (g_parameter >= OT_VI_MAX_PIPE_NUM)) {
        printf("vi pipe id %d is invalid, must be [0, %d)!\n", g_parameter, OT_VI_MAX_PIPE_NUM);
        return TD_FAILURE;
    }
    if ((g_chn < 0) || (g_chn >= OT_VI_MAX_PHYS_CHN_NUM)) {
        printf("Physic Channel id %d is invalid, must be [0, %d)!\n", g_chn, OT_VI_MAX_PHYS_CHN_NUM);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
#else
    if ((g_parameter < 0) || (g_parameter >= OT_VPSS_MAX_GRP_NUM)) {
        printf("vpss grp %d is invalid, must be [0, %d)!\n", g_parameter, OT_VPSS_MAX_GRP_NUM);
        return TD_FAILURE;
    }
    if ((g_chn < 0) || (g_chn >= OT_VPSS_MAX_PHYS_CHN_NUM)) {
        printf("Physic Channel id %d is invalid, must be [0, %d)!\n", g_chn, OT_VPSS_MAX_PHYS_CHN_NUM);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
#endif
}

static td_s32 fisheye_calibrate_check_and_set(int argc, char *argv[])
{
    td_s32 i;
    td_char *end_ptr = TD_NULL;
    const td_s32 base = 10; /* 10:Decimal */
    td_s32 ret;

    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("NOTICE: This tool only only support PIXEL_FORMAT_YVU_SEMIPLANAR_420\n");
    printf("NOTICE: This tool only only support DYNAMIC_RANGE_SDR8\n");
    printf("NOTICE: This tool only only support COMPRESS_MODE_NONE\n");

    /* register signal handler */
    (td_void)signal(SIGINT, sig_handler);
    (td_void)signal(SIGTERM, sig_handler);

    if (argc > 1) {
        if (!strncmp(argv[1], "-h", 2)) { /* 2 bytes */
            usage();
            return TD_FAILURE;
        }
    }

    if (argc != 3) { /* argc 3 */
        usage();
        return TD_FAILURE;
    }

    for (i = 1; i < argc; i++) {
        if (!fisheye_calibrate_is_valid_digit_str(argv[i])) {
            printf("%s is invalid, must be reasonable non negative integers!!!!\n\n", argv[i]);
            return TD_FAILURE;
        }
    }

    errno = 0;
    g_parameter = (td_s32)strtol(argv[1], &end_ptr, base);
    if ((errno != 0) || (*end_ptr != '\0')) {
        return TD_FAILURE;
    }

    errno = 0;
    g_chn = (td_s32)strtol(argv[2], &end_ptr, base); /* 2: chn id */
    if ((errno != 0) || (*end_ptr != '\0')) {
        return TD_FAILURE;
    }

    ret = fisheye_calibrate_check_param();
    if (ret != TD_SUCCESS) {
        return ret;
    }
    return TD_SUCCESS;
}

static td_s32 fisheye_calibrate_recover_chn_attr(chn_attr_type *chn_attr)
{
    td_s32 ret;

    if (g_orig_depth != -1U) {
        chn_attr->depth = g_orig_depth;
#ifdef CONFIG_OT_VI_ALL_SUPPORT
        ret = ss_mpi_vi_set_chn_attr(g_parameter, g_chn, chn_attr);
#else
        chn_attr->chn_mode = g_vpss_chn_mode;
        ret = ss_mpi_vpss_set_chn_attr(g_parameter, g_chn, chn_attr);
#endif
        if (ret != TD_SUCCESS) {
            printf("set chn attr error!\n");
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_s32 fisheye_calibrate_set_chn_attr(chn_attr_type *chn_attr)
{
    td_s32 ret;

#ifdef CONFIG_OT_VI_ALL_SUPPORT
    ret = ss_mpi_vi_get_chn_attr(g_parameter, g_chn, chn_attr);
#else
    ret = ss_mpi_vpss_get_chn_attr(g_parameter, g_chn, chn_attr);
#endif
    if (ret != TD_SUCCESS) {
        printf("get chn attr error!!!\n");
        return TD_FAILURE;
    }

    g_orig_depth = chn_attr->depth;
    chn_attr->depth = 1;

#ifdef CONFIG_OT_VI_ALL_SUPPORT
    ret = ss_mpi_vi_set_chn_attr(g_parameter, g_chn, chn_attr);
#else
    g_vpss_chn_mode = chn_attr->chn_mode;
    chn_attr->chn_mode = OT_VPSS_CHN_MODE_USER;
    ret = ss_mpi_vpss_set_chn_attr(g_parameter, g_chn, chn_attr);
#endif
    if (ret != TD_SUCCESS) {
        printf("set chn attr error!!!\n");
        return TD_FAILURE;
    }

    /* switch off fisheye if enabled */
    ret = fisheye_calibrate_get_chn_fisheye(&g_correction_attr);
    if (ret != TD_SUCCESS) {
        printf("get fisheye attr failed with ret:0x%x!\n", ret);
        return ret;
    }
    g_fisheye_orig_en = g_correction_attr.enable;
    if (g_fisheye_orig_en == TD_TRUE) {
        g_correction_attr.enable = TD_FALSE;
        ret = fisheye_calibrate_set_chn_fisheye(&g_correction_attr);
        if (ret != TD_SUCCESS) {
            printf("set fisheye attr failed with ret:0x%x!\n", ret);
            return ret;
        }
    }

    usleep(900000); /* 900000us */

    if (g_quit == TD_TRUE) {
        (td_void)fisheye_calibrate_recover_chn_attr(chn_attr);
        return TD_FAILURE;
    }

    return ret;
}

static td_s32 fisheye_calibrate_vb_create_pool(const ot_video_frame_info *frame_in,
    td_u32 *blk_size, ot_vb_pool *h_pool)
{
    ot_pic_buf_attr pic_buf_attr;
    ot_vb_pool_cfg vb_pool_cfg;

    if ((frame_in->video_frame.compress_mode != OT_COMPRESS_MODE_NONE) ||
        (frame_in->video_frame.dynamic_range != OT_DYNAMIC_RANGE_SDR8) ||
        (frame_in->video_frame.pixel_format != OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420)) {
        printf("compress_mode or dynamic_range or pixel_format not support!!! \n");
        return TD_FAILURE;
    }

    /* Save VI PIC For UserMode */
    pic_buf_attr.width = frame_in->video_frame.width;
    pic_buf_attr.height = frame_in->video_frame.height;
    pic_buf_attr.pixel_format = frame_in->video_frame.pixel_format;
    pic_buf_attr.bit_width = OT_DATA_BIT_WIDTH_8;
    pic_buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    pic_buf_attr.video_format = OT_VIDEO_FORMAT_LINEAR;
    pic_buf_attr.align = 32; /* align 32 bytes */
    *blk_size = ot_common_get_pic_buf_size(&pic_buf_attr);
    (td_void)memset_s(&vb_pool_cfg, sizeof(ot_vb_pool_cfg), 0, sizeof(ot_vb_pool_cfg));
    vb_pool_cfg.blk_size = *blk_size;
    vb_pool_cfg.blk_cnt  = 4; /* 4 blk */
    vb_pool_cfg.remap_mode = OT_VB_REMAP_MODE_NONE;
    *h_pool = ss_mpi_vb_create_pool(&vb_pool_cfg);
    if (*h_pool == OT_VB_INVALID_POOL_ID) {
        printf("ss_mpi_vb_create_pool failed! \n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void fisheye_calibrate_result_process(const ot_video_frame_info *frame_in,
                                                const ot_video_frame_info *frame_out)
{
    td_s32 ret;
    td_u32 i;
    ot_fisheye_calibration_output out_calibrate;

    if (g_quit == TD_TRUE) {
        return;
    }

    (td_void)memset_s(&out_calibrate, sizeof(ot_fisheye_calibration_output), 0, sizeof(ot_fisheye_calibration_output));

    printf("Compute Calibrate Result.....\n");
    ret = ss_mpi_fisheye_calibration_compute(&frame_in->video_frame, FISHEYE_CALIBRATE_LEVEL, &out_calibrate);
    if (ret != TD_SUCCESS) {
        printf("Mark Result Failed!\n");
        return;
    }

    if (g_quit == TD_TRUE) {
        return;
    }

    printf(" Radius_X=%d,\n Radius_Y=%d,\n Radius=%u,\n OffsetH=%d,\n OffsetV=%d. \n",
           out_calibrate.calibration_result.radius_x, out_calibrate.calibration_result.radius_y,
           out_calibrate.calibration_result.radius, out_calibrate.calibration_result.offset_h,
           out_calibrate.calibration_result.offset_v);

    printf("Mark Calibrate Result.....\n");
    ret = ss_mpi_fisheye_calibration_mark(&frame_in->video_frame, &frame_out->video_frame,
        &out_calibrate.calibration_result);
    if (ret != TD_SUCCESS) {
        printf("Mark Result Failed!\n");
        return;
    }

    /* switch on fisheye and apply fisheye calibrate result if enabled */
    if (g_fisheye_orig_en == TD_TRUE) {
        g_correction_attr.enable = TD_TRUE;
        g_correction_attr.fisheye_attr.hor_offset =
            fisheye_clip3(-511, 511, out_calibrate.calibration_result.offset_h); /* 511 : 2^9 - 1 */

        g_correction_attr.fisheye_attr.ver_offset =
            fisheye_clip3(-511, 511, out_calibrate.calibration_result.offset_v); /* 511 : 2^9 - 1 */

        for (i = 0; i < g_correction_attr.fisheye_attr.rgn_num; i++) {
            g_correction_attr.fisheye_attr.fisheye_rgn_attr[i].out_radius = out_calibrate.calibration_result.radius;
        }

        ret = fisheye_calibrate_set_chn_fisheye(&g_correction_attr);
        if (ret != TD_SUCCESS) {
            printf("Set Fisheye Attr Failed!\n");
            return;
        }
    }
}

static td_s32 fisheye_calibrate_open_file(const ot_video_frame_info *frame_in)
{
    td_s32 ret;
    td_char yuv_name[256] = {0};
    td_char *pix_frm = TD_NULL;
    char resolved_path[PATH_MAX] = {0};

    if (frame_in == NULL) {
        printf("fisheye_calibrate_open_file null ptr err\n");
        return TD_FAILURE;
    }

    pix_frm = (OT_PIXEL_FORMAT_YUV_400 == frame_in->video_frame.pixel_format) ? "single" :
              ((frame_in->video_frame.pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) ? "p420" : "p422");
    if (realpath(SAVE_PATH, resolved_path) == TD_NULL) {
        return TD_FAILURE;
    }

    ret = snprintf_s(yuv_name, sizeof(yuv_name), sizeof(yuv_name) - 1,
        "%s/fisheye_calibrate_out_%d_%u_%u_%s.yuv", resolved_path,
        g_chn, frame_in->video_frame.width, frame_in->video_frame.height, pix_frm);
    if (ret == -1) {
        return TD_FAILURE;
    }

    g_fd_out = fopen(yuv_name, "wb");
    if (g_fd_out == TD_NULL) {
        printf("open file %s err\n", yuv_name);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 fisheye_calibrate_release_chn_frame(ot_video_frame_info *frame_in)
{
#ifdef CONFIG_OT_VI_ALL_SUPPORT
    if (ss_mpi_vi_release_chn_frame(g_parameter, g_chn, frame_in) != TD_SUCCESS) {
        return TD_FAILURE;
    }
#else
    if (ss_mpi_vpss_release_chn_frame(g_parameter, g_chn, frame_in) != TD_SUCCESS) {
        return TD_FAILURE;
    }
#endif
    return TD_SUCCESS;
}

td_s32 fisheye_calibrate_init_vgs_pool(fisheye_dump_mem *dump_mem, ot_vb_calc_cfg *vb_calc_cfg,
    ot_video_frame_info *frame_in)
{
    td_u32 width = frame_in->video_frame.width;
    td_u32 height = frame_in->video_frame.height;
    ot_pixel_format pixel_format = frame_in->video_frame.pixel_format;
    ot_pic_buf_attr buf_attr = {0};
    ot_vb_pool_cfg vb_pool_cfg = {0};

    if (vb_calc_cfg == TD_NULL) {
        printf("vb_calc_cfg is null!\n");
        return TD_FAILURE;
    }

    buf_attr.width = width;
    buf_attr.height = height;
    buf_attr.pixel_format = pixel_format;
    buf_attr.bit_width = OT_DATA_BIT_WIDTH_8;
    buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    buf_attr.align = 0;
    ot_common_get_pic_buf_cfg(&buf_attr, vb_calc_cfg);

    vb_pool_cfg.blk_size = vb_calc_cfg->vb_size;
    vb_pool_cfg.blk_cnt = 1;
    vb_pool_cfg.remap_mode = OT_VB_REMAP_MODE_NONE;

    g_pool = ss_mpi_vb_create_pool(&vb_pool_cfg);
    if (g_pool == OT_VB_INVALID_POOL_ID) {
        printf("MPI_VB_CreatePool failed! \n");
        return TD_FAILURE;
    }
    if (dump_mem == TD_NULL) {
        printf("dump mem is null!\n");
        return TD_FAILURE;
    }

    dump_mem->vb_pool = g_pool;
    dump_mem->vb_blk = ss_mpi_vb_get_blk(dump_mem->vb_pool, vb_calc_cfg->vb_size, TD_NULL);
    if (dump_mem->vb_blk == OT_VB_INVALID_HANDLE) {
        printf("get vb blk failed!\n");
        return TD_FAILURE;
    }

    dump_mem->phys_addr = ss_mpi_vb_handle_to_phys_addr(dump_mem->vb_blk);
    return TD_SUCCESS;
}

td_void fisheye_calibrate_set_vgs_frame_info(ot_video_frame_info *vgs_frame_info, const fisheye_dump_mem *dump_mem,
    const ot_vb_calc_cfg *vb_calc_cfg, const ot_video_frame_info *frame_in)
{
    if ((frame_in == TD_NULL) || (vb_calc_cfg == TD_NULL) ||
        (dump_mem == TD_NULL) || (vgs_frame_info == TD_NULL)) {
        return;
    }
    vgs_frame_info->video_frame.phys_addr[0] = dump_mem->phys_addr;
    vgs_frame_info->video_frame.phys_addr[1] = vgs_frame_info->video_frame.phys_addr[0] + vb_calc_cfg->main_y_size;
    vgs_frame_info->video_frame.width = frame_in->video_frame.width;
    vgs_frame_info->video_frame.height = frame_in->video_frame.height;
    vgs_frame_info->video_frame.stride[0] = vb_calc_cfg->main_stride;
    vgs_frame_info->video_frame.stride[1] = vb_calc_cfg->main_stride;
    vgs_frame_info->video_frame.compress_mode = OT_COMPRESS_MODE_NONE;
    vgs_frame_info->video_frame.pixel_format = frame_in->video_frame.pixel_format;
    vgs_frame_info->video_frame.video_format = OT_VIDEO_FORMAT_LINEAR;
    vgs_frame_info->video_frame.field = OT_VIDEO_FIELD_FRAME;
    vgs_frame_info->video_frame.dynamic_range = frame_in->video_frame.dynamic_range;
    vgs_frame_info->video_frame.pts = 0;
    vgs_frame_info->video_frame.time_ref = 0;
    vgs_frame_info->pool_id = dump_mem->vb_pool;
    vgs_frame_info->mod_id = OT_ID_VGS;
}

static td_s32 fisheye_calibrate_send_vgs(ot_video_frame_info *frame_in, fisheye_dump_mem *dump_mem)
{
    ot_video_frame_info vgs_frame_info = {0};
    ot_vgs_task_attr vgs_task_attr;
    ot_vb_calc_cfg vb_calc_cfg = {0};

    if (fisheye_calibrate_init_vgs_pool(dump_mem, &vb_calc_cfg, frame_in) != TD_SUCCESS) {
        printf("init vgs pool failed\n");
        return TD_FAILURE;
    }

    fisheye_calibrate_set_vgs_frame_info(&vgs_frame_info, dump_mem, &vb_calc_cfg, frame_in);

    if (ss_mpi_vgs_begin_job(&g_handle) != TD_SUCCESS) {
        printf("ss_mpi_vgs_begin_job failed\n");
        return TD_FAILURE;
    }

    if (memcpy_s(&vgs_task_attr.img_in, sizeof(ot_video_frame_info), frame_in, sizeof(ot_video_frame_info)) != EOK) {
        printf("memcpy_s img_in failed\n");
        goto err_exit;
    }

    if (memcpy_s(&vgs_task_attr.img_out, sizeof(ot_video_frame_info),
        &vgs_frame_info, sizeof(ot_video_frame_info)) != EOK) {
        printf("memcpy_s img_out failed\n");
        goto err_exit;
    }

    if (ss_mpi_vgs_add_scale_task(g_handle, &vgs_task_attr, OT_VGS_SCALE_COEF_NORM) != TD_SUCCESS) {
        printf("ss_mpi_vgs_add_scale_task failed\n");
        goto err_exit;
    }

    if (ss_mpi_vgs_end_job(g_handle) != TD_SUCCESS) {
        printf("ss_mpi_vgs_end_job failed\n");
        goto err_exit;
    }

    fisheye_calibrate_release_chn_frame(frame_in);

    if (memcpy_s(frame_in, sizeof(ot_video_frame_info),
        &vgs_task_attr.img_out, sizeof(ot_video_frame_info)) != EOK) {
        printf("memcpy_s frame_out failed\n");
        return TD_FAILURE;
    }

    dump_mem->vb_pool = OT_VB_INVALID_POOL_ID;
    if (g_pool != OT_VB_INVALID_POOL_ID) {
        ss_mpi_vb_destroy_pool(g_pool);
        g_pool = OT_VB_INVALID_POOL_ID;
    }
    g_handle = -1;
    return TD_SUCCESS;

err_exit:
    fisheye_calibrate_release_chn_frame(frame_in);
    ss_mpi_vgs_cancel_job(g_handle);
    g_handle = -1;
    return TD_FAILURE;
}

static td_s32 fisheye_calibrate_convert_chn_frame(ot_video_frame_info *frame_in, fisheye_dump_mem *dump_mem)
{
    td_bool is_send_to_vgs;

    is_send_to_vgs = ((frame_in->video_frame.compress_mode != OT_COMPRESS_MODE_NONE) ||
            (frame_in->video_frame.video_format != OT_VIDEO_FORMAT_LINEAR));
    if (is_send_to_vgs) {
        if (fisheye_calibrate_send_vgs(frame_in, dump_mem) != TD_SUCCESS) {
            return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

static td_s32 fisheye_calibrate_get_chn_frame(ot_video_frame_info *frame_in, fisheye_dump_mem *dump_mem)
{
#ifdef CONFIG_OT_VI_ALL_SUPPORT
    if (ss_mpi_vi_get_chn_frame(g_parameter, g_chn, frame_in, 2000) != TD_SUCCESS) { /* 2000 milli_sec */
        return TD_FAILURE;
    }
#else
    if (ss_mpi_vpss_get_chn_frame(g_parameter, g_chn, frame_in, 2000) != TD_SUCCESS) { /* 2000 milli_sec */
        return TD_FAILURE;
    }
#endif
    return fisheye_calibrate_convert_chn_frame(frame_in, dump_mem);
}

static td_void fisheye_calibrate_restore(fisheye_dump_mem *dump_mem)
{
    if (dump_mem->vb_blk != 0) {
        ss_mpi_vb_release_blk(dump_mem->vb_blk);
    }
    if (g_pool != OT_VB_INVALID_POOL_ID) {
        ss_mpi_vb_destroy_pool(g_pool);
        g_pool = OT_VB_INVALID_POOL_ID;
    }
}

#ifdef __LITEOS__
td_s32 fisheye_calibrate(int argc, char *argv[])
#else
td_s32 main(int argc, char *argv[])
#endif
{
    ot_vb_pool h_pool;
    td_u32 blk_size;
    ot_vb_blk vb_blk = OT_VB_INVALID_HANDLE;
    ot_video_frame_info frame_in = { 0 };
    ot_video_frame_info frame_out;
    chn_attr_type chn_attr;
    fisheye_dump_mem dump_mem = {0};

    if (fisheye_calibrate_check_and_set(argc, argv) != TD_SUCCESS) {
        goto end0;
    }

    if (fisheye_calibrate_set_chn_attr(&chn_attr) != TD_SUCCESS) {
        goto end0;
    }

    if (fisheye_calibrate_get_chn_frame(&frame_in, &dump_mem) != TD_SUCCESS) {
        printf("fisheye calibrate get chn frame err\n");
        goto end1;
    }

    if (fisheye_calibrate_open_file(&frame_in) != TD_SUCCESS) {
        goto end1;
    }

    if (fisheye_calibrate_vb_create_pool(&frame_in, &blk_size, &h_pool) != TD_SUCCESS) {
        goto end2;
    }

    (td_void)memcpy_s(&frame_out, sizeof(ot_video_frame_info), &frame_in, sizeof(ot_video_frame_info));
    if (fisheye_calibrate_misc_get_vb(&frame_out, &frame_in, &vb_blk, h_pool) != TD_SUCCESS) {
        goto end3;
    }

    if (fisheye_calibrate_map_virt_addr(&frame_in) != TD_SUCCESS) {
        goto end4;
    }

    fisheye_calibrate_result_process(&frame_in, &frame_out);
    fisheye_calibrate_save_sp42x_to_planar(&frame_out.video_frame);
    printf("Calibrate Finished.....\n");

end4:
    ss_mpi_sys_munmap((td_void*)frame_out.video_frame.virt_addr[0], blk_size);
    ss_mpi_vb_release_blk(vb_blk);
end3:
    ss_mpi_vb_destroy_pool(h_pool);
end2:
    safe_close_file();
end1:
    fisheye_calibrate_restore(&dump_mem);
    fisheye_calibrate_recover_chn_attr(&chn_attr);
end0:
    return TD_SUCCESS;
}
