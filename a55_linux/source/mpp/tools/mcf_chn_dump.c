/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "ot_math.h"
#include "ot_buffer.h"
#include "securec.h"
#include "ot_common.h"
#include "ot_common_video.h"
#include "ot_common_sys.h"
#include "ot_common_vb.h"
#include "ss_mpi_sys_mem.h"
#include "ss_mpi_vb.h"
#include "ss_mpi_vgs.h"
#include "ot_common_mcf.h"
#include "ss_mpi_mcf.h"

#define MAX_DIGIT_LEN 4
#define MAX_DUMP_FRAME_CNT 64
#define MAX_FRM_WIDTH 20480
#define FILE_NAME_LENGTH 128
#define PIXEL_FORMAT_STRING_LEN 10
#define GET_CHN_FRAME_TIMEOUT 5000
#define MCF_ATTR_ARGV_BASE 10

/* set MCF_GET_CHN_FRAME_CONTINUOUSLY to 1 to get continuous frame */
#define MCF_GET_CHN_FRAME_CONTINUOUSLY 0
/* set the value of MCF_GET_CHN_FRAME_NUM according to how many frames you need to dump */
#define MCF_GET_CHN_FRAME_NUM 40
typedef struct {
    ot_vb_blk vb_blk;
    ot_vb_pool vb_pool;
    td_u32 pool_id;

    td_phys_addr_t phys_addr;
    td_void *virt_addr;
} mcf_dump_mem;

static td_u32 g_mcf_depth_flag = 0;
static volatile sig_atomic_t g_signal_flag = 0;

static ot_mcf_grp g_mcf_grp = 0;
static ot_mcf_chn g_mcf_chn = 0;
static td_u32 g_orig_depth = 0;
static ot_video_frame_info g_frame;
static ot_video_frame_info g_multi_frame[MCF_GET_CHN_FRAME_NUM];

static ot_vb_pool g_pool = OT_VB_INVALID_POOL_ID;
static mcf_dump_mem g_dump_mem = { 0 };
static ot_vgs_handle g_handle = -1;
static td_u32 g_blk_size = 0;

static td_char *g_user_page_addr[2] = { TD_NULL, TD_NULL }; /* 2 Y and C */
static td_u32 g_size = 0;
static td_u32 g_c_size = 0;

static FILE *g_pfd = TD_NULL;

static td_void mcf_chn_dump_flush_file(FILE *fd)
{
    td_s32 ret;
    ret = fflush(fd);
    if (ret != 0) {
        printf("%s,%d fflush file error\n", __FUNCTION__, __LINE__);
    }
}

static td_void mcf_chn_dump_fprintf(FILE *fd, const char *str)
{
    if (fd == TD_NULL) {
        return;
    }
    if (fprintf(fd, "%s", str) < 0) {
        printf("%s,%d fprintf file error\n", __FUNCTION__, __LINE__);
    }
}

static td_void mcf_chn_dump_fwrite(const char *buffer, size_t size, size_t count, FILE *stream)
{
    (td_void)fwrite(buffer, size, count, stream);
}

static td_void mcf_chn_dump_covert_chroma_sp42x_to_planar(const ot_video_frame *frame, FILE *fd,
    td_u32 uv_height, td_bool is_uv_invert)
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

    mcf_chn_dump_flush_file(fd);
    /* save U */
    mcf_chn_dump_fprintf(stderr, "U......");
    mcf_chn_dump_flush_file(stderr);
    for (h = 0; h < uv_height; h++) {
        mem_content = virt_addr_c + h * frame->stride[1];
        if (!is_uv_invert) {
            mem_content += 1;
        }

        for (w = 0; w < frame->width / 2; w++) { /* 2 chroma width */
            tmp_buf[w] = *mem_content;
            mem_content += 2; /* 2 semiplanar steps */
        }
        mcf_chn_dump_fwrite((char *)tmp_buf, frame->width / 2, 1, fd); /* 2 chroma width */
    }
    mcf_chn_dump_flush_file(fd);

    /* save V */
    mcf_chn_dump_fprintf(stderr, "V......");
    mcf_chn_dump_flush_file(stderr);
    for (h = 0; h < uv_height; h++) {
        mem_content = virt_addr_c + h * frame->stride[1];
        if (is_uv_invert) {
            mem_content += 1;
        }

        for (w = 0; w < frame->width / 2; w++) { /* 2 chroma width */
            tmp_buf[w] = *mem_content;
            mem_content += 2; /* 2 semiplanar steps */
        }

        mcf_chn_dump_fwrite((char *)tmp_buf, frame->width / 2, 1, fd); /* 2 chroma width */
    }

    mcf_chn_dump_flush_file(fd);
    if (g_user_page_addr[1] != TD_NULL) {
        ss_mpi_sys_munmap(g_user_page_addr[1], g_c_size);
        g_user_page_addr[1] = TD_NULL;
    }
}

