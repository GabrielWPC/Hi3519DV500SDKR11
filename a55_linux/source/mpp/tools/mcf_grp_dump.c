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
#define MCF_GRP_MAX_DUMP_FRAME_CNT 64
#define MAX_FRM_WIDTH 20480
#define FILE_NAME_LENGTH 128
#define PIXEL_FORMAT_STRING_LEN 10
#define GET_CHN_FRAME_TIMEOUT 5000
#define MCF_ATTR_ARGV_BASE 10

static td_u32 g_mcf_depth_flag = 0;
static volatile sig_atomic_t g_signal_flag = 0;

static ot_mcf_grp g_mcf_grp = 0;
static td_u32 g_orig_depth = 0;
static ot_video_frame_info g_grp_frame[MCF_GRP_MAX_DUMP_FRAME_CNT][OT_MCF_PIPE_NUM]; /* 0 for color, 1 for mono */

static td_char *g_user_page_addr[2] = { TD_NULL, TD_NULL }; /* 2 Y and C */
static td_u32 g_size = 0;
static td_u32 g_c_size = 0;

static FILE *g_grp_pfd[OT_MCF_PIPE_NUM] = { TD_NULL, TD_NULL }; /* 0 for color, 1 for mono */

static td_void mcf_grp_dump_flush_file(FILE *fd)
{
    td_s32 ret;
    ret = fflush(fd);
    if (ret != 0) {
        printf("%s,%d fflush file error\n", __FUNCTION__, __LINE__);
    }
}

static td_void mcf_grp_dump_fprintf(FILE *fd, const char *str)
{
    if (fd == TD_NULL) {
        return;
    }
    if (fprintf(fd, "%s", str) < 0) {
        printf("%s,%d fprintf file error\n", __FUNCTION__, __LINE__);
    }
}

static td_void mcf_grp_dump_fwrite(const char *buffer, size_t size, size_t count, FILE *stream)
{
    (td_void)fwrite(buffer, size, count, stream);
}

static td_void mcf_grp_dump_covert_chroma_sp42x_to_planar(const ot_video_frame *frame, FILE *fd,
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

    mcf_grp_dump_flush_file(fd);
    /* save U */
    mcf_grp_dump_fprintf(stderr, "U......");
    mcf_grp_dump_flush_file(stderr);
    for (h = 0; h < uv_height; h++) {
        mem_content = virt_addr_c + h * frame->stride[1];
        if (!is_uv_invert) {
            mem_content += 1;
        }

        for (w = 0; w < frame->width / 2; w++) { /* 2 chroma width */
            tmp_buf[w] = *mem_content;
            mem_content += 2; /* 2 semiplanar steps */
        }
        mcf_grp_dump_fwrite((char *)tmp_buf, frame->width / 2, 1, fd); /* 2 chroma width */
    }
    mcf_grp_dump_flush_file(fd);

    /* save V */
    mcf_grp_dump_fprintf(stderr, "V......");
    mcf_grp_dump_flush_file(stderr);
    for (h = 0; h < uv_height; h++) {
        mem_content = virt_addr_c + h * frame->stride[1];
        if (is_uv_invert) {
            mem_content += 1;
        }

        for (w = 0; w < frame->width / 2; w++) { /* 2 chroma width */
            tmp_buf[w] = *mem_content;
            mem_content += 2; /* 2 semiplanar steps */
        }

        mcf_grp_dump_fwrite((char *)tmp_buf, frame->width / 2, 1, fd); /* 2 chroma width */
    }

    mcf_grp_dump_flush_file(fd);
    if (g_user_page_addr[1] != TD_NULL) {
        ss_mpi_sys_munmap(g_user_page_addr[1], g_c_size);
        g_user_page_addr[1] = TD_NULL;
    }
}

