/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "ot_math.h"
#include "ot_common.h"
#include "securec.h"
#include "ot_common_video.h"
#include "ot_common_sys.h"
#include "ss_mpi_sys_mem.h"
#include "ot_common_vb.h"
#include "ss_mpi_vb.h"
#include "ot_common_vpss.h"
#include "ss_mpi_vpss.h"
#include "ss_mpi_vgs.h"
#include "ot_buffer.h"

#define MAX_DIGIT_LEN 4
#define MAX_FRM_WIDTH 20480
#define FILE_NAME_LENGTH 128
#define PIXEL_FORMAT_STRING_LEN 10
#define VPSS_ATTR_ARGV_BASE 10

typedef struct {
    ot_vb_blk vb_blk;
    ot_vb_pool vb_pool;
    td_u32 pool_id;

    td_phys_addr_t phys_addr;
    td_void *virt_addr;
} vpss_dump_mem;

ot_video_frame_info g_frame;
vpss_dump_mem g_mem = { 0 };
static ot_vpss_grp g_vpss_grp = 0;
ot_vb_pool g_pool = OT_VB_INVALID_POOL_ID;
ot_vgs_handle g_handle = -1;
FILE *g_pfd = TD_NULL;
td_u32 g_blk_size = 0;
static td_char *g_user_page_addr[2] = {TD_NULL, TD_NULL}; /* 2 Y and C */
static volatile sig_atomic_t g_signal_flag = 0;
static td_u32 g_ysize, g_c_size;