/* When saving a file, sp420 will be denoted by p420 and sp422 will be denoted by p422 in the name of the file */
static td_void sample_yuv_8bit_dump(const ot_video_frame *frame, FILE *fd)
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
    mcf_chn_dump_fprintf(stderr, "saving......Y......");
    mcf_chn_dump_flush_file(stderr);

    for (h = 0; h < frame->height; h++) {
        mem_content = virt_addr_y + h * frame->stride[0];
        mcf_chn_dump_fwrite(mem_content, frame->width, 1, fd);
    }

    mcf_chn_dump_flush_file(fd);
    if (pixel_format != OT_PIXEL_FORMAT_YUV_400) {
        mcf_chn_dump_covert_chroma_sp42x_to_planar(frame, fd, uv_height, is_uv_invert);
    }
    char buffer[128];
    (td_void)snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1, "done %u!\n", frame->time_ref);

    mcf_chn_dump_fprintf(stderr, buffer);
    mcf_chn_dump_flush_file(stderr);
    ss_mpi_sys_munmap(g_user_page_addr[0], g_size);
    g_user_page_addr[0] = NULL;
}

static td_void mcf_restore_depth(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn)
{
    td_s32 ret;
    ot_mcf_chn_attr chn_attr;
    ot_mcf_ext_chn_attr ext_chn_attr;

    if (mcf_chn >= OT_MCF_MAX_PHYS_CHN_NUM) {
        ret = ss_mpi_mcf_get_ext_chn_attr(mcf_grp, mcf_chn, &ext_chn_attr);
        if (ret != TD_SUCCESS) {
            printf("get ext chn attr error!!!\n");
        }

        ext_chn_attr.depth = g_orig_depth;
        ret = ss_mpi_mcf_set_ext_chn_attr(mcf_grp, mcf_chn, &ext_chn_attr);
        if (ret != TD_SUCCESS) {
            printf("set ext chn attr error!!!\n");
        }
    } else {
        ret = ss_mpi_mcf_get_chn_attr(mcf_grp, mcf_chn, &chn_attr);
        if (ret != TD_SUCCESS) {
            printf("get chn attr error!!!\n");
        }

        chn_attr.depth = g_orig_depth;
        ret = ss_mpi_mcf_set_chn_attr(mcf_grp, mcf_chn, &chn_attr);
        if (ret != TD_SUCCESS) {
            printf("set chn attr error!!!\n");
        }
    }
}

static td_void mcf_restore(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn)
{
    td_s32 ret;

    if (g_frame.pool_id != OT_VB_INVALID_POOL_ID) {
        ret = ss_mpi_mcf_release_chn_frame(mcf_grp, mcf_chn, &g_frame);
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
        fclose(g_pfd);
        g_pfd = TD_NULL;
    }

    if (g_mcf_depth_flag) {
        mcf_restore_depth(mcf_grp, mcf_chn);
        g_mcf_depth_flag = 0;
    }
    return;
}

void mcf_chn_dump_handle_sig(int signo)
{
    if (g_signal_flag) {
        return;
    }

    if (signo == SIGINT || signo == SIGTERM) {
        g_signal_flag = 1;
    }
    return;
}