/* When saving a file, sp420 will be denoted by p420 and sp422 will be denoted by p422 in the name of the file */
static td_void sample_mcf_grp_yuv_8bit_dump(const ot_video_frame *frame, FILE *fd)
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
    mcf_grp_dump_fprintf(stderr, "saving......Y......");
    mcf_grp_dump_flush_file(stderr);

    for (h = 0; h < frame->height; h++) {
        mem_content = virt_addr_y + h * frame->stride[0];
        mcf_grp_dump_fwrite(mem_content, frame->width, 1, fd);
    }
    mcf_grp_dump_flush_file(fd);

    if (pixel_format != OT_PIXEL_FORMAT_YUV_400) {
        mcf_grp_dump_covert_chroma_sp42x_to_planar(frame, fd, uv_height, is_uv_invert);
    }
    char buffer[128];
    (td_void)snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1,
                        "done %u, pts: %llu!\n", frame->time_ref, frame->pts);

    mcf_grp_dump_fprintf(stderr, buffer);
    mcf_grp_dump_flush_file(stderr);
    ss_mpi_sys_munmap(g_user_page_addr[0], g_size);
    g_user_page_addr[0] = NULL;
}

static td_void mcf_grp_set_depth(ot_mcf_grp mcf_grp, td_u32 depth)
{
    td_s32 ret;
    ot_mcf_grp_attr grp_attr;

    ret = ss_mpi_mcf_get_grp_attr(mcf_grp, &grp_attr);
    if (ret != TD_SUCCESS) {
        printf("get grp attr error!!!\n");
    }
    g_orig_depth = grp_attr.depth;
    grp_attr.depth = depth;
    ret = ss_mpi_mcf_set_grp_attr(mcf_grp, &grp_attr);
    if (ret != TD_SUCCESS) {
        printf("set grp attr error!!!\n");
    }
    g_mcf_depth_flag = 1;
}

static td_void mcf_grp_restore_depth(ot_mcf_grp mcf_grp)
{
    td_s32 ret;
    ot_mcf_grp_attr grp_attr;
    if (g_mcf_depth_flag) {
        ret = ss_mpi_mcf_get_grp_attr(mcf_grp, &grp_attr);
        if (ret != TD_SUCCESS) {
            printf("get grp attr error!!!\n");
        }
        grp_attr.depth = g_orig_depth;
        ret = ss_mpi_mcf_set_grp_attr(mcf_grp, &grp_attr);
        if (ret != TD_SUCCESS) {
            printf("set grp attr error!!!\n");
        }
        g_mcf_depth_flag = 0;
    }
}

static td_void mcf_grp_restore(ot_mcf_grp mcf_grp)
{
    td_s32 ret;
    td_s32 i;

    for (i = 0; i < MCF_GRP_MAX_DUMP_FRAME_CNT; i++) {
        if ((g_grp_frame[i][0].pool_id != OT_VB_INVALID_POOL_ID) ||
            (g_grp_frame[i][1].pool_id != OT_VB_INVALID_POOL_ID)) {
            ret = ss_mpi_mcf_release_grp_frame(mcf_grp, &g_grp_frame[i][0], &g_grp_frame[i][1]);
            if (ret != TD_SUCCESS) {
                printf("Release Chn Frame error!!!\n");
            }
            g_grp_frame[i][0].pool_id = OT_VB_INVALID_POOL_ID;
            g_grp_frame[i][1].pool_id = OT_VB_INVALID_POOL_ID;
        }
    }

    if (g_user_page_addr[0] != TD_NULL) {
        ss_mpi_sys_munmap(g_user_page_addr[0], g_size);
        g_user_page_addr[0] = TD_NULL;
    }
    if (g_user_page_addr[1] != TD_NULL) {
        ss_mpi_sys_munmap(g_user_page_addr[1], g_size);
        g_user_page_addr[1] = TD_NULL;
    }

    if (g_grp_pfd[0] != TD_NULL) {
        fclose(g_grp_pfd[0]);
        g_grp_pfd[0] = TD_NULL;
    }

    if (g_grp_pfd[1] != TD_NULL) {
        fclose(g_grp_pfd[1]);
        g_grp_pfd[1] = TD_NULL;
    }

    mcf_grp_restore_depth(mcf_grp);
    return;
}

