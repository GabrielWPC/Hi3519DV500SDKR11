/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "ot_math.h"
#include "ot_common.h"
#include "ot_common_video.h"
#include "ot_common_sys.h"
#include "ss_mpi_sys_mem.h"
#include "ot_buffer.h"
#include "ot_common_vb.h"
#include "ss_mpi_vb.h"
#include "ot_common_vi.h"
#include "ss_mpi_vi.h"
#include "ss_mpi_vgs.h"
#include "securec.h"
#include "ot_common_isp.h"
#include "ss_mpi_isp.h"

#define MAX_DUMP_FRAME_CNT      64
#define DUMP_FRAME_DEPTH        2
#define MAX_FRM_WIDTH           8192
#define FILE_NAME_LENGTH        128
#define PIXEL_FORMAT_STRING_LEN 10

typedef struct {
    ot_vb_blk         vb_blk;
    ot_vb_pool        vb_pool;
    td_u32            pool_id;

    td_phys_addr_t    phys_addr;
    td_void           *virt_addr;
} vi_dump_mem;

static td_u32 g_vi_depth_flag = 0;
static volatile sig_atomic_t g_signal_flag = 0;

static ot_vi_pipe g_vi_pipe = 0;
static ot_vi_chn g_vi_chn = 0;
static td_u32 g_orig_depth = 0;
static ot_video_frame_info g_frame;

static ot_vb_pool g_pool = OT_VB_INVALID_POOL_ID;
static vi_dump_mem g_dump_mem = { 0 };
static ot_vgs_handle g_handle = -1;
static td_u32 g_blk_size = 0;

static td_char *g_user_page_addr[2] = { TD_NULL, TD_NULL }; /* 2 Y and C */
static td_u32 g_size = 0;
static td_u32 g_c_size = 0;

static FILE *g_pfd = TD_NULL;
static ot_isp_dump_frame_pos g_dump_frame_pos = OT_ISP_DUMP_FRAME_POS_NORMAL;
static ot_pixel_format  g_ori_pix_format;
static ot_compress_mode g_ori_compress_mode;