td_s32 mcf_chn_dump_make_yuv_file_name(ot_mcf_grp grp, ot_mcf_chn chn, td_u32 frame_cnt)
{
    td_char yuv_name[FILE_NAME_LENGTH];
    td_char pixel_str[PIXEL_FORMAT_STRING_LEN];
    switch (g_frame.video_frame.pixel_format) {
        case OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420:
        case OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420:
            if (snprintf_s(pixel_str, PIXEL_FORMAT_STRING_LEN, PIXEL_FORMAT_STRING_LEN - 1, "P420") == -1) {
                printf("set pixel name fail!\n");
                return TD_FAILURE;
            }
            break;

        case OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422:
        case OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422:
            if (snprintf_s(pixel_str, PIXEL_FORMAT_STRING_LEN, PIXEL_FORMAT_STRING_LEN - 1, "P422") == -1) {
                printf("set pixel name fail!\n");
                return TD_FAILURE;
            }
            break;

        default:
            if (snprintf_s(pixel_str, PIXEL_FORMAT_STRING_LEN, PIXEL_FORMAT_STRING_LEN - 1, "P400") == -1) {
                printf("set pixel name fail!\n");
                return TD_FAILURE;
            }
            break;
    }

    /* make file name */
    if (snprintf_s(yuv_name, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./mcf_grp%d_chn%d_%ux%u_%s_%u.yuv",
        grp, chn, g_frame.video_frame.width,
        g_frame.video_frame.height, pixel_str, frame_cnt) == -1) {
        printf("set out put file name fail!\n");
        return TD_FAILURE;
    }
    printf("Dump YUV frame of mcf chn %d  to file: \"%s\"\n", chn, yuv_name);

    /* open file */
    g_pfd = fopen(yuv_name, "wb");
    if (g_pfd == TD_NULL) {
        printf("open file failed, errno %d!\n", errno);
        return TD_FAILURE;
    }
    mcf_chn_dump_flush_file(stdout);
    return TD_SUCCESS;
}

td_void mcf_chn_dump_set_vgs_frame_info(ot_video_frame_info *vgs_frame_info, const mcf_dump_mem *dump_mem,
    const ot_vb_calc_cfg *vb_calc_cfg, const ot_video_frame_info *mcf_frame_info)
{
    if ((mcf_frame_info == TD_NULL) || (vb_calc_cfg == TD_NULL) ||
        (dump_mem == TD_NULL) || (vgs_frame_info == TD_NULL)) {
        return;
    }

    vgs_frame_info->video_frame.phys_addr[0] = dump_mem->phys_addr;
    vgs_frame_info->video_frame.phys_addr[1] = vgs_frame_info->video_frame.phys_addr[0] + vb_calc_cfg->main_y_size;
    vgs_frame_info->video_frame.width = mcf_frame_info->video_frame.width;
    vgs_frame_info->video_frame.height = mcf_frame_info->video_frame.height;
    vgs_frame_info->video_frame.stride[0] = vb_calc_cfg->main_stride;
    vgs_frame_info->video_frame.stride[1] = vb_calc_cfg->main_stride;
    vgs_frame_info->video_frame.compress_mode = OT_COMPRESS_MODE_NONE;
    vgs_frame_info->video_frame.pixel_format = mcf_frame_info->video_frame.pixel_format;
    vgs_frame_info->video_frame.video_format = OT_VIDEO_FORMAT_LINEAR;
    vgs_frame_info->video_frame.dynamic_range = mcf_frame_info->video_frame.dynamic_range;
    vgs_frame_info->video_frame.pts = 0;
    vgs_frame_info->video_frame.time_ref = 0;
    vgs_frame_info->pool_id = dump_mem->vb_pool;
    vgs_frame_info->mod_id = OT_ID_VGS;
}