static td_void vpss_src_dump_covert_chroma_sp42x_to_planar(const ot_video_frame *frame, FILE *fd, td_u32 uv_height)
{
    /* If this value is too small and the image is big, this memory may not be enough */
    static unsigned char tmp_buf[MAX_FRM_WIDTH];
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
        mem_content += 1;

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

/* When saving a file,sp420 will be denoted by p420 and sp422 will be denoted by p422 in the name of the file */
static void sample_yuv_8bit_dump(const ot_video_frame *video_frame_buf, FILE *pfd)
{
    unsigned int h;
    char *buf_virt_y = TD_NULL;
    char *mem_content = TD_NULL;
    ot_pixel_format pixel_format = video_frame_buf->pixel_format;
    /* When the storage format is a planar format, this variable is used to keep the height of the UV component */
    td_u32 uv_height;

    g_ysize = (video_frame_buf->stride[0]) * (video_frame_buf->height);

    if (pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420 || pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420) {
        g_c_size = (video_frame_buf->stride[1]) * (video_frame_buf->height) / 2; /* 2 uv height */
        uv_height = video_frame_buf->height / 2;                                /* 2 uv height */
    } else if (pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422 ||
        pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422) {
        g_c_size = (video_frame_buf->stride[1]) * (video_frame_buf->height);
        uv_height = video_frame_buf->height;
    } else {
        g_c_size = 0;
        uv_height = 0;
    }

    g_user_page_addr[0] = (td_char *)ss_mpi_sys_mmap(video_frame_buf->phys_addr[0], g_ysize);
    if (g_user_page_addr[0] == TD_NULL) {
        return;
    }
    buf_virt_y = g_user_page_addr[0];

    /* save Y */
    (td_void)fprintf(stderr, "saving......Y......");
    (td_void)fflush(stderr);

    for (h = 0; h < video_frame_buf->height; h++) {
        mem_content = buf_virt_y + h * video_frame_buf->stride[0];
        (td_void)fwrite(mem_content, video_frame_buf->width, 1, pfd);
    }

    (td_void)fflush(pfd);
    if (pixel_format != OT_PIXEL_FORMAT_YUV_400) {
        vpss_src_dump_covert_chroma_sp42x_to_planar(video_frame_buf, pfd, uv_height);
    }

    (td_void)fprintf(stderr, "done %u!\n", video_frame_buf->time_ref);
    (td_void)fflush(stderr);

    ss_mpi_sys_munmap(g_user_page_addr[0], g_ysize);
    g_user_page_addr[0] = TD_NULL;
}

static td_void vpss_restore(ot_vpss_grp grp)
{
    if (g_frame.pool_id != OT_VB_INVALID_POOL_ID) {
        ss_mpi_vpss_release_grp_frame(grp, &g_frame);
        g_frame.pool_id = OT_VB_INVALID_POOL_ID;
    }

    if (g_handle != -1) {
        ss_mpi_vgs_cancel_job(g_handle);
        g_handle = -1;
    }

    if (g_user_page_addr[0] != TD_NULL) {
        ss_mpi_sys_munmap(g_user_page_addr[0], g_ysize);
        g_user_page_addr[0] = TD_NULL;
    }

    if (g_user_page_addr[1] != TD_NULL) {
        ss_mpi_sys_munmap(g_user_page_addr[1], g_c_size);
        g_user_page_addr[1] = TD_NULL;
    }

    if (g_mem.vb_pool != OT_VB_INVALID_POOL_ID) {
        ss_mpi_vb_release_blk(g_mem.vb_blk);
        g_mem.vb_pool = OT_VB_INVALID_POOL_ID;
    }

    if (g_pool != OT_VB_INVALID_POOL_ID) {
        ss_mpi_vb_destroy_pool(g_pool);
        g_pool = OT_VB_INVALID_POOL_ID;
    }

    if (g_pfd != TD_NULL) {
        fclose(g_pfd);
        g_pfd = TD_NULL;
    }
    return;
}

static void vpss_src_dump_handle_sig(int signo)
{
    if (g_signal_flag) {
        return;
    }

    if (signo == SIGINT || signo == SIGTERM) {
        g_signal_flag = 1;
    }
    return;
}

static td_void vpss_src_dump_set_vgs_frame_info(ot_video_frame_info *vgs_frame_info, const vpss_dump_mem *dump_mem,
    const ot_vb_calc_cfg *vb_calc_cfg, const ot_video_frame_info *vpss_frame_info)
{
    vgs_frame_info->video_frame.phys_addr[0] = dump_mem->phys_addr;
    vgs_frame_info->video_frame.phys_addr[1] = vgs_frame_info->video_frame.phys_addr[0] + vb_calc_cfg->main_y_size;
    vgs_frame_info->video_frame.width = vpss_frame_info->video_frame.width;
    vgs_frame_info->video_frame.height = vpss_frame_info->video_frame.height;
    vgs_frame_info->video_frame.stride[0] = vb_calc_cfg->main_stride;
    vgs_frame_info->video_frame.stride[1] = vb_calc_cfg->main_stride;
    vgs_frame_info->video_frame.compress_mode = OT_COMPRESS_MODE_NONE;
    vgs_frame_info->video_frame.pixel_format = vpss_frame_info->video_frame.pixel_format;
    vgs_frame_info->video_frame.video_format = OT_VIDEO_FORMAT_LINEAR;
    vgs_frame_info->video_frame.dynamic_range = vpss_frame_info->video_frame.dynamic_range;
    vgs_frame_info->video_frame.pts = 0;
    vgs_frame_info->video_frame.time_ref = 0;
    vgs_frame_info->pool_id = dump_mem->vb_pool;
    vgs_frame_info->mod_id = OT_ID_VGS;
}

static td_s32 vpss_src_dump_init_vgs_pool(vpss_dump_mem *dump_mem, ot_vb_calc_cfg *vb_calc_cfg)
{
    td_u32 width = g_frame.video_frame.width;
    td_u32 height = g_frame.video_frame.height;
    ot_pixel_format pixel_format = g_frame.video_frame.pixel_format;
    ot_pic_buf_attr buf_attr = {0};
    ot_vb_pool_cfg vb_pool_cfg = {0};

    buf_attr.width = width;
    buf_attr.height = height;
    buf_attr.pixel_format = pixel_format;
    buf_attr.bit_width = OT_DATA_BIT_WIDTH_8;
    buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    buf_attr.align = 0;
    ot_common_get_pic_buf_cfg(&buf_attr, vb_calc_cfg);

    g_blk_size = vb_calc_cfg->vb_size;

    vb_pool_cfg.blk_size = g_blk_size;
    vb_pool_cfg.blk_cnt = 1;
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

static td_s32 vpss_src_dump_send_vgs_and_dump(const ot_video_frame_info *vpss_frame_info)
{
    ot_video_frame_info vgs_frame_info = {0};
    ot_vgs_task_attr vgs_task_attr;
    ot_vb_calc_cfg vb_calc_cfg = {0};

    if (vpss_src_dump_init_vgs_pool(&g_mem, &vb_calc_cfg) != TD_SUCCESS) {
        printf("init vgs pool failed\n");
        return TD_FAILURE;
    }

    vpss_src_dump_set_vgs_frame_info(&vgs_frame_info, &g_mem, &vb_calc_cfg, vpss_frame_info);

    if (ss_mpi_vgs_begin_job(&g_handle) != TD_SUCCESS) {
        printf("ss_mpi_vgs_begin_job failed\n");
        return TD_FAILURE;
    }

    if (memcpy_s(&vgs_task_attr.img_in, sizeof(ot_video_frame_info),
        vpss_frame_info, sizeof(ot_video_frame_info)) != EOK) {
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
    ss_mpi_vb_release_blk(g_mem.vb_blk);
    g_mem.vb_pool = OT_VB_INVALID_POOL_ID;
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

static td_s32 vpss_src_dump_open_yuv_file(ot_vpss_grp grp, const ot_video_frame *frame)
{
    td_char yuv_name[FILE_NAME_LENGTH];
    td_char pixel_str[PIXEL_FORMAT_STRING_LEN];

    switch (frame->pixel_format) {
        case OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420:
        case OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420:
            if (snprintf_s(pixel_str, PIXEL_FORMAT_STRING_LEN, PIXEL_FORMAT_STRING_LEN - 1, "P420") < EOK) {
                printf("set pixel name fail!\n");
                return TD_FAILURE;
            }
            break;

        case OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422:
        case OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422:
            if (snprintf_s(pixel_str, PIXEL_FORMAT_STRING_LEN, PIXEL_FORMAT_STRING_LEN - 1, "P422") < EOK) {
                printf("set pixel name fail!\n");
                return TD_FAILURE;
            }
            break;

        default:
            if (snprintf_s(pixel_str, PIXEL_FORMAT_STRING_LEN, PIXEL_FORMAT_STRING_LEN - 1, "P400") < EOK) {
                printf("set pixel name fail!\n");
                return TD_FAILURE;
            }
            break;
    }

    if (snprintf_s(yuv_name, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./vpss%d_%ux%u_%s.yuv",
        grp, frame->width, frame->height, pixel_str) < EOK) {
        printf("set output file name fail!\n");
        return TD_FAILURE;
    }

    printf("Dump YUV frame of vpss%d  to file: \"%s\"\n", grp, yuv_name);

    g_pfd = fopen(yuv_name, "wb");
    if (g_pfd == TD_NULL) {
        printf("open file failed, errno %d!\n", errno);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 vpss_src_dump_try_get_frame(ot_vpss_grp grp)
{
    td_s32 try_times = 10;
    const td_s32 millisec = 1000;

    /* get frame  */
    while ((ss_mpi_vpss_get_grp_frame(grp, &g_frame, millisec) != TD_SUCCESS)) {
        if (g_signal_flag == 1) {
            printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
            return TD_FAILURE;
        }
        try_times--;
        if (try_times <= 0) {
            printf("get frame error for 10 times,now exit !!!\n");
            return TD_FAILURE;
        }
        sleep(2); /* sleep 2 seconds */
    }

    return vpss_src_dump_open_yuv_file(grp, &g_frame.video_frame);
}

static td_void sample_misc_vpss_dump_src_image(ot_vpss_grp grp)
{
    td_bool is_send_to_vgs;
    if (vpss_src_dump_try_get_frame(grp) != TD_SUCCESS) {
        goto exit;
    }
    is_send_to_vgs = ((g_frame.video_frame.compress_mode != OT_COMPRESS_MODE_NONE) ||
                    (g_frame.video_frame.video_format != OT_VIDEO_FORMAT_LINEAR));

    if (is_send_to_vgs) {
        vpss_src_dump_send_vgs_and_dump(&g_frame);
    } else {
        sample_yuv_8bit_dump(&g_frame.video_frame, g_pfd);
    }
exit:
    vpss_restore(grp);
    return;
}

static void usage(void)
{
    printf("\n"
           "*************************************************\n"
           "Usage: ./vpss_src_dump [Grp]\n"
           "1)VpssGrp: \n"
           "   Vpss group id\n"
           "*)Example:\n"
           "e.g : ./vpss_src_dump 0 \n"
           "*************************************************\n"
           "\n");
}

static td_bool vpss_src_dump_is_valid_digit_str(const td_char *str)
{
    size_t i;
    size_t str_len;

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

static td_s32 vpss_get_argv_val(char *argv[], td_u32 index, td_slong *val)
{
    td_char *end_ptr = TD_NULL;
    td_slong result;

    errno = 0;
    result = strtol(argv[index], &end_ptr, VPSS_ATTR_ARGV_BASE);
    if ((end_ptr == argv[index]) || (*end_ptr != '\0')) {
        return TD_FAILURE;
    } else if (((result == LONG_MIN) || (result == LONG_MAX)) &&
        (errno == ERANGE)) {
        return TD_FAILURE;
    }

    *val = result;
    return TD_SUCCESS;
}

#ifdef __LITEOS__
td_s32 vpss_src_dump(int argc, char *argv[])
#else
td_s32 main(int argc, char *argv[])
#endif
{
    td_s32 ret;
    td_slong result = 0;

    g_vpss_grp = 0;

    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("\tTo see more usage, please enter: ./vpss_src_dump -h\n\n");

    if (argc > 1) {
        if (!strncmp(argv[1], "-h", 2)) { /* 2 help */
            usage();
            return TD_SUCCESS;
        }
    }

    if (argc != 2) { /* 2 args */
        usage();
        return TD_SUCCESS;
    }

    if (!vpss_src_dump_is_valid_digit_str(argv[1])) {
        printf("%s is invalid, must be reasonable non negative integers!!!!\n\n", argv[1]);
        return -1;
    }

    ret = vpss_get_argv_val(argv, 1, &result);
    if (ret != TD_SUCCESS) {
        printf("get grp failed!\n");
        return -1;
    }
    g_vpss_grp = (ot_vpss_grp)result;
    if (!value_between(g_vpss_grp, 0, OT_VPSS_MAX_GRP_NUM - 1)) {
        printf("grp id must be [0,%d]!!!!\n\n", OT_VPSS_MAX_GRP_NUM - 1);
        return -1;
    }

    g_frame.pool_id = OT_VB_INVALID_POOL_ID;
    g_handle = -1;
    g_user_page_addr[0] = TD_NULL;
    g_user_page_addr[1] = TD_NULL;
    g_mem.phys_addr = 0;
    g_mem.vb_pool = OT_VB_INVALID_POOL_ID;
    g_pool = OT_VB_INVALID_POOL_ID;
    g_pfd = TD_NULL;

    g_signal_flag = 0;

#ifndef __LITEOS__
    (td_void)signal(SIGINT, vpss_src_dump_handle_sig);
    (td_void)signal(SIGTERM, vpss_src_dump_handle_sig);
#endif

    sample_misc_vpss_dump_src_image(g_vpss_grp);

    return TD_SUCCESS;
}