void mcf_grp_dump_handle_sig(int signo)
{
    if (g_signal_flag) {
        return;
    }

    if (signo == SIGINT || signo == SIGTERM) {
        g_signal_flag = 1;
    }
    return;
}

static td_s32 mcf_grp_get_pixel_str(ot_pixel_format pixel_format, char *pixel_str, td_u32 pixel_str_len)
{
    ot_unused(pixel_str_len);
    switch (pixel_format) {
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

    return TD_SUCCESS;
}

static td_s32 mcf_grp_dump_make_yuv_file_name(ot_mcf_grp grp, td_u32 frame_cnt)
{
    td_char color_yuv_name[FILE_NAME_LENGTH], mono_yuv_name[FILE_NAME_LENGTH];
    td_char color_pixel_str[PIXEL_FORMAT_STRING_LEN], mono_pixel_str[PIXEL_FORMAT_STRING_LEN];

    if (mcf_grp_get_pixel_str(g_grp_frame[0][0].video_frame.pixel_format, color_pixel_str,
        PIXEL_FORMAT_STRING_LEN) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if (mcf_grp_get_pixel_str(g_grp_frame[0][1].video_frame.pixel_format, mono_pixel_str,
        PIXEL_FORMAT_STRING_LEN) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    /* make file name */
    if (snprintf_s(color_yuv_name, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./mcf_grp%d_color_%ux%u_%s_%u.yuv",
        grp, g_grp_frame[0][0].video_frame.width, g_grp_frame[0][0].video_frame.height,
        color_pixel_str, frame_cnt) == -1) {
        printf("set out put file name fail!\n");
        return TD_FAILURE;
    }
    if (snprintf_s(mono_yuv_name, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./mcf_grp%d_mono_%ux%u_%s_%u.yuv",
        grp, g_grp_frame[0][1].video_frame.width, g_grp_frame[0][1].video_frame.height,
        mono_pixel_str, frame_cnt) == -1) {
        printf("set out put file name fail!\n");
        return TD_FAILURE;
    }

    printf("Dump YUV frame of mcf grp %d to file: \"%s\" and \"%s\" \n", grp, color_yuv_name, mono_yuv_name);

    /* open file */
    g_grp_pfd[0] = fopen(color_yuv_name, "wb");
    if (g_grp_pfd[0] == TD_NULL) {
        printf("open color file failed, errno %d!\n", errno);
        return TD_FAILURE;
    }
    g_grp_pfd[1] = fopen(mono_yuv_name, "wb");
    if (g_grp_pfd[1] == TD_NULL) {
        printf("open mono file failed, errno %d!\n", errno);
        return TD_FAILURE;
    }

    mcf_grp_dump_flush_file(stdout);
    return TD_SUCCESS;
}

static td_s32 mcf_grp_dump_get_frame(ot_mcf_grp grp, td_u32 frame_cnt)
{
    td_s32 ret;
    td_u32 cnt = frame_cnt;
    const td_s32 milli_sec = GET_CHN_FRAME_TIMEOUT;
    td_s32 i = 0;
    td_s32 j = 0;

    mcf_grp_set_depth(grp, 8); /* 8 depth */

    /* get frame */
    while (cnt--) {
        if (g_signal_flag == 1) {
            printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
            return TD_FAILURE;
        }

        if (ss_mpi_mcf_get_grp_frame(grp, &g_grp_frame[i][0], &g_grp_frame[i][1], milli_sec) != TD_SUCCESS) {
            printf("Get frame fail \n");
            usleep(1000); /* 1000 sleep */
            continue;
        }

        i++;
    }

    mcf_grp_restore_depth(grp);

    while (j < i) {
        if (g_signal_flag == 1) {
            printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
            return TD_FAILURE;
        }

        if ((OT_DYNAMIC_RANGE_SDR8 == g_grp_frame[j][0].video_frame.dynamic_range) &&
            (OT_DYNAMIC_RANGE_SDR8 == g_grp_frame[j][1].video_frame.dynamic_range)) {
            sample_mcf_grp_yuv_8bit_dump(&g_grp_frame[j][0].video_frame, g_grp_pfd[0]);
            sample_mcf_grp_yuv_8bit_dump(&g_grp_frame[j][1].video_frame, g_grp_pfd[1]);
        } else {
            printf("Only support dump 8-bit data!\n");
        }
        printf("Get frame %d!!\n", i - j);

        /* release frame after using */
        ret = ss_mpi_mcf_release_grp_frame(grp, &g_grp_frame[j][0], &g_grp_frame[j][1]);
        if (ret != TD_SUCCESS) {
            printf("Release frame error, now exit !!!\n");
            return ret;
        }
        g_grp_frame[j][0].pool_id = OT_VB_INVALID_POOL_ID;
        g_grp_frame[j][1].pool_id = OT_VB_INVALID_POOL_ID;
        j++;
    }

    return TD_SUCCESS;
}

static td_s32 mcf_grp_dump_try_get_frame(ot_mcf_grp grp, td_u32 frame_cnt)
{
    const td_s32 milli_sec = GET_CHN_FRAME_TIMEOUT;
    td_s32 try_times = 10;
    td_s32 ret;

    mcf_grp_set_depth(grp, 1);

    while (ss_mpi_mcf_get_grp_frame(grp, &g_grp_frame[0][0], &g_grp_frame[0][1], milli_sec) != TD_SUCCESS) {
        if (g_signal_flag == 1) {
            printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
            return TD_FAILURE;
        }
        printf("trying !!!\n");
        try_times--;
        if (try_times <= 0) {
            printf("get frame error for 10 times,now exit !!!\n");
            return TD_FAILURE;
        }

        usleep(40000); /* 40000 sleep */
    }

    if ((g_grp_frame[0][0].video_frame.video_format != OT_VIDEO_FORMAT_LINEAR) ||
        (g_grp_frame[0][1].video_frame.video_format != OT_VIDEO_FORMAT_LINEAR)) {
        printf("only support linear frame dump!\n");
        ss_mpi_mcf_release_grp_frame(grp, &g_grp_frame[0][0], &g_grp_frame[0][1]);
        g_grp_frame[0][0].pool_id = OT_VB_INVALID_POOL_ID;
        g_grp_frame[0][1].pool_id = OT_VB_INVALID_POOL_ID;
        return TD_FAILURE;
    }

    if (mcf_grp_dump_make_yuv_file_name(grp, frame_cnt) != TD_SUCCESS) {
        ss_mpi_mcf_release_grp_frame(grp, &g_grp_frame[0][0], &g_grp_frame[0][1]);
        g_grp_frame[0][0].pool_id = OT_VB_INVALID_POOL_ID;
        g_grp_frame[0][1].pool_id = OT_VB_INVALID_POOL_ID;
        return TD_FAILURE;
    }

    ret = ss_mpi_mcf_release_grp_frame(grp, &g_grp_frame[0][0], &g_grp_frame[0][1]);
    if (ret != TD_SUCCESS) {
        printf("Release frame error ,now exit !!!\n");
        return TD_FAILURE;
    }
    g_grp_frame[0][0].pool_id = OT_VB_INVALID_POOL_ID;
    g_grp_frame[0][1].pool_id = OT_VB_INVALID_POOL_ID;

    mcf_grp_restore_depth(grp);
    return TD_SUCCESS;
}

td_void sample_mcf_grp_dump(ot_mcf_grp grp, td_u32 frame_cnt)
{
    td_s32 i;

    for (i = 0; i < MCF_GRP_MAX_DUMP_FRAME_CNT; i++) {
        if (memset_s(&g_grp_frame[i][0], OT_MCF_PIPE_NUM * sizeof(ot_video_frame_info),
                     0, OT_MCF_PIPE_NUM * sizeof(ot_video_frame_info)) != EOK) {
            printf("memset_s frame error!!!\n");
            goto exit;
        }
        g_grp_frame[i][0].pool_id = OT_VB_INVALID_POOL_ID;
        g_grp_frame[i][1].pool_id = OT_VB_INVALID_POOL_ID;
    }

    if (mcf_grp_dump_try_get_frame(grp, frame_cnt) != TD_SUCCESS) {
        goto exit;
    }

    if (mcf_grp_dump_get_frame(grp, frame_cnt) != TD_SUCCESS) {
        goto exit;
    }

exit:
    mcf_grp_restore(grp);
    return;
}

static td_void usage(td_void)
{
    printf("\n"
           "*************************************************\n"
           "Usage: ./mcf_grp_dump [McfGrp] [FrmCnt]\n"
           "1)McfGrp: \n"
           "   Mcf group id\n"
           "2)FrmCnt: \n"
           "   the count of frame to be dump\n"
           "*)Example:\n"
           "   e.g : ./mcf_grp_dump 0 10\n"
           "   e.g : ./mcf_grp_dump 1 10\n"
           "*************************************************\n"
           "\n");
}

static td_void mcf_grp_dump_init_global_params(td_void)
{
    g_mcf_depth_flag = 0;
    g_signal_flag = 0;
    g_user_page_addr[0] = TD_NULL;
    g_user_page_addr[1] = TD_NULL;
    g_orig_depth = 0;
    g_size = 0;
    g_grp_pfd[0] = TD_NULL;
    g_grp_pfd[1] = TD_NULL;
}

static td_bool mcf_grp_dump_is_valid_digit_str(const td_char *str)
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

static td_void mcf_grp_dump_check_signal(td_void)
{
    if (signal(SIGINT, mcf_grp_dump_handle_sig) == SIG_ERR) {
        perror("error\n");
    }

    if (signal(SIGTERM, mcf_grp_dump_handle_sig) == SIG_ERR) {
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

    ret = mcf_get_argv_val(argv, 2, &result); /* 2 frame count */
    if (ret != TD_SUCCESS) {
        printf("get frame cnt failed!\n");
        return TD_FAILURE;
    }
    *frame_cnt = (td_s32)result;
    if ((*frame_cnt < 1) || (*frame_cnt > MCF_GRP_MAX_DUMP_FRAME_CNT)) {
        printf("dump frame cnt must be [1, %d].\n", MCF_GRP_MAX_DUMP_FRAME_CNT);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

#ifdef __LITEOS__
td_s32 mcf_grp_dump(int argc, char *argv[])
#else
td_s32 main(int argc, char *argv[])
#endif
{
    td_s32 i;
    td_s32 frame_cnt;

    printf("\nNOTICE: This tool only can be used for TESTING under mcf fusion mode!!!\n");
    printf("\tTo see more usage, please enter: ./mcf_grp_dump -h\n\n");

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

    if (argc != 3) { /* 3 args */
        usage();
        exit(TD_SUCCESS);
    }

    for (i = 1; i < argc; i++) {
        if (!mcf_grp_dump_is_valid_digit_str(argv[i])) {
            printf("%s is invalid, must be reasonable non negative integers!!!!\n\n", argv[i]);
            return TD_FAILURE;
        }
    }

    if (mcf_parse_input_param(argv, &frame_cnt) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    mcf_grp_dump_init_global_params();
#ifndef __LITEOS__
    mcf_grp_dump_check_signal();
#endif
    sample_mcf_grp_dump(g_mcf_grp, (td_u32)frame_cnt);
    return TD_SUCCESS;
}