td_s32 mcf_chn_dump_init_vgs_pool(mcf_dump_mem *dump_mem, ot_vb_calc_cfg *vb_calc_cfg)
{
    td_u32 width = g_frame.video_frame.width;
    td_u32 height = g_frame.video_frame.height;
    ot_pixel_format pixel_format = g_frame.video_frame.pixel_format;
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

    g_blk_size = vb_calc_cfg->vb_size;

    vb_pool_cfg.blk_size = g_blk_size;
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
    dump_mem->vb_blk = ss_mpi_vb_get_blk(dump_mem->vb_pool, g_blk_size, TD_NULL);
    if (dump_mem->vb_blk == OT_VB_INVALID_HANDLE) {
        printf("get vb blk failed!\n");
        return TD_FAILURE;
    }

    dump_mem->phys_addr = ss_mpi_vb_handle_to_phys_addr(dump_mem->vb_blk);
    return TD_SUCCESS;
}

td_s32 mcf_chn_dump_send_vgs_and_dump(const ot_video_frame_info *mcf_frame_info)
{
    ot_video_frame_info vgs_frame_info = {0};
    ot_vgs_task_attr vgs_task_attr;
    ot_vb_calc_cfg vb_calc_cfg = {0};

    if (mcf_chn_dump_init_vgs_pool(&g_dump_mem, &vb_calc_cfg) != TD_SUCCESS) {
        printf("init vgs pool failed\n");
        return TD_FAILURE;
    }

    mcf_chn_dump_set_vgs_frame_info(&vgs_frame_info, &g_dump_mem, &vb_calc_cfg, mcf_frame_info);

    if (ss_mpi_vgs_begin_job(&g_handle) != TD_SUCCESS) {
        printf("ss_mpi_vgs_begin_job failed\n");
        return TD_FAILURE;
    }

    if (memcpy_s(&vgs_task_attr.img_in, sizeof(ot_video_frame_info),
        mcf_frame_info, sizeof(ot_video_frame_info)) != EOK) {
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

static td_bool mcf_chn_dump_is_frame_continuous(td_u32 frame_cnt)
{
    if (MCF_GET_CHN_FRAME_CONTINUOUSLY == 0) {
        return TD_FALSE;
    }

    if (frame_cnt > MCF_GET_CHN_FRAME_NUM) {
        printf("MCF_GET_CHN_FRAME_NUM(%d) is small than frame_cnt(%d), "
            "please edit the value of this macro if you want to get frame continuously!\n",
            MCF_GET_CHN_FRAME_NUM, frame_cnt);
        return TD_FALSE;
    }

    return TD_TRUE;
}
static td_s32 mcf_chn_dump_uncontinuous_frame(ot_mcf_grp grp, ot_mcf_chn chn, td_u32 frame_cnt)
{
    td_s32 ret;
    td_u32 cnt = frame_cnt;
    const td_s32 milli_sec = GET_CHN_FRAME_TIMEOUT;
    td_bool is_send_to_vgs;

    /* get frame */
    while (cnt--) {
        if (g_signal_flag == 1) {
            printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
            return TD_FAILURE;
        }
        if (ss_mpi_mcf_get_chn_frame(grp, chn, &g_frame, milli_sec) != TD_SUCCESS) {
            printf("Get frame fail \n");
            usleep(1000); /* 1000 sleep */
            continue;
        }

        is_send_to_vgs = ((g_frame.video_frame.compress_mode != OT_COMPRESS_MODE_NONE) ||
            (g_frame.video_frame.video_format != OT_VIDEO_FORMAT_LINEAR));

        if (is_send_to_vgs) {
            if (mcf_chn_dump_send_vgs_and_dump(&g_frame) != TD_SUCCESS) {
                ss_mpi_mcf_release_chn_frame(grp, chn, &g_frame);
                g_frame.pool_id = OT_VB_INVALID_POOL_ID;
                return TD_FAILURE;
            }
        } else {
            if (OT_DYNAMIC_RANGE_SDR8 == g_frame.video_frame.dynamic_range) {
                sample_yuv_8bit_dump(&g_frame.video_frame, g_pfd);
            } else {
                printf("Only support dump 8-bit data!\n");
            }
        }

        printf("Get frame %u!!\n", cnt);
        /* release frame after using */
        ret = ss_mpi_mcf_release_chn_frame(grp, chn, &g_frame);
        if (ret != TD_SUCCESS) {
            printf("Release frame error ,now exit !!!\n");
            return ret;
        }
        g_frame.pool_id = OT_VB_INVALID_POOL_ID;
    }
    return TD_SUCCESS;
}

static td_s32 mcf_chn_dump_continuous_frame(ot_mcf_grp grp, ot_mcf_chn chn, td_u32 frame_cnt)
{
    td_u32 cnt = frame_cnt;
    td_u32 get_frame_cnt = 0;
    const td_s32 milli_sec = GET_CHN_FRAME_TIMEOUT;
    td_s32 ret;

    /* get frame */
    while (cnt--) {
        if (g_signal_flag == 1) {
            printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
            return TD_FAILURE;
        }

        if (ss_mpi_mcf_get_chn_frame(grp, chn, &g_multi_frame[get_frame_cnt], milli_sec) != TD_SUCCESS) {
            printf("Get frame failed!\n");
            usleep(1000); /* 1000 sleep */
            continue;
        }

        get_frame_cnt++;
    }

    cnt = 0;
    while (cnt < get_frame_cnt) {
        if (g_signal_flag == 1) {
            printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
            return TD_FAILURE;
        }

        if (OT_DYNAMIC_RANGE_SDR8 == g_multi_frame[cnt].video_frame.dynamic_range) {
            sample_yuv_8bit_dump(&g_multi_frame[cnt].video_frame, g_pfd);
        } else {
            printf("Only support dump 8-bit data!\n");
        }

        printf("Get frame %u\n", cnt);
        /* release frame after using */
        ret = ss_mpi_mcf_release_chn_frame(grp, chn, &g_multi_frame[cnt]);
        if (ret != TD_SUCCESS) {
            printf("Release frame failed, now exit!\n");
            return ret;
        }
        g_multi_frame[cnt].pool_id = OT_VB_INVALID_POOL_ID;
        cnt++;
    }

    return TD_SUCCESS;
}

static td_void mcf_chn_dump_release_multi_frame(ot_mcf_grp grp, ot_mcf_chn chn, td_u32 frame_cnt)
{
    td_u32 i;

    for (i = 0; i < frame_cnt; i++) {
        if (g_multi_frame[i].pool_id != OT_VB_INVALID_POOL_ID) {
            (td_void)ss_mpi_mcf_release_chn_frame(grp, chn, &g_multi_frame[i]);
        }
    }
}

static td_s32 mcf_chn_dump_get_frame(ot_mcf_grp grp, ot_mcf_chn chn, td_u32 frame_cnt)
{
    td_s32 ret;
    td_bool get_ctoninuous_frame = mcf_chn_dump_is_frame_continuous(frame_cnt);
    if (get_ctoninuous_frame == TD_TRUE) {
        ret = mcf_chn_dump_continuous_frame(grp, chn, frame_cnt);
        if (ret != TD_SUCCESS) {
            mcf_chn_dump_release_multi_frame(grp, chn, frame_cnt);
        }
    } else {
        ret = mcf_chn_dump_uncontinuous_frame(grp, chn, frame_cnt);
    }

    return ret;
}
static td_s32 mcf_chn_dump_try_get_frame(ot_mcf_grp grp, ot_mcf_chn chn, td_u32 frame_cnt)
{
    const td_s32 milli_sec = GET_CHN_FRAME_TIMEOUT;
    td_s32 try_times = 10;
    td_s32 ret;
    while (ss_mpi_mcf_get_chn_frame(grp, chn, &g_frame, milli_sec) != TD_SUCCESS) {
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

    if (g_frame.video_frame.video_format != OT_VIDEO_FORMAT_LINEAR) {
        printf("only support linear frame dump!\n");
        ss_mpi_mcf_release_chn_frame(grp, chn, &g_frame);
        g_frame.pool_id = OT_VB_INVALID_POOL_ID;
        return TD_FAILURE;
    }

    if (mcf_chn_dump_make_yuv_file_name(grp, chn, frame_cnt) != TD_SUCCESS) {
        ss_mpi_mcf_release_chn_frame(grp, chn, &g_frame);
        g_frame.pool_id = OT_VB_INVALID_POOL_ID;
        return TD_FAILURE;
    }

    ret = ss_mpi_mcf_release_chn_frame(grp, chn, &g_frame);
    if (ret != TD_SUCCESS) {
        printf("Release frame error ,now exit !!!\n");
        return TD_FAILURE;
    }
    g_frame.pool_id = OT_VB_INVALID_POOL_ID;
    return TD_SUCCESS;
}

td_void sample_misc_mcf_dump(ot_mcf_grp grp, ot_mcf_chn chn, td_u32 frame_cnt)
{
    const td_u32 depth = 2;
    td_s32 ret;
    ot_mcf_chn_attr chn_attr;
    ot_mcf_ext_chn_attr ext_chn_attr;

    if (chn >= OT_MCF_MAX_PHYS_CHN_NUM) {
        ret = ss_mpi_mcf_get_ext_chn_attr(grp, chn, &ext_chn_attr);
        if (ret != TD_SUCCESS) {
            printf("get ext chn attr error!!!\n");
            return;
        }

        g_orig_depth = ext_chn_attr.depth;
        ext_chn_attr.depth = depth;

        if (ss_mpi_mcf_set_ext_chn_attr(grp, chn, &ext_chn_attr) != TD_SUCCESS) {
            printf("set ext chn attr error!!!\n");
            goto exit;
        }
    } else {
        ret = ss_mpi_mcf_get_chn_attr(grp, chn, &chn_attr);
        if (ret != TD_SUCCESS) {
            printf("get chn attr error!!!\n");
            return;
        }

        g_orig_depth = chn_attr.depth;
        chn_attr.depth = depth;

        if (ss_mpi_mcf_set_chn_attr(grp, chn, &chn_attr) != TD_SUCCESS) {
            printf("set chn attr error!!!\n");
            goto exit;
        }
    }

    g_mcf_depth_flag = 1;

    if (memset_s(&g_frame, sizeof(ot_video_frame_info), 0, sizeof(ot_video_frame_info)) != EOK) {
        printf("memset_s frame error!!!\n");
        goto exit;
    }
    g_frame.pool_id = OT_VB_INVALID_POOL_ID;

    if (mcf_chn_dump_try_get_frame(grp, chn, frame_cnt) != TD_SUCCESS) {
        goto exit;
    }

    if (mcf_chn_dump_get_frame(grp, chn, frame_cnt) != TD_SUCCESS) {
        goto exit;
    }

exit:
    mcf_restore(grp, chn);
    return;
}

static td_void usage(td_void)
{
    printf("\n"
           "*************************************************\n"
           "Usage: ./mcf_chn_dump [McfGrp] [McfChn] [FrmCnt]\n"
           "1)McfGrp: \n"
           "   Mcf group id\n"
           "2)McfChn: \n"
           "   mcf chn id\n"
           "3)FrmCnt: \n"
           "   the count of frame to be dump\n"
           "*)Example:\n"
           "   e.g : ./mcf_chn_dump 0 0 1\n"
           "   e.g : ./mcf_chn_dump 1 4 2\n"
           "*)set MCF_GET_CHN_FRAME_CONTINUOUSLY to 1 to get continuous frame\n"
           "*************************************************\n"
           "\n");
}

static td_void mcf_chn_dump_init_global_params(td_void)
{
    g_mcf_depth_flag = 0;
    g_signal_flag = 0;
    g_user_page_addr[0] = TD_NULL;
    g_user_page_addr[1] = TD_NULL;
    g_frame.pool_id = OT_VB_INVALID_POOL_ID;
    g_orig_depth = 0;
    g_pool = OT_VB_INVALID_POOL_ID;
    g_handle = -1;
    g_blk_size = 0;
    g_size = 0;
    g_pfd = TD_NULL;
}

static td_bool mcf_chn_dump_is_valid_digit_str(const td_char *str)
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

static td_void mcf_chn_dump_check_signal(td_void)
{
    if (signal(SIGINT, mcf_chn_dump_handle_sig) == SIG_ERR) {
        perror("error\n");
    }

    if (signal(SIGTERM, mcf_chn_dump_handle_sig) == SIG_ERR) {
        perror("error\n");
    }
}

static td_s32 mcf_get_argv_val(char *argv[], td_u32 index, td_slong *val)
{
    td_char *end_ptr = TD_NULL;
    td_slong result;

    errno = 0;
    result = strtol(argv[index], &end_ptr, MCF_ATTR_ARGV_BASE);
    if ((end_ptr == argv[index]) || (*end_ptr != '\0')) {
        return TD_FAILURE;
    } else if (((result == LONG_MIN) || (result == LONG_MAX)) &&
        (errno == ERANGE)) {
        return TD_FAILURE;
    }

    *val = result;
    return TD_SUCCESS;
}

static td_s32 mcf_parse_input_param(char *argv[], td_s32 *frame_cnt)
{
    td_slong result = 0;
    td_s32 ret;

    ret = mcf_get_argv_val(argv, 1, &result); /* 1 grp id */
    if (ret != TD_SUCCESS) {
        printf("get grp failed!\n");
        return TD_FAILURE;
    }
    g_mcf_grp = (ot_mcf_grp)result;
    if (!value_between(g_mcf_grp, 0, OT_MCF_MAX_GRP_NUM - 1)) {
        printf("grp id must be [0, %d]!!!!\n\n", OT_MCF_MAX_GRP_NUM - 1);
        return TD_FAILURE;
    }

    ret = mcf_get_argv_val(argv, 2, &result); /* 2 chn id */
    if (ret != TD_SUCCESS) {
        printf("get chn failed!\n");
        return TD_FAILURE;
    }
    g_mcf_chn = (ot_mcf_chn)result;
    if (!value_between(g_mcf_chn, 0, OT_MCF_MAX_CHN_NUM - 1)) {
        printf("chn id must be [0, %d]!!!!\n\n", OT_MCF_MAX_CHN_NUM - 1);
        return TD_FAILURE;
    }

    ret = mcf_get_argv_val(argv, 3, &result); /* 3 frame count */
    if (ret != TD_SUCCESS) {
        printf("get frame cnt failed!\n");
        return TD_FAILURE;
    }
    *frame_cnt = (td_s32)result;
    if (*frame_cnt <= 0 || *frame_cnt > MAX_DUMP_FRAME_CNT) {
        printf("dump frame cnt %d is out of range [1, %d]!\n", *frame_cnt, MAX_DUMP_FRAME_CNT);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

#ifdef __LITEOS__
td_s32 mcf_chn_dump(int argc, char *argv[])
#else
td_s32 main(int argc, char *argv[])
#endif
{
    td_s32 i;
    td_s32 frame_cnt;

    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("\tTo see more usage, please enter: ./mcf_chn_dump -h\n\n");

    if (argv == TD_NULL) {
        usage();
        exit(TD_SUCCESS);
    }

    if (argc > 1) {
        if (!strncmp(argv[1], "-h", 2)) { /* 2 help */
            usage();
            exit(TD_SUCCESS);
        }
    }

    if (argc != 4) { /* 4 args */
        usage();
        exit(TD_SUCCESS);
    }

    for (i = 1; i < argc; i++) {
        if (!mcf_chn_dump_is_valid_digit_str(argv[i])) {
            printf("%s is invalid, must be reasonable non negative integers!!!!\n\n", argv[i]);
            return -1;
        }
    }

    if (mcf_parse_input_param(argv, &frame_cnt) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    mcf_chn_dump_init_global_params();
#ifndef __LITEOS__
    mcf_chn_dump_check_signal();
#endif
    sample_misc_mcf_dump(g_mcf_grp, g_mcf_chn, (td_u32)frame_cnt);
    return TD_SUCCESS;
}