static td_s32 vi_change_chn_frame_depth(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, td_u32 *orig_depth)
{
    if (vi_chn < OT_VI_MAX_PHYS_CHN_NUM) {
        ot_vi_chn_attr chn_attr;
        if (ss_mpi_vi_get_chn_attr(vi_pipe, vi_chn, &chn_attr) != TD_SUCCESS) {
            return TD_FAILURE;
        }

        *orig_depth = chn_attr.depth;
        chn_attr.depth = DUMP_FRAME_DEPTH;

        if (ss_mpi_vi_set_chn_attr(vi_pipe, vi_chn, &chn_attr) != TD_SUCCESS) {
            return TD_FAILURE;
        }
    } else {
        printf("vi_chn %d err\n", vi_chn);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}


static td_s32 vi_restore_chn_frame_default_depth(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn)
{
    if (vi_chn < OT_VI_MAX_PHYS_CHN_NUM) {
        ot_vi_chn_attr chn_attr;
        if (ss_mpi_vi_get_chn_attr(vi_pipe, vi_chn, &chn_attr) != TD_SUCCESS) {
            return TD_FAILURE;
        }
        chn_attr.depth = g_orig_depth;
        if (ss_mpi_vi_set_chn_attr(vi_pipe, vi_chn, &chn_attr) != TD_SUCCESS) {
            return TD_FAILURE;
        }
    } else {
        printf("vi_chn %d err\n", vi_chn);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void vi_chn_dump_covert_chroma_sp42x_to_planar(const ot_video_frame *frame, FILE *fd,
    td_u32 uv_height, td_bool is_uv_invert)
{
    /* If this value is too small and the image is big, this memory may not be enough */
    unsigned char tmp_buf[MAX_FRM_WIDTH];
    char *mem_content = TD_NULL;
    char *virt_addr_c = TD_NULL;
    td_u32 w, h;
    td_phys_addr_t phys_addr;

    phys_addr = frame->phys_addr[1];
    g_user_page_addr[1] = (td_char *)ss_mpi_sys_mmap(phys_addr, g_c_size);
    if (g_user_page_addr[1] == TD_NULL) {
        printf("mmap chroma data error!!!\n");
        return;
    }
    virt_addr_c = g_user_page_addr[1];

    (td_void)fflush(fd);
    /* save U */
    (td_void)fprintf(stderr, "U......");
    (td_void)fflush(stderr);
    for (h = 0; h < uv_height; h++) {
        mem_content = virt_addr_c + h * frame->stride[1];
        if (!is_uv_invert) {
            mem_content += 1;
        }

        for (w = 0; w < frame->width / 2; w++) { /* 2 chroma width */
            tmp_buf[w] = *mem_content;
            mem_content += 2; /* 2 semiplanar steps */
        }

        (td_void)fwrite(tmp_buf, frame->width / 2, 1, fd); /* 2 chroma width */
    }
    (td_void)fflush(fd);

    /* save V */
    (td_void)fprintf(stderr, "V......");
    (td_void)fflush(stderr);
    for (h = 0; h < uv_height; h++) {
        mem_content = virt_addr_c + h * frame->stride[1];
        if (is_uv_invert) {
            mem_content += 1;
        }

        for (w = 0; w < frame->width / 2; w++) { /* 2 chroma width */
            tmp_buf[w] = *mem_content;
            mem_content += 2; /* 2 semiplanar steps */
        }

        (td_void)fwrite(tmp_buf, frame->width / 2, 1, fd); /* 2 chroma width */
    }

    (td_void)fflush(fd);
    if (g_user_page_addr[1] != TD_NULL) {
        ss_mpi_sys_munmap(g_user_page_addr[1], g_c_size);
        g_user_page_addr[1] = TD_NULL;
    }
}

/* When saving a file, sp420 will be denoted by p420 and sp422 will be denoted by p422 in the name of the file */
static td_void sample_yuv_8bit_dump(ot_video_frame *frame, FILE *fd)
{
    unsigned int h;
    char *virt_addr_y = TD_NULL;
    char *mem_content = TD_NULL;
    td_phys_addr_t phys_addr;
    ot_pixel_format pixel_format = frame->pixel_format;
    /* When the storage format is a planar format, this variable is used to keep the height of the UV component */
    td_u32 uv_height = 0;
    td_bool is_uv_invert = (pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420 ||
        pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422) ? TD_TRUE : TD_FALSE;

    g_size = (frame->stride[0]) * (frame->height);
    if (pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420 || pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420) {
        g_c_size = (frame->stride[1]) * (frame->height) / 2; /* 2 uv height */
        uv_height = frame->height / 2; /* 2 uv height */
    } else if (pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422 ||
        pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422) {
        g_c_size = (frame->stride[1]) * (frame->height);
        uv_height = frame->height;
    } else if (pixel_format == OT_PIXEL_FORMAT_YUV_400) {
        g_c_size = 0;
        uv_height = frame->height;
    }

    phys_addr = frame->phys_addr[0];
    g_user_page_addr[0] = (td_char *)ss_mpi_sys_mmap(phys_addr, g_size);
    if (g_user_page_addr[0] == TD_NULL) {
        return;
    }

    virt_addr_y = g_user_page_addr[0];

    /* save Y */
    (td_void)fprintf(stderr, "saving......Y......");
    (td_void)fflush(stderr);

    for (h = 0; h < frame->height; h++) {
        mem_content = virt_addr_y + h * frame->stride[0];
        (td_void)fwrite(mem_content, frame->width, 1, fd);
    }

    (td_void)fflush(fd);
    if (pixel_format != OT_PIXEL_FORMAT_YUV_400) {
        vi_chn_dump_covert_chroma_sp42x_to_planar(frame, fd, uv_height, is_uv_invert);
    }

    (td_void)fprintf(stderr, "done %u!\n", frame->time_ref);
    (td_void)fflush(stderr);
    ss_mpi_sys_munmap(g_user_page_addr[0], g_size);
    g_user_page_addr[0] = NULL;
}

static td_void vi_restore_dump_be_frame_attr(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn)
{
    ot_vi_chn_attr chn_attr;
    ot_isp_be_frame_attr be_frame_attr;

    if (g_ori_pix_format != OT_PIXEL_FORMAT_BUTT) {
        ss_mpi_vi_get_chn_attr(vi_pipe, vi_chn, &chn_attr);
        chn_attr.compress_mode = g_ori_compress_mode;
        chn_attr.pixel_format  = g_ori_pix_format;
        ss_mpi_vi_set_chn_attr(vi_pipe, vi_chn, &chn_attr);
    }

    be_frame_attr.frame_pos = OT_ISP_DUMP_FRAME_POS_NORMAL;
    ss_mpi_isp_set_be_frame_attr(vi_pipe, &be_frame_attr);
}

static td_void vi_restore(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn)
{
    td_s32 ret;

    if (g_frame.pool_id != OT_VB_INVALID_POOL_ID) {
        ret = ss_mpi_vi_release_chn_frame(vi_pipe, vi_chn, &g_frame);
        if (ret != TD_SUCCESS) {
            printf("Release Chn Frame error!!!\n");
        }
        g_frame.pool_id = OT_VB_INVALID_POOL_ID;
    }

    if (g_handle != -1) {
        ss_mpi_vgs_cancel_job(g_handle);
        g_handle = -1;
    }

    if (g_dump_mem.vb_pool != OT_VB_INVALID_POOL_ID) {
        ss_mpi_vb_release_blk(g_dump_mem.vb_blk);
        g_dump_mem.vb_pool = OT_VB_INVALID_POOL_ID;
    }

    if (g_pool != OT_VB_INVALID_POOL_ID) {
        ss_mpi_vb_destroy_pool(g_pool);
        g_pool = OT_VB_INVALID_POOL_ID;
    }

    if (g_user_page_addr[0] != TD_NULL) {
        ss_mpi_sys_munmap(g_user_page_addr[0], g_size);
        g_user_page_addr[0] = TD_NULL;
    }
    if (g_user_page_addr[1] != TD_NULL) {
        ss_mpi_sys_munmap(g_user_page_addr[1], g_size);
        g_user_page_addr[1] = TD_NULL;
    }

    if (g_pfd != TD_NULL) {
        (td_void)fclose(g_pfd);
        g_pfd = TD_NULL;
    }
    if (g_dump_frame_pos != OT_ISP_DUMP_FRAME_POS_NORMAL) {
        vi_restore_dump_be_frame_attr(vi_pipe, vi_chn);
        g_dump_frame_pos = OT_ISP_DUMP_FRAME_POS_NORMAL;
    }

    if (g_vi_depth_flag) {
        ret = vi_restore_chn_frame_default_depth(vi_pipe, vi_chn);
        if (ret != TD_SUCCESS) {
            printf("restore chn depth error!!!\n");
        }
        g_vi_depth_flag = 0;
    }
}

static void vi_chn_dump_handle_sig(td_s32 signo)
{
    if (g_signal_flag) {
        return;
    }

    if (signo == SIGINT || signo == SIGTERM) {
        g_signal_flag = 1;
    }

    return;
}

static td_char *get_pixel_format_str(ot_pixel_format pixel_format)
{
    switch (pixel_format) {
        case OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422:
            return "P422";
        case OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420:
            return "P420";
        case OT_PIXEL_FORMAT_YUV_400:
            return "P400";
        case OT_PIXEL_FORMAT_RGB_BAYER_8BPP:
            return "Raw8";
        case OT_PIXEL_FORMAT_RGB_BAYER_10BPP:
            return "Raw10";
        case OT_PIXEL_FORMAT_RGB_BAYER_12BPP:
            return "Raw12";
        case OT_PIXEL_FORMAT_RGB_BAYER_14BPP:
            return "Raw14";
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP:
            return "Raw16";
        default:
            return "na";
    }
}

static td_char *get_file_suffix_name(ot_pixel_format pixel_format)
{
    switch (pixel_format) {
        case OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422:
        case OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420:
        case OT_PIXEL_FORMAT_YUV_400:
            return "yuv";
        case OT_PIXEL_FORMAT_RGB_BAYER_8BPP:
        case OT_PIXEL_FORMAT_RGB_BAYER_10BPP:
        case OT_PIXEL_FORMAT_RGB_BAYER_12BPP:
        case OT_PIXEL_FORMAT_RGB_BAYER_14BPP:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP:
            return "raw";
        default:
            return "na";
    }
}

static td_s32 vi_chn_dump_make_frame_file_name(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, td_u32 frame_cnt)
{
    td_char yuv_name[FILE_NAME_LENGTH];

    /* make file name */
    if (snprintf_s(yuv_name, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./vi_pipe%d_chn%d_%ux%u_%s_%u.%s",
        vi_pipe, vi_chn, g_frame.video_frame.width, g_frame.video_frame.height,
        get_pixel_format_str(g_frame.video_frame.pixel_format),
        frame_cnt,
        get_file_suffix_name(g_frame.video_frame.pixel_format)) == -1) {
        printf("set output file name failed!\n");
        return TD_FAILURE;
    }

    printf("Dump YUV frame of vi pipe %d chn %d to file: \"%s\"\n", vi_pipe, vi_chn, yuv_name);

    /* open file */
    g_pfd = fopen(yuv_name, "wb");
    if (g_pfd == TD_NULL) {
        printf("open file failed:%s!\n", strerror(errno));
        return TD_FAILURE;
    }
    (td_void)fflush(stdout);
    return TD_SUCCESS;
}

static td_void vi_chn_dump_set_vgs_frame_info(ot_video_frame_info *vgs_frame_info, const vi_dump_mem *dump_mem,
    const ot_vb_calc_cfg *vb_calc_cfg, const ot_video_frame_info *vi_frame_info)
{
    vgs_frame_info->video_frame.phys_addr[0]  = dump_mem->phys_addr;
    vgs_frame_info->video_frame.phys_addr[1]  = vgs_frame_info->video_frame.phys_addr[0] + vb_calc_cfg->main_y_size;
    vgs_frame_info->video_frame.width         = vi_frame_info->video_frame.width;
    vgs_frame_info->video_frame.height        = vi_frame_info->video_frame.height;
    vgs_frame_info->video_frame.stride[0]     = vb_calc_cfg->main_stride;
    vgs_frame_info->video_frame.stride[1]     = vb_calc_cfg->main_stride;
    vgs_frame_info->video_frame.compress_mode = OT_COMPRESS_MODE_NONE;
    vgs_frame_info->video_frame.pixel_format  = vi_frame_info->video_frame.pixel_format;
    vgs_frame_info->video_frame.video_format  = OT_VIDEO_FORMAT_LINEAR;
    vgs_frame_info->video_frame.dynamic_range = vi_frame_info->video_frame.dynamic_range;
    vgs_frame_info->video_frame.pts           = 0;
    vgs_frame_info->video_frame.time_ref      = 0;
    vgs_frame_info->pool_id                   = dump_mem->vb_pool;
    vgs_frame_info->mod_id                    = OT_ID_VGS;
}

static td_s32 vi_chn_dump_init_vgs_pool(vi_dump_mem *dump_mem, ot_vb_calc_cfg *vb_calc_cfg)
{
    td_u32 width = g_frame.video_frame.width;
    td_u32 height = g_frame.video_frame.height;
    ot_pixel_format pixel_format = g_frame.video_frame.pixel_format;
    ot_pic_buf_attr buf_attr = {0};
    ot_vb_pool_cfg vb_pool_cfg = {0};

    buf_attr.width         = width;
    buf_attr.height        = height;
    buf_attr.pixel_format  = pixel_format;
    buf_attr.bit_width     = OT_DATA_BIT_WIDTH_8;
    buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    buf_attr.align         = 0;
    ot_common_get_pic_buf_cfg(&buf_attr, vb_calc_cfg);

    g_blk_size = vb_calc_cfg->vb_size;

    vb_pool_cfg.blk_size   = g_blk_size;
    vb_pool_cfg.blk_cnt    = 1;
    vb_pool_cfg.remap_mode = OT_VB_REMAP_MODE_NONE;
    g_pool = ss_mpi_vb_create_pool(&vb_pool_cfg);
    if (g_pool == OT_VB_INVALID_POOL_ID) {
        printf("OT_MPI_VB_CreatePool failed! \n");
        return TD_FAILURE;
    }

    dump_mem->vb_pool = g_pool;
    dump_mem->vb_blk = ss_mpi_vb_get_blk(dump_mem->vb_pool, g_blk_size, TD_NULL);
    if (dump_mem->vb_blk == OT_VB_INVALID_HANDLE) {
        printf("get vb blk failed!\n");
        return TD_FAILURE;
    }

    dump_mem->phys_addr = ss_mpi_vb_handle_to_phys_addr(dump_mem->vb_blk);
    return TD_SUCCESS;
}

static td_s32 vi_chn_dump_send_vgs_and_dump(const ot_video_frame_info *vi_frame_info)
{
    ot_video_frame_info vgs_frame_info = {0};
    ot_vgs_task_attr vgs_task_attr;
    ot_vb_calc_cfg vb_calc_cfg = {0};

    if (vi_chn_dump_init_vgs_pool(&g_dump_mem, &vb_calc_cfg) != TD_SUCCESS) {
        printf("init vgs pool failed\n");
        return TD_FAILURE;
    }

    vi_chn_dump_set_vgs_frame_info(&vgs_frame_info, &g_dump_mem, &vb_calc_cfg, vi_frame_info);

    if (ss_mpi_vgs_begin_job(&g_handle) != TD_SUCCESS) {
        printf("ss_mpi_vgs_begin_job failed\n");
        return TD_FAILURE;
    }

    if (memcpy_s(&vgs_task_attr.img_in, sizeof(ot_video_frame_info),
        vi_frame_info, sizeof(ot_video_frame_info)) != EOK) {
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

    /* save frame to file */
    if (vgs_frame_info.video_frame.dynamic_range == OT_DYNAMIC_RANGE_SDR8) {
        sample_yuv_8bit_dump(&vgs_frame_info.video_frame, g_pfd);
    } else {
        printf("Only support dump 8-bit data!\n");
    }
    ss_mpi_vb_release_blk(g_dump_mem.vb_blk);
    g_dump_mem.vb_pool = OT_VB_INVALID_POOL_ID;
    if (g_pool != OT_VB_INVALID_POOL_ID) {
        ss_mpi_vb_destroy_pool(g_pool);
        g_pool = OT_VB_INVALID_POOL_ID;
    }
    g_handle = -1;
    return TD_SUCCESS;
err_exit:
    ss_mpi_vgs_cancel_job(g_handle);
    g_handle = -1;
    return TD_FAILURE;
}

static td_u32 vi_get_raw_bit_width(ot_pixel_format pixel_format)
{
    td_u32 bit_width;

    switch (pixel_format) {
        case OT_PIXEL_FORMAT_RGB_BAYER_8BPP:
            bit_width = 8;  /* 8: 8bit */
            break;
        case OT_PIXEL_FORMAT_RGB_BAYER_10BPP:
            bit_width = 10; /* 10: 10bit */
            break;
        case OT_PIXEL_FORMAT_RGB_BAYER_12BPP:
            bit_width = 12; /* 12: 12bit */
            break;
        case OT_PIXEL_FORMAT_RGB_BAYER_14BPP:
            bit_width = 14; /* 14: 14bit */
            break;
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP:
            bit_width = 16; /* 16: 16bit */
            break;
        default:
            bit_width = 8;  /* 8: 8bit */
            break;
    }

    return bit_width;
}

static td_s32 vi_raw_convert_bit_pixel(const td_u8 *data, td_u32 data_num, td_u32 bit_width, td_u16 *out_data)
{
    td_s32 i, tmp_data_num, out_cnt;
    td_u32 u32_val;
    td_u64 u64_val;
    const td_u8 *tmp_data = data;

    out_cnt = 0;
    switch (bit_width) {
        case 10: /* 10: 10bit */
            tmp_data_num = data_num / 4; /* 4 pixels consist of 5 bytes  */
            for (i = 0; i < tmp_data_num; i++) {
                tmp_data = data + 5 * i; /* 5: include 5bytes */
                /* 0/8/16/24/32: byte align */
                u64_val = tmp_data[0] + ((td_u32)tmp_data[1] << 8) + ((td_u32)tmp_data[2] << 16) +
                    ((td_u32)tmp_data[3] << 24) + ((td_u64)tmp_data[4] << 32); /* 3/4: index, 24/32: align */

                out_data[out_cnt++] = (td_u16)((u64_val >> 0)  & 0x3ff); /* 0:  10 bit align */
                out_data[out_cnt++] = (td_u16)((u64_val >> 10) & 0x3ff); /* 10: 10 bit align */
                out_data[out_cnt++] = (td_u16)((u64_val >> 20) & 0x3ff); /* 20: 10 bit align */
                out_data[out_cnt++] = (td_u16)((u64_val >> 30) & 0x3ff); /* 30: 10 bit align */
            }
            break;
        case 12: /* 12: 12bit */
            tmp_data_num = data_num / 2; /* 2 pixels consist of 3 bytes  */
            for (i = 0; i < tmp_data_num; i++) {
                tmp_data = data + 3 * i; /* 3: include 3bytes */
                u32_val = tmp_data[0] + (tmp_data[1] << 8) + (tmp_data[2] << 16); /* 1/2: index, 8/16: align */
                out_data[out_cnt++] = (td_u16)(u32_val & 0xfff);
                out_data[out_cnt++] = (td_u16)((u32_val >> 12) & 0xfff); /* 12: 12 bit align */
            }
            break;
        case 14: /* 14: 14bit */
            tmp_data_num = data_num / 4; /* 4 pixels consist of 7 bytes  */
            for (i = 0; i < tmp_data_num; i++) {
                tmp_data = data + 7 * i; /* 7: include 7bytes */
                u64_val = tmp_data[0] +
                    ((td_u32)tmp_data[1] <<  8) + ((td_u32)tmp_data[2] << 16) + /* 1/2: index, 8/16:  align */
                    ((td_u32)tmp_data[3] << 24) + ((td_u64)tmp_data[4] << 32) + /* 3/4: index, 24/32: align */
                    ((td_u64)tmp_data[5] << 40) + ((td_u64)tmp_data[6] << 48);  /* 5/6: index, 40/48: align */

                out_data[out_cnt++] = (td_u16)((u64_val >> 0)  & 0x3fff); /* 0:  14 bit align */
                out_data[out_cnt++] = (td_u16)((u64_val >> 14) & 0x3fff); /* 14: 14 bit align */
                out_data[out_cnt++] = (td_u16)((u64_val >> 28) & 0x3fff); /* 28: 14 bit align */
                out_data[out_cnt++] = (td_u16)((u64_val >> 42) & 0x3fff); /* 42: 14 bit align */
            }
            break;
        default:
            printf("unsuport bit_width: %u\n", bit_width);
            return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void vi_save_raw_file(const ot_video_frame *v_buf, td_u32 byte_align, FILE *pfd)
{
    td_u32 height;
    td_phys_addr_t phys_addr;
    td_u32 size;
    td_u8 *virt_addr;
    td_u16 *u16_data = TD_NULL;
    td_u8 *u8_data = TD_NULL;
    td_u32 nbit = vi_get_raw_bit_width(v_buf->pixel_format);

    size = (v_buf->stride[0]) * (v_buf->height);
    phys_addr = v_buf->phys_addr[0];

    virt_addr = (td_u8 *)ss_mpi_sys_mmap(phys_addr, size);
    if (virt_addr == TD_NULL) {
        printf("ss_mpi_sys_mmap failed!\n");
        return;
    }

    u8_data = virt_addr;
    if ((nbit != 8) && (nbit != 16)) { /* 8/16 : bit width */
        u16_data = (td_u16 *)malloc(v_buf->width * 2); /* 2: 2bytes */
        if (u16_data == TD_NULL) {
            printf("malloc memory failed\n");
            goto exit;
        }
    }

    printf("saving......raw data......stride[0]: %u, width: %u, height: %u\n",
        v_buf->stride[0], v_buf->width, v_buf->height);

    for (height = 0; height < v_buf->height; height++) {
        /* 8/16: not equal 8 or 16 need byte align */
        if ((nbit != 8) && (nbit != 16) && (byte_align == 1)) {
            vi_raw_convert_bit_pixel(u8_data, v_buf->width, nbit, u16_data);
            (td_void)fwrite(u16_data, v_buf->width, 2, pfd); /* 2: 2bytes */
        } else {
            td_u32 width_bytes = (v_buf->width * nbit + 7) / 8; /* 7/8: align */
            (td_void)fwrite(u8_data, width_bytes, 1, pfd);
        }
        u8_data += v_buf->stride[0];
    }
    (td_void)fflush(pfd);

    printf("done time_ref: %u!\n", v_buf->time_ref);

exit:
    if (u16_data != TD_NULL) {
        free(u16_data);
    }
    ss_mpi_sys_munmap(virt_addr, size);
    virt_addr = TD_NULL;
}

static td_s32 vi_chn_dump_get_frame(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, td_u32 frame_cnt)
{
    td_s32 ret;
    td_u32 cnt = frame_cnt;
    td_s32 milli_sec = -1;
    td_bool is_send_to_vgs;

    /* get frame */
    while (cnt--) {
        if (g_signal_flag == 1) {
            printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
            return TD_FAILURE;
        }
        if (ss_mpi_vi_get_chn_frame(vi_pipe, vi_chn, &g_frame, milli_sec) != TD_SUCCESS) {
            printf("Get frame fail \n");
            usleep(1000); /* 1000 sleep */
            continue;
        }

        is_send_to_vgs = ((g_frame.video_frame.compress_mode != OT_COMPRESS_MODE_NONE) ||
            (g_frame.video_frame.video_format != OT_VIDEO_FORMAT_LINEAR));

        if (is_send_to_vgs) {
            if (vi_chn_dump_send_vgs_and_dump(&g_frame) != TD_SUCCESS) {
                ss_mpi_vi_release_chn_frame(vi_pipe, vi_chn, &g_frame);
                g_frame.pool_id = OT_VB_INVALID_POOL_ID;
                return TD_FAILURE;
            }
        } else {
            if (g_frame.video_frame.pixel_format == OT_PIXEL_FORMAT_YUV_400 ||
                g_frame.video_frame.pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420 ||
                g_frame.video_frame.pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422) {
                sample_yuv_8bit_dump(&g_frame.video_frame, g_pfd);
            } else {
                vi_save_raw_file(&g_frame.video_frame, 1, g_pfd);
            }
        }

        printf("Get frame %u!!\n", cnt);
        /* release frame after using */
        ret = ss_mpi_vi_release_chn_frame(vi_pipe, vi_chn, &g_frame);
        if (ret != TD_SUCCESS) {
            printf("Release frame error ,now exit !!!\n");
            return ret;
        }
        g_frame.pool_id = OT_VB_INVALID_POOL_ID;
    }
    return TD_SUCCESS;
}

static td_s32 vi_set_be_dump_frame_attr(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn)
{
    td_s32 ret;
    td_s32 milli_sec = -1;
    td_s32 try_times = 10;
    ot_isp_be_frame_attr be_frame_attr;
    ot_vi_chn_attr       chn_attr;

    while (ss_mpi_vi_get_chn_frame(vi_pipe, vi_chn, &g_frame, milli_sec) != TD_SUCCESS) {
        try_times--;
        if (try_times <= 0) {
            printf("get frame error for 10 times,now exit !!!\n");
            return TD_FAILURE;
        }
        usleep(40000); /* 40000 sleep */
    }

    ret = ss_mpi_vi_release_chn_frame(vi_pipe, vi_chn, &g_frame);
    if (ret != TD_SUCCESS) {
        printf("Release frame error, now exit !!!\n");
        return TD_FAILURE;
    }

    ss_mpi_vi_get_chn_attr(vi_pipe, vi_chn, &chn_attr);
    g_ori_pix_format    = chn_attr.pixel_format;
    g_ori_compress_mode = chn_attr.compress_mode;

    chn_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    chn_attr.pixel_format  = OT_PIXEL_FORMAT_RGB_BAYER_16BPP;
    ret = ss_mpi_vi_set_chn_attr(vi_pipe, vi_chn, &chn_attr);
    if (ret != TD_SUCCESS) {
        printf("set vi chn attr error, now exit !!!\n");
        return ret;
    }

    be_frame_attr.frame_pos = g_dump_frame_pos;
    ret = ss_mpi_isp_set_be_frame_attr(vi_pipe, &be_frame_attr);
    if (ret != TD_SUCCESS) {
        printf("set be frame attr error, now exit !!!\n");
        return ret;
    }
    sleep(1);

    return TD_SUCCESS;
}

static td_s32 vi_chn_dump_try_get_frame(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, td_u32 frame_cnt)
{
    td_s32 milli_sec = -1;
    td_s32 try_times = 10;
    td_s32 ret;

    if (g_dump_frame_pos != OT_ISP_DUMP_FRAME_POS_NORMAL) {
        ret = vi_set_be_dump_frame_attr(vi_pipe, vi_chn);
        if (ret != TD_SUCCESS) {
            return TD_FAILURE;
        }
    }

    while (ss_mpi_vi_get_chn_frame(vi_pipe, vi_chn, &g_frame, milli_sec) != TD_SUCCESS) {
        if (g_signal_flag == 1) {
            printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
            return TD_FAILURE;
        }
        try_times--;
        if (try_times <= 0) {
            printf("get frame error for 10 times,now exit !!!\n");
            return TD_FAILURE;
        }
        usleep(40000); /* 40000 sleep */
    }

    if (vi_chn_dump_make_frame_file_name(vi_pipe, vi_chn, frame_cnt) != TD_SUCCESS) {
        ss_mpi_vi_release_chn_frame(vi_pipe, vi_chn, &g_frame);
        g_frame.pool_id = OT_VB_INVALID_POOL_ID;
        return TD_FAILURE;
    }

    ret = ss_mpi_vi_release_chn_frame(vi_pipe, vi_chn, &g_frame);
    if (ret != TD_SUCCESS) {
        printf("Release frame error ,now exit !!!\n");
        return TD_FAILURE;
    }

    g_frame.pool_id = OT_VB_INVALID_POOL_ID;

    return TD_SUCCESS;
}

static td_void vi_do_chn_dump_frame(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, td_u32 frame_cnt)
{
    td_s32 ret;

    ret = vi_change_chn_frame_depth(vi_pipe, vi_chn, &g_orig_depth);
    if (ret != TD_SUCCESS) {
        printf("set chn dump depth failed!\n");
        return;
    }

    g_vi_depth_flag = 1;

    if (memset_s(&g_frame, sizeof(ot_video_frame_info), 0, sizeof(ot_video_frame_info)) != EOK) {
        printf("memset_s frame error!!!\n");
        goto exit;
    }
    g_frame.pool_id = OT_VB_INVALID_POOL_ID;

    if (vi_chn_dump_try_get_frame(vi_pipe, vi_chn, frame_cnt) != TD_SUCCESS) {
        goto exit;
    }

    if (vi_chn_dump_get_frame(vi_pipe, vi_chn, frame_cnt) != TD_SUCCESS) {
        goto exit;
    }

exit:
    vi_restore(vi_pipe, vi_chn);
}

static td_void vi_chn_dump_init_global_params(td_void)
{
    g_vi_depth_flag     = 0;
    g_signal_flag       = 0;
    g_user_page_addr[0] = TD_NULL;
    g_user_page_addr[1] = TD_NULL;
    g_frame.pool_id     = OT_VB_INVALID_POOL_ID;
    g_orig_depth        = 0;
    g_pool              = OT_VB_INVALID_POOL_ID;
    g_handle            = -1;
    g_blk_size          = 0;
    g_size              = 0;
    g_pfd               = TD_NULL;
    g_ori_pix_format    = OT_PIXEL_FORMAT_BUTT;
}

static td_void tool_usage(void)
{
    printf(
        "\n"
        "**********************************************************\n"
        "usage: ./vi_chn_dump [vi_pipe] [vi_chn] [frame_cnt] [dump_frame_pos]\n"
        "1)vi_pipe: \n"
        "   vi pipe id\n"
        "2)vi_chn: \n"
        "   vi chn id\n"
        "3)frame_cnt: \n"
        "   the count of frame to be dump\n"
        "4)dump_frame_pos: \n"
        "   the position of frame to be dump(only vi_chn 0 support dump raw after wdr\n"
        "       0: dump frame after all isp module. default\n"
        "       1: dump raw after wdr module\n"
        "*)example:\n"
        "   e.g : ./vi_chn_dump 0 0 1 1\n"
        "   e.g : ./vi_chn_dump 1 2 2 0\n"
        "**********************************************************\n"
        "\n");
}

static const td_char *vi_chn_dump_get_argv_name(td_s32 index)
{
    const td_char *argv_name[4] = {"vi_pipe", "vi_chn", "frame_cnt", "dump_frame_pos"}; /* 4: arg nums */

    if (index >= 4 || index < 0) { /* 4: arg nums */
        return "-";
    }

    return argv_name[index];
}

static td_s32 vi_chn_dump_get_argv_val(char *argv[], td_s32 index, td_s32 min_val, td_s32 max_val, td_s32 *val)
{
    td_char *end_ptr = TD_NULL;
    td_s32 result;
    const td_s32 base = 10; /* 10 number system */

    errno = 0;
    result = (td_s32)strtol(argv[index], &end_ptr, base);
    if ((end_ptr == argv[index]) || (*end_ptr != '\0')) {
        return TD_FAILURE;
    }
    if ((errno == ERANGE) || (errno != 0 && result == 0)) {
        return TD_FAILURE;
    }

    if ((result < min_val) || (result > max_val)) {
        printf("Failure: input arg_index(%d) arg_name(%s) arg_val (%d) is wrong. should be [%d, %d]!\n",
            index, vi_chn_dump_get_argv_name(index - 1), result, min_val, max_val);
        return TD_FAILURE;
    }

    *val = result;
    return TD_SUCCESS;
}

#ifdef __LITEOS__
td_s32 vi_chn_dump(int argc, char *argv[])
#else
td_s32 main(int argc, char *argv[])
#endif
{
    td_s32 frame_cnt;
    td_s32 dump_pos;
    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("\tTo see more usage, please enter: ./vi_chn_dump -h\n\n");

    if ((argc > 1) && !strncmp(argv[1], "-h", 2)) { /* 2 help */
        tool_usage();
        return TD_SUCCESS;
    }

    if ((argc != 4) && (argc != 5)) { /* 4/5: arg num */
        tool_usage();
        return TD_FAILURE;
    }

    if (vi_chn_dump_get_argv_val(argv, 1, 0, OT_VI_MAX_PIPE_NUM - 1, &g_vi_pipe) != TD_SUCCESS) { /* arg 1 */
        tool_usage();
        return TD_FAILURE;
    }

    if (vi_chn_dump_get_argv_val(argv, 2, 0, OT_VI_MAX_CHN_NUM - 1, &g_vi_chn) != TD_SUCCESS) { /* arg 2 */
        tool_usage();
        return TD_FAILURE;
    }

    if (vi_chn_dump_get_argv_val(argv, 3, 0, MAX_DUMP_FRAME_CNT, &frame_cnt) != TD_SUCCESS) { /* arg 3 */
        tool_usage();
        return TD_FAILURE;
    }

    if (argc == 5) { /* 5: arg num */
        /* arg 4 */
        if (vi_chn_dump_get_argv_val(argv, 4, 0, OT_ISP_DUMP_FRAME_POS_BUTT - 1, &dump_pos) != TD_SUCCESS) {
            tool_usage();
            return TD_FAILURE;
        }

        g_dump_frame_pos = (dump_pos == 0) ? OT_ISP_DUMP_FRAME_POS_NORMAL : OT_ISP_DUMP_FRAME_POS_AFTER_WDR;

        if ((g_vi_chn >= OT_VI_MAX_PHYS_CHN_NUM) && (g_dump_frame_pos == OT_ISP_DUMP_FRAME_POS_AFTER_WDR)) {
            printf("Only vi_phys_chn support dump raw after wdr module.\n");
            return TD_FAILURE;
        }
    }

    vi_chn_dump_init_global_params();

#ifndef __LITEOS__
    (td_void)signal(SIGINT, vi_chn_dump_handle_sig);
    (td_void)signal(SIGTERM, vi_chn_dump_handle_sig);
#endif

    vi_do_chn_dump_frame(g_vi_pipe, g_vi_chn, frame_cnt);

    return TD_SUCCESS;
}
