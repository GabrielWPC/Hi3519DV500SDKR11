/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <limits.h>
#include <sys/select.h>

#include "ot_buffer.h"
#include "ss_mpi_sys_bind.h"
#include "ss_mpi_sys_mem.h"
#include "ss_mpi_vi.h"
#include "ss_mpi_vpss.h"
#include "ss_mpi_venc.h"
#include "ot_common_motionsensor.h"
#include "ot_motionsensor_chip_cmd.h"
#include "ot_motionsensor_mng_cmd.h"
#include "ss_mpi_motionfusion.h"
#include "securec.h"

#define DUMP_FRAME_DEPTH        2
#define FILE_NAME_LENGTH        128
#define MAX_FRM_WIDTH           8192
#define DIS_DUMP_ARG_BASE       10
#define MAX_DIGIT_LEN           4
#define TOTAL_NODE_NUM          8
#define FILE_NAME_SHORT         5
#define DIS_FRAME_BUF_RATIO_MIN 70
#define VENC_WIDTH              1920
#define VENC_HEIGHT             1080
#define VENC_FRAME_RATE         30
#define VENC_BIT_RATE           (1024 * 2 + 2048 * 30 / 30)
#define VENC_BUF_SIZE           (1920 * 1080 * 3 / 4)
#define VENC_STRIDE             64
#define MIN_FRAMENUM             1
#define DUMP_YUV_MODE           0
#define DUMP_STREAM_MODE        1
#define IMU_LUT_LEN             30
#define MAX_VENC_NUM            16

static pthread_t g_dis_dump_yuv_pid;
static pthread_t g_dis_dump_stream_pid;
static pthread_t g_dis_dump_yuv_pid1;
static pthread_t g_dis_dump_stream_pid1;

static volatile sig_atomic_t g_signal_flag = 0;
static td_s32 g_msensor_mngfd = -1;
static td_s32 g_msensor_userfd = -1;
static td_u32 g_exposuretime_tmp = 0;
static td_u32 g_h_max_tmp = 0;
static td_u32 g_v_max_tmp = 0;
static const td_u32 fusion_id = 0;
static ot_dis_attr dis_attr;
static ot_dis_cfg dis_cfg;
static td_s32 venc_chn;
static FILE *msensor_param_fd;
static FILE *ldc_fd;

typedef enum {
    DIS_DUMP_STREAM = 0,
    DIS_DUMP_IMU_DATA,
    DIS_DUMP_STREAM_AND_IMU_DATA
} dis_dump_mode;

typedef struct {
    td_u32 exposuretime;
    td_u32      hmax;
    td_u32      vmax;
} dis_dump_isp_info;

typedef struct {
    td_bool enable;
    td_bool dis_enable;
    ot_vi_pipe vi_pipe;
    td_s32 chn;
    td_s32 frame_cnt;
    dis_dump_mode dump_mode;

    ot_vpss_grp vpss_grp;
    ot_vpss_chn vpss_chn;

    td_u32 orig_depth;
    ot_vpss_chn_mode orig_chn_mode;

    ot_video_frame_info frame;
    td_char *user_page_addr[2]; /* 2 Y and C */
    td_u32 size;
    td_u32 c_size;
    td_u32 depth_flag;

    td_u64 begin_pts;
    td_u64 end_pts;
    ot_msensor_buf_attr msensor_buf_attr;

    FILE *pts_fd;
    FILE *gyro_fd;
    FILE *acc_fd;
    FILE *hv_fd;
    FILE *yuv_fd;
    FILE *stream_fd;

    td_s32 picture_cnt;
} dis_dump_info;

typedef struct list_exposuretime {
    td_u64 pts;
    td_u32 exposuretime;
    td_u32 h_max;
    td_u32 v_max;
    struct list_exposuretime* next;
} list_exp;

static td_void list_add_node(td_u64 pts, dis_dump_isp_info *dis_isp, list_exp *head)
{
    list_exp *new_node = TD_NULL;
    new_node = (list_exp *)malloc(sizeof(list_exp));
    if (new_node == TD_NULL) {
        printf("malloc exp node failed!\n");
        return;
    }
    new_node->pts = pts;
    new_node->exposuretime = dis_isp->exposuretime;
    new_node->h_max = dis_isp->hmax;
    new_node->v_max = dis_isp->vmax;

    new_node->next = head->next;
    head->next = new_node;
    g_exposuretime_tmp = dis_isp->exposuretime;
    g_h_max_tmp = dis_isp->hmax;
    g_v_max_tmp = dis_isp->vmax;
}

static list_exp* list_find_node(td_u64 pts, dis_dump_isp_info *dis_isp, list_exp *head)
{
    list_exp *tmp_node = {0};
    tmp_node = head->next;

    while (tmp_node != TD_NULL) {
        if (tmp_node->pts == pts) {
            dis_isp->exposuretime = tmp_node->exposuretime;
            dis_isp->hmax = tmp_node->h_max;
            dis_isp->vmax = tmp_node->v_max;
            return tmp_node;
        }
        tmp_node = tmp_node->next;
    }
    printf("not find exposuretime, pts:%llu, use previous frame exposuretime\n", pts);
    dis_isp->exposuretime = g_exposuretime_tmp;
    dis_isp->hmax = g_h_max_tmp;
    dis_isp->vmax = g_v_max_tmp;
    return TD_NULL;
}

static td_void list_del_node(list_exp *old_node, list_exp *head)
{
    list_exp *tmp_node = {0};
    tmp_node = head;

    while (tmp_node != TD_NULL) {
        if (tmp_node->next == old_node) {
            tmp_node->next = old_node->next;
            free(old_node);
            old_node = TD_NULL;
            return;
        }
        tmp_node = tmp_node->next;
    }
}

static td_void list_del_all_node(list_exp *head)
{
    list_exp *tmp_node = {0};
    list_exp *tmp_node1 = {0};

    tmp_node = head->next;
    while (tmp_node != TD_NULL) {
        tmp_node1 = tmp_node->next;
        free(tmp_node);
        tmp_node = TD_NULL;
        if (tmp_node1 != NULL) {
            tmp_node = tmp_node1;
        }
    }
}

static td_char *dis_dump_get_file_suffix_name(ot_pixel_format pixel_format)
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

static td_char *dis_dump_get_pixel_format_str(ot_pixel_format pixel_format)
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

static td_s32 dis_dump_make_frame_file_name(dis_dump_info *dump_info)
{
    td_char yuv_name[FILE_NAME_LENGTH];
    td_char vi_name[FILE_NAME_SHORT] = "VI";
    td_char vpss_name[FILE_NAME_SHORT] = "VPSS";
    td_s32 pipe = dump_info->dis_enable ? dump_info->vpss_grp : dump_info->vi_pipe;
    td_s32 chn = dump_info->dis_enable ? dump_info->vpss_chn : dump_info->chn;

    /* make file name */
    if (snprintf_s(yuv_name, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./out%s_pipe%d_chn%d_%ux%u_%s_%d.%s",
        dump_info->dis_enable ? vpss_name : vi_name,
        pipe, chn, dump_info->frame.video_frame.width, dump_info->frame.video_frame.height,
        dis_dump_get_pixel_format_str(dump_info->frame.video_frame.pixel_format),
        dump_info->frame_cnt,
        dis_dump_get_file_suffix_name(dump_info->frame.video_frame.pixel_format)) == -1) {
        printf("set output file name failed!\n");
        return TD_FAILURE;
    }

    printf("Dump YUV frame of %s pipe %d chn %d to file: \"%s\"\n", dump_info->dis_enable ? vpss_name : vi_name,
        pipe, chn, yuv_name);

    /* open file */
    dump_info->yuv_fd = fopen(yuv_name, "wb");
    if (dump_info->yuv_fd == TD_NULL) {
        printf("open file failed:%s!\n", strerror(errno));
        return TD_FAILURE;
    }
    (td_void)fflush(stdout);
    return TD_SUCCESS;
}

static td_s32 dis_dump_set_vpss_depth(dis_dump_info *dump_info)
{
    const td_u32 depth = 2;
    td_s32 ret;
    ot_vpss_chn_attr chn_attr;
    ot_vpss_ext_chn_attr ext_chn_attr;
    ot_vpss_grp grp = dump_info->vpss_grp;
    ot_vpss_chn chn = dump_info->vpss_chn;

    if (chn >= OT_VPSS_MAX_PHYS_CHN_NUM) {
        ret = ss_mpi_vpss_get_ext_chn_attr(grp, chn, &ext_chn_attr);
        if (ret != TD_SUCCESS) {
            printf("get ext chn attr error!!!\n");
            return TD_FAILURE;
        }

        dump_info->orig_depth = ext_chn_attr.depth;
        ext_chn_attr.depth = depth;

        if (ss_mpi_vpss_set_ext_chn_attr(grp, chn, &ext_chn_attr) != TD_SUCCESS) {
            printf("set ext chn attr error!!!\n");
            return TD_FAILURE;
        }
    } else {
        ret = ss_mpi_vpss_get_chn_attr(grp, chn, &chn_attr);
        if (ret != TD_SUCCESS) {
            printf("get chn attr error!!!\n");
            return TD_FAILURE;
        }

        dump_info->orig_depth = chn_attr.depth;
        dump_info->orig_chn_mode = chn_attr.chn_mode;
        chn_attr.depth = depth;
        chn_attr.chn_mode = OT_VPSS_CHN_MODE_USER;

        if (ss_mpi_vpss_set_chn_attr(grp, chn, &chn_attr) != TD_SUCCESS) {
            printf("set chn attr error!!!\n");
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_s32 dis_dump_set_chn_depth(dis_dump_info *dump_info)
{
    if (dump_info->dis_enable == TD_FALSE) {
        if (dump_info->chn < OT_VI_MAX_PHYS_CHN_NUM) {
            ot_vi_chn_attr chn_attr;
            if (ss_mpi_vi_get_chn_attr(dump_info->vi_pipe, dump_info->chn, &chn_attr) != TD_SUCCESS) {
                return TD_FAILURE;
            }

            dump_info->orig_depth = chn_attr.depth;
            chn_attr.depth = DUMP_FRAME_DEPTH;

            if (ss_mpi_vi_set_chn_attr(dump_info->vi_pipe, dump_info->chn, &chn_attr) != TD_SUCCESS) {
                return TD_FAILURE;
            }
        } else {
            printf("vi_chn %d err\n", dump_info->chn);
            return TD_FAILURE;
        }
    } else {
        if (dis_dump_set_vpss_depth(dump_info) != TD_SUCCESS) {
            return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

static td_s32 dis_dump_get_chn_frame(dis_dump_info *dump_info, td_s32 milli_sec)
{
    td_s32 ret;
    if (dump_info->dis_enable == TD_TRUE) {
        ret = ss_mpi_vpss_get_chn_frame(dump_info->vpss_grp, dump_info->vpss_chn, &dump_info->frame, milli_sec);
        if (ret != TD_SUCCESS) {
            printf("ss_mpi_vpss_get_chn_frame failed\n");
            return TD_FAILURE;
        }
    } else {
        ret = ss_mpi_vi_get_chn_frame(dump_info->vi_pipe, dump_info->chn, &dump_info->frame, milli_sec);
        if (ret != TD_SUCCESS) {
            printf("ss_mpi_vi_get_chn_frame failed\n");
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_s32 dis_dump_release_chn_frame(dis_dump_info *dump_info)
{
    td_s32 ret;
    if (dump_info->dis_enable == TD_TRUE) {
        ret = ss_mpi_vpss_release_chn_frame(dump_info->vpss_grp, dump_info->vpss_chn, &dump_info->frame);
        if (ret != TD_SUCCESS) {
            printf("ss_mpi_vpss_release_chn_frame failed\n");
            return TD_FAILURE;
        }
    } else {
        ret = ss_mpi_vi_release_chn_frame(dump_info->vi_pipe, dump_info->chn, &dump_info->frame);
        if (ret != TD_SUCCESS) {
            printf("ss_mpi_vi_get_chn_frame failed\n");
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}


static td_s32 dis_dump_chn_try_get_frame(dis_dump_info *dump_info)
{
    td_s32 milli_sec = -1;
    td_s32 try_times = 10;
    td_s32 ret;

    while (dis_dump_get_chn_frame(dump_info, milli_sec) != TD_SUCCESS) {
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

    if (dis_dump_make_frame_file_name(dump_info) != TD_SUCCESS) {
        dis_dump_release_chn_frame(dump_info);
        dump_info->frame.pool_id = OT_VB_INVALID_POOL_ID;
        return TD_FAILURE;
    }

    ret = dis_dump_release_chn_frame(dump_info);
    if (ret != TD_SUCCESS) {
        printf("Release frame error ,now exit !!!\n");
        return TD_FAILURE;
    }

    dump_info->frame.pool_id = OT_VB_INVALID_POOL_ID;

    return TD_SUCCESS;
}

static td_void dis_dump_covert_chroma_sp42x_to_planar(const ot_video_frame *frame, FILE *fd,
    td_u32 uv_height, td_bool is_uv_invert, dis_dump_info *dump_info)
{
    /* If this value is too small and the image is big, this memory may not be enough */
    unsigned char tmp_buf[MAX_FRM_WIDTH];
    char *mem_content = TD_NULL;
    char *virt_addr_c = TD_NULL;
    td_u32 w, h;
    td_phys_addr_t phys_addr;

    phys_addr = frame->phys_addr[1];
    dump_info->user_page_addr[1] = (td_char *)ss_mpi_sys_mmap(phys_addr, dump_info->c_size);
    if (dump_info->user_page_addr[1] == TD_NULL) {
        printf("mmap chroma data error!!!\n");
        return;
    }
    virt_addr_c = dump_info->user_page_addr[1];

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
    if (dump_info->user_page_addr[1] != TD_NULL) {
        ss_mpi_sys_munmap(dump_info->user_page_addr[1], dump_info->c_size);
        dump_info->user_page_addr[1] = TD_NULL;
    }
}

/* When saving a file, sp420 will be denoted by p420 and sp422 will be denoted by p422 in the name of the file */
static td_void dis_dump_yuv_8bit_dump(ot_video_frame *frame, FILE *fd, dis_dump_info *dump_info)
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

    dump_info->size = (frame->stride[0]) * (frame->height);
    if (pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420 || pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420) {
        dump_info->c_size = (frame->stride[1]) * (frame->height) / 2; /* 2 uv height */
        uv_height = frame->height / 2; /* 2 uv height */
    } else if (pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422 ||
        pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422) {
        dump_info->c_size = (frame->stride[1]) * (frame->height);
        uv_height = frame->height;
    } else if (pixel_format == OT_PIXEL_FORMAT_YUV_400) {
        dump_info->c_size = 0;
        uv_height = frame->height;
    }

    phys_addr = frame->phys_addr[0];
    dump_info->user_page_addr[0] = (td_char *)ss_mpi_sys_mmap(phys_addr, dump_info->size);
    if (dump_info->user_page_addr[0] == TD_NULL) {
        return;
    }

    virt_addr_y = dump_info->user_page_addr[0];

    /* save Y */
    (td_void)fprintf(stderr, "saving......Y......");
    (td_void)fflush(stderr);

    for (h = 0; h < frame->height; h++) {
        mem_content = virt_addr_y + h * frame->stride[0];
        (td_void)fwrite(mem_content, frame->width, 1, fd);
    }

    (td_void)fflush(fd);
    if (pixel_format != OT_PIXEL_FORMAT_YUV_400) {
        dis_dump_covert_chroma_sp42x_to_planar(frame, fd, uv_height, is_uv_invert, dump_info);
    }

    (td_void)fprintf(stderr, "done %u!\n", frame->time_ref);
    (td_void)fflush(stderr);
    ss_mpi_sys_munmap(dump_info->user_page_addr[0], dump_info->size);
    dump_info->user_page_addr[0] = NULL;
}

static td_s32 dis_dump_start_msensormng(ot_msensor_param *msensor_param)
{
    td_s32 ret;
    td_s32 msensor_chipfd = -1;

    msensor_chipfd = open("/dev/motionsensor_chip", O_RDWR);
    if (msensor_chipfd < 0) {
        printf("open motionsensor_chip failed\n");
        return TD_FAILURE;
    }

    g_msensor_mngfd = open("/dev/motionsensor_mng", O_RDWR);
    if (g_msensor_mngfd < 0) {
        printf("open motionsensor_mng failed\n");
        goto exit1;
    }

    ret = ioctl(msensor_chipfd, MSENSOR_CMD_GET_PARAM, msensor_param);
    if (ret != TD_SUCCESS) {
        printf("msensor get param failed!\n");
        goto exit2;
    }
    close(msensor_chipfd);

    ret = ioctl(g_msensor_mngfd, MSENSOR_CMD_ADD_USER, &g_msensor_userfd);
    if (ret != TD_SUCCESS) {
        printf("MSENSOR_CMD_ADD_USER failed, ret:%x !\n", ret);
        goto exit1;
    }

    return  TD_SUCCESS;
exit2:
    close(g_msensor_mngfd);
    g_msensor_mngfd = -1;
exit1:
    close(msensor_chipfd);
    return TD_FAILURE;
}


static td_void dis_dump_stop_msensormng(dis_dump_info *dump_info)
{
    if (g_msensor_userfd > 0) {
        ioctl(g_msensor_mngfd, MSENSOR_CMD_DELETE_USER, &g_msensor_userfd);
        g_msensor_userfd = -1;
    }

    if (g_msensor_mngfd > 0) {
        close(g_msensor_mngfd);
        g_msensor_mngfd = -1;
    }

    if (msensor_param_fd != TD_NULL) {
        fclose(msensor_param_fd);
        msensor_param_fd = TD_NULL;
    }

    if (ldc_fd != TD_NULL) {
        fclose(ldc_fd);
        ldc_fd = TD_NULL;
    }

    if (dump_info->gyro_fd != TD_NULL) {
        fclose(dump_info->gyro_fd);
        dump_info->gyro_fd = TD_NULL;
    }

    if (dump_info->pts_fd != TD_NULL) {
        fclose(dump_info->pts_fd);
        dump_info->pts_fd = TD_NULL;
    }

    if (dump_info->acc_fd != TD_NULL) {
        fclose(dump_info->acc_fd);
        dump_info->acc_fd = TD_NULL;
    }

    if (dump_info->hv_fd != TD_NULL) {
        fclose(dump_info->hv_fd);
        dump_info->hv_fd = TD_NULL;
    }

    if (dump_info->msensor_buf_attr.virt_addr != TD_NULL) {
        ss_mpi_sys_munmap((td_void *)(td_uintptr_t)dump_info->msensor_buf_attr.virt_addr,
            dump_info->msensor_buf_attr.buf_len);
    }
}

static td_s32 dis_dump_mfusion_get_gyro_config(const td_u32 fusion_id,
    ot_mfusion_six_side_calibration *six_side_calibration, ot_mfusion_drift *drift,
    ot_mfusion_temperature_drift *temperature_drift)
{
    if (ss_mpi_mfusion_get_gyro_six_side_calibration(fusion_id, six_side_calibration) != TD_SUCCESS) {
        printf("dis_dump_get_gyro_six_side_calibration failed\n");
        return TD_FAILURE;
    }

    if (ss_mpi_mfusion_get_gyro_online_drift(fusion_id, drift) != TD_SUCCESS) {
        printf("dis_dump_get_gyro_six_side_calibration failed\n");
        return TD_FAILURE;
    }

    if (ss_mpi_mfusion_get_gyro_online_temperature_drift(fusion_id, temperature_drift) != TD_SUCCESS) {
        printf("dis_dump_get_gyro_six_side_calibration failed\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}


static td_s32 dis_dump_get_msensordata(ot_msensor_data_type data_type, td_u64 begin_pts, td_u64 end_pts,
    dis_dump_info *dump_info)
{
    td_s32 i, ret = TD_SUCCESS;
    td_u32 j, num;
    td_char data_buf[FILE_NAME_LENGTH] = {0};
    td_s32 *x_viraddr = TD_NULL;
    td_s32 *y_viraddr = TD_NULL;
    td_s32 *z_viraddr = TD_NULL;
    td_s32 *temp_viraddr = TD_NULL;
    td_u64 *pts_viraddr = TD_NULL;
    ot_msensor_data_info msensor_data_info = {0};
    msensor_data_info.id = g_msensor_userfd;
    msensor_data_info.data_type = data_type;
    msensor_data_info.begin_pts = begin_pts;
    msensor_data_info.end_pts = end_pts;

    ret = ioctl(g_msensor_mngfd, MSENSOR_CMD_GET_DATA, &msensor_data_info);
    if (ret != TD_SUCCESS) {
        printf("msensor_cmd_get_data failed\n");
        return TD_FAILURE;
    }

    for (i = 0; i < 2; i++) { /* cyclic buffer 2 */
        num = msensor_data_info.data[i].num;
        x_viraddr = (td_s32 *)(td_uintptr_t)(dump_info->msensor_buf_attr.virt_addr +
            ((td_u64)(td_uintptr_t)msensor_data_info.data[i].x_phys_addr - dump_info->msensor_buf_attr.phys_addr));
        y_viraddr = (td_s32 *)(td_uintptr_t)(dump_info->msensor_buf_attr.virt_addr +
            ((td_u64)(td_uintptr_t)msensor_data_info.data[i].y_phys_addr - dump_info->msensor_buf_attr.phys_addr));
        z_viraddr = (td_s32 *)(td_uintptr_t)(dump_info->msensor_buf_attr.virt_addr +
            ((td_u64)(td_uintptr_t)msensor_data_info.data[i].z_phys_addr - dump_info->msensor_buf_attr.phys_addr));
        temp_viraddr = (td_s32 *)(td_uintptr_t)(dump_info->msensor_buf_attr.virt_addr +
            ((td_u64)(td_uintptr_t)msensor_data_info.data[i].temperature_phys_addr -
            dump_info->msensor_buf_attr.phys_addr));
        pts_viraddr = (td_u64 *)(td_uintptr_t)(dump_info->msensor_buf_attr.virt_addr +
            ((td_u64)(td_uintptr_t)msensor_data_info.data[i].pts_phys_addr - dump_info->msensor_buf_attr.phys_addr));
        for (j = 0; j < num; j++) {
            (td_void)snprintf_s(data_buf, sizeof(data_buf), sizeof(data_buf) - 1, "%d,%d,%d,%d,%llu\n",
                x_viraddr[j], y_viraddr[j], z_viraddr[j], temp_viraddr[j], pts_viraddr[j]);
            (td_void)fwrite(data_buf, strlen(data_buf), 1,
                (MSENSOR_DATA_GYRO == data_type) ? dump_info->gyro_fd : dump_info->acc_fd);
        }
    }

    ret = ioctl(g_msensor_mngfd, MSENSOR_CMD_RELEASE_BUF, &msensor_data_info);
    if (ret != TD_SUCCESS) {
        printf("MSENSOR_CMD_RELEASE_BUF failed, ret:%x \n", ret);
        return ret;
    }
    return ret;
}

static td_void dis_dump_write_proc_title(const char *name)
{
    td_char title[FILE_NAME_LENGTH] = {0};
    td_s32 ret;

    ret = snprintf_s(title, sizeof(title),
        sizeof(title) - 1, "[%s]\n", name);
    if (ret <= 0) {
        printf("dis_dump_write_proc_title of %s faliure\n", name);
    }
    (td_void)fwrite(title, strlen(title), 1, msensor_param_fd);
}
static td_s32 dis_dump_get_mensor_param_config_msensor_param(ot_msensor_param *msensor_param)
{
    td_s32 ret;
    td_char msensor_param_attr[FILE_NAME_LENGTH] = {0};
    td_char msensor_param_config_gyro_config[FILE_NAME_LENGTH] = {0};
    td_char msensor_param_config_acc_config[FILE_NAME_LENGTH] = {0};
    ret = snprintf_s(msensor_param_attr, sizeof(msensor_param_attr), sizeof(msensor_param_attr) - 1,
        "device_mask=%u\ntemperature_mask=%u\n\n", msensor_param->attr.device_mask,
        msensor_param->attr.temperature_mask);
    if (ret <= 0) {
        printf("snprintf_s msensor_param_attr faliure\n");
        return TD_FAILURE;
    }
    ret = snprintf_s(msensor_param_config_gyro_config, sizeof(msensor_param_config_gyro_config),
        sizeof(msensor_param_config_gyro_config) - 1,
        "odr=%u\nfsr=%u\ndata_width=%u\ntemperature_max=%d\ntemperature_min=%d\n\n",
        msensor_param->config.gyro_config.odr, msensor_param->config.gyro_config.fsr,
        msensor_param->config.gyro_config.data_width, msensor_param->config.gyro_config.temperature_max,
        msensor_param->config.gyro_config.temperature_min);
    if (ret <= 0) {
        printf("snprintf_s msensor_param_config_gyro_config faliure\n");
        return TD_FAILURE;
    }
    ret = snprintf_s(msensor_param_config_acc_config, sizeof(msensor_param_config_acc_config),
        sizeof(msensor_param_config_acc_config) - 1,
        "odr=%u\nfsr=%u\ndata_width=%u\ntemperature_max=%d\ntemperature_min=%d\n\n",
        msensor_param->config.acc_config.odr, msensor_param->config.acc_config.fsr,
        msensor_param->config.acc_config.data_width, msensor_param->config.acc_config.temperature_max,
        msensor_param->config.acc_config.temperature_min);
    if (ret <= 0) {
        printf("snprintf_s msensor_param_config_acc_config faliure\n");
        return TD_FAILURE;
    }
    dis_dump_write_proc_title("msensor_attr");
    (td_void)fwrite(msensor_param_attr, strlen(msensor_param_attr), 1, msensor_param_fd);
    dis_dump_write_proc_title("gyro_config");
    (td_void)fwrite(msensor_param_config_gyro_config, strlen(msensor_param_config_gyro_config), 1, msensor_param_fd);
    dis_dump_write_proc_title("acc_config");
    (td_void)fwrite(msensor_param_config_acc_config, strlen(msensor_param_config_acc_config), 1, msensor_param_fd);
    return TD_SUCCESS;
}

static td_s32 dis_dump_get_mensor_param_config_calibration(ot_mfusion_six_side_calibration *six_side_calibration,
    ot_mfusion_drift *drift)
{
    td_s32 ret;
    td_char six_calibration[FILE_NAME_LENGTH] = {0};
    td_char gyro_drift_attr[FILE_NAME_LENGTH] = {0};

    ret = snprintf_s(six_calibration, sizeof(six_calibration), sizeof(six_calibration) - 1,
        "enable=%d\nmatrix=\"%d, %d, %d, %d, %d, %d, %d, %d, %d\"\n\n",
        six_side_calibration->enable,
        six_side_calibration->matrix[0], six_side_calibration->matrix[1],    /* index:0,1      */
        six_side_calibration->matrix[2], six_side_calibration->matrix[3],    /* index:2,3      */
        six_side_calibration->matrix[4], six_side_calibration->matrix[5],    /* index:4,5      */
        six_side_calibration->matrix[6], six_side_calibration->matrix[7],    /* index:6,7      */
        six_side_calibration->matrix[8]);    /* index:8      */
    if (ret <= 0) {
        printf("snprintf_s six_calibration faliure\n");
        return TD_FAILURE;
    }

    ret = snprintf_s(gyro_drift_attr, sizeof(gyro_drift_attr), sizeof(gyro_drift_attr) - 1,
        "enable=%d\nmatrix=\"%d, %d, %d\"\n\n",
        drift->enable, drift->drift[0], drift->drift[1], drift->drift[2]); /* index:0,1,2 */
    if (ret <= 0) {
        printf("snprintf_s gyro_drift_attr faliure\n");
        return TD_FAILURE;
    }

    dis_dump_write_proc_title("six_side_calibration");
    (td_void)fwrite(six_calibration, strlen(six_calibration), 1, msensor_param_fd);
    dis_dump_write_proc_title("gyro_drift_attr");
    (td_void)fwrite(gyro_drift_attr, strlen(gyro_drift_attr), 1, msensor_param_fd);
    return TD_SUCCESS;
}

static td_s32 dis_dump_get_mensor_param_config_temperature_drift(
    ot_mfusion_temperature_drift *temperature_drift)
{
    td_s32 i;
    td_s32 ret;
    td_char temperature_drift_attr[FILE_NAME_LENGTH] = {0};
    ret = snprintf_s(temperature_drift_attr, sizeof(temperature_drift_attr), sizeof(temperature_drift_attr) - 1,
        "enable=%d\nmode=%d\n;DRIFT_CURV = 0\n;DRIFT_LUT = 1\nrange_min=%d\nrange_max=%d\nstep=%u\n",
        temperature_drift->enable, temperature_drift->mode, temperature_drift->temperature_lut.range_min,
        temperature_drift->temperature_lut.range_max,
        temperature_drift->temperature_lut.step);
    if (ret <= 0) {
        printf("snprintf_s temperature_drift_attr faliure\n");
        return TD_FAILURE;
    }
    dis_dump_write_proc_title("temperature_drift_attr");
    (td_void)fwrite(temperature_drift_attr, strlen(temperature_drift_attr), 1, msensor_param_fd);

    ret = snprintf_s(temperature_drift_attr, sizeof(temperature_drift_attr), sizeof(temperature_drift_attr) - 1,
        ";%10s %15s %15s %15s %15s %10s\n", "x", "y", "z", "time", "nearest_temp", "temp");
    if (ret <= 0) {
        printf("snprintf_s temperature_drift_attr_x_y_z faliure\n");
        return TD_FAILURE;
    }
    (td_void)fwrite(temperature_drift_attr, strlen(temperature_drift_attr), 1, msensor_param_fd);
    ret = snprintf_s(temperature_drift_attr, sizeof(temperature_drift_attr), sizeof(temperature_drift_attr) - 1,
        ";%10s %15s %15s %15s %15s %10s\n", "x", "y", "z", "time", "nearest_temp", "temp");
    ret += snprintf_s(temperature_drift_attr, sizeof(temperature_drift_attr), sizeof(temperature_drift_attr) - 1,
        "temp_lut=\\\n");
    if (ret <= 0) {
        printf("snprintf_s temperature_drift_attr faliure\n");
        return TD_FAILURE;
    }
    (td_void)fwrite(temperature_drift_attr, strlen(temperature_drift_attr), 1, msensor_param_fd);
    for (i = 0; i < IMU_LUT_LEN; i++) {
        ret = snprintf_s(temperature_drift_attr, sizeof(temperature_drift_attr), sizeof(temperature_drift_attr) - 1,
        "%10d,%10d,%10d,%15d,%15d,%10u,\\\n", temperature_drift->temperature_lut.imu_lut[i][0], /* index:0 */
        temperature_drift->temperature_lut.imu_lut[i][1], /* index:1 */
        temperature_drift->temperature_lut.imu_lut[i][2], /* index:2 */
        temperature_drift->temperature_lut.gyro_lut_status[i][0], /* index:0 */
        temperature_drift->temperature_lut.imu_lut[i][1], /* index:1 */
        (temperature_drift->temperature_lut.range_min +
        temperature_drift->temperature_lut.step * i) / 1024); /* 1024 step per centigrade */
        if (ret <= 0) {
        printf("snprintf_s temperature_drift_attr_lut faliure\n");
        return TD_FAILURE;
        }
        (td_void)fwrite(temperature_drift_attr, strlen(temperature_drift_attr), 1, msensor_param_fd);
    }
    return TD_SUCCESS;
}

static td_s32 dis_dump_get_mensor_param_config(ot_msensor_param *msensor_param,
    ot_mfusion_six_side_calibration *six_side_calibration, ot_mfusion_drift *drift,
    ot_mfusion_temperature_drift *temperature_drift)
{
    td_s32 ret;
    td_char dis_other_attr[FILE_NAME_LENGTH] = {0};
    td_char dis_cfg_attr[FILE_NAME_LENGTH] = {0};

    if (dis_dump_get_mensor_param_config_msensor_param(msensor_param) != TD_SUCCESS) {
        printf("dis_dump_get_mensor_param_config1 faliure\n");
        return TD_FAILURE;
    }

    if (dis_dump_get_mensor_param_config_calibration(six_side_calibration, drift) != TD_SUCCESS) {
        printf("dis_dump_get_mensor_param_config2 faliure\n");
        return TD_FAILURE;
    }

    if (dis_dump_get_mensor_param_config_temperature_drift(temperature_drift) != TD_SUCCESS) {
        printf("dis_dump_get_mensor_param_config2 faliure\n");
        return TD_FAILURE;
    }
    ret = snprintf_s(dis_other_attr, sizeof(dis_other_attr), sizeof(dis_other_attr) - 1,
        "enable=%d\ntimelag=%d\nstrength=%u\n\n",
        dis_attr.enable, dis_attr.timelag, dis_attr.strength);
    if (ret <= 0) {
        printf("snprintf_s dis_other_attr faliure\n");
        return TD_FAILURE;
    }
    ret = snprintf_s(dis_cfg_attr, sizeof(dis_cfg_attr), sizeof(dis_cfg_attr) - 1,
        "pdt_type=%d\n;IPC = 0\n;DV = 1\ncrop_ratio=%u\nframe_rate=%u\ncamera_steady=%d\nscale=%d\n",
        dis_cfg.pdt_type, dis_cfg.crop_ratio, dis_cfg.frame_rate, dis_cfg.camera_steady, dis_cfg.scale);
    if (ret <= 0) {
        printf("snprintf_s dis_cfg_attr faliure\n");
        return TD_FAILURE;
    }

    dis_dump_write_proc_title("dis_attr");
    (td_void)fwrite(dis_other_attr, strlen(dis_other_attr), 1, msensor_param_fd);
    dis_dump_write_proc_title("dis_cfg");
    (td_void)fwrite(dis_cfg_attr, strlen(dis_cfg_attr), 1, msensor_param_fd);

    printf("fwrite =====mensor_param_config success=====\n");
    return TD_SUCCESS;
}

static td_s32 dis_dump_get_ldc_param_config(const ot_dis_attr *dis_attr)
{
    td_char dis_ldc[FILE_NAME_LENGTH * 2] = {0};
    td_char src_calibration_ratio[FILE_NAME_LENGTH] = {0};
    td_char dst_calibration_ratio[FILE_NAME_LENGTH] = {0};
    td_s32 ret;

    ret = snprintf_s(dis_ldc, sizeof(dis_ldc), sizeof(dis_ldc) - 1,
    "[dis_ldc_en]\nenable=%d\n\n[dis_ldc_attr]\nfocal_len_x=%d\nfocal_len_y=%d\n"
    "coord_shift_x=%d\ncoord_shift_y=%d\nmax_du=%d\n",
    dis_attr->dis_ldc_en, dis_attr->dis_ldc_attr.focal_len_x,
    dis_attr->dis_ldc_attr.focal_len_y, dis_attr->dis_ldc_attr.coord_shift_x,
    dis_attr->dis_ldc_attr.coord_shift_y, dis_attr->dis_ldc_attr.max_du);
    if (ret <= 0) {
        printf("snprintf_s dis_ldc faliure\n");
        return TD_FAILURE;
    }

    ret = snprintf_s(src_calibration_ratio, sizeof(src_calibration_ratio), sizeof(src_calibration_ratio) - 1,
    "src_calibration_ratio=\"%d, %d, %d, %d, %d, %d, %d, %d, %d\"\n",
    dis_attr->dis_ldc_attr.src_calibration_ratio[0], dis_attr->dis_ldc_attr.src_calibration_ratio[1], /* index:0,1 */
    dis_attr->dis_ldc_attr.src_calibration_ratio[2], dis_attr->dis_ldc_attr.src_calibration_ratio[3], /* index:2,3 */
    dis_attr->dis_ldc_attr.src_calibration_ratio[4], dis_attr->dis_ldc_attr.src_calibration_ratio[5], /* index:4,5 */
    dis_attr->dis_ldc_attr.src_calibration_ratio[6], dis_attr->dis_ldc_attr.src_calibration_ratio[7], /* index:6,7 */
    dis_attr->dis_ldc_attr.src_calibration_ratio[8]); /* index:8 */
    if (ret <= 0) {
        printf("snprintf_s src_calibration_ratio faliure\n");
        return TD_FAILURE;
    }

    ret = snprintf_s(dst_calibration_ratio, sizeof(dst_calibration_ratio), sizeof(dst_calibration_ratio) - 1,
    "dst_calibration_ratio=\"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\"\n",
    dis_attr->dis_ldc_attr.dst_calibration_ratio[0], dis_attr->dis_ldc_attr.dst_calibration_ratio[1], /* index:0,1 */
    dis_attr->dis_ldc_attr.dst_calibration_ratio[2], dis_attr->dis_ldc_attr.dst_calibration_ratio[3], /* index:2,3 */
    dis_attr->dis_ldc_attr.dst_calibration_ratio[4], dis_attr->dis_ldc_attr.dst_calibration_ratio[5], /* index:4,5 */
    dis_attr->dis_ldc_attr.dst_calibration_ratio[6], dis_attr->dis_ldc_attr.dst_calibration_ratio[7], /* index:6,7 */
    dis_attr->dis_ldc_attr.dst_calibration_ratio[8], dis_attr->dis_ldc_attr.dst_calibration_ratio[9], /* index:8,9 */
    dis_attr->dis_ldc_attr.dst_calibration_ratio[10], /* index:10 */
    dis_attr->dis_ldc_attr.dst_calibration_ratio[11], /* index:11 */
    dis_attr->dis_ldc_attr.dst_calibration_ratio[12], /* index:12 */
    dis_attr->dis_ldc_attr.dst_calibration_ratio[13]); /* index:13 */
    if (ret <= 0) {
        printf("snprintf_s dst_calibration_ratio faliure\n");
        return TD_FAILURE;
    }

    (td_void)fwrite(dis_ldc, strlen(dis_ldc), 1, ldc_fd);
    (td_void)fwrite(src_calibration_ratio, strlen(src_calibration_ratio), 1, ldc_fd);
    (td_void)fwrite(dst_calibration_ratio, strlen(dst_calibration_ratio), 1, ldc_fd);
    printf("fwrite =====get_ldc_param_config success=====\n");

    return TD_SUCCESS;
}

static td_s32 dis_dump_get_gyro_data(dis_dump_info *dump_info, td_u64 pts, dis_dump_isp_info *dis_isp)
{
    td_s32 i;
    td_char pts_buf[FILE_NAME_LENGTH] = {0};
    td_char h_v_buf[FILE_NAME_LENGTH] = {0};

    for (i = MSENSOR_DATA_GYRO; i <= MSENSOR_DATA_ACC; i++) {
        dump_info->end_pts = pts;
        if (dump_info->begin_pts == 0) {
            dump_info->begin_pts = pts - 1;
        } else {
            if (dis_dump_get_msensordata(i, dump_info->begin_pts, dump_info->end_pts, dump_info) != TD_SUCCESS) {
                return TD_FAILURE;
            }
            if (i == MSENSOR_DATA_ACC) {
                dump_info->begin_pts = dump_info->end_pts + 1;
            }
        }
    }

    (td_void)snprintf_s(pts_buf, sizeof(pts_buf), sizeof(pts_buf) - 1, "%u,%llu\n", dis_isp->exposuretime, pts);
    (td_void)snprintf_s(h_v_buf, sizeof(h_v_buf), sizeof(h_v_buf) - 1, "%u,%u\n", dis_isp->hmax, dis_isp->vmax);
    (td_void)fwrite(pts_buf, strlen(pts_buf), 1, dump_info->pts_fd);
    (td_void)fwrite(h_v_buf, strlen(h_v_buf), 1, dump_info->hv_fd);
    printf("fwrite =====gyro/acc/pts data success=====\n");

    return TD_SUCCESS;
}

static td_s32 dis_dump_get_isp_exposuretime(dis_dump_isp_info *dis_isp, ot_video_frame_info *frame_info)
{
    ot_isp_frame_info *isp_frame_info = TD_NULL;
    td_u64 isp_phyaddr = frame_info->video_frame.supplement.isp_info_phys_addr;

    isp_frame_info = (ot_isp_frame_info *)ss_mpi_sys_mmap_cached(isp_phyaddr, sizeof(ot_isp_frame_info));
    if (isp_frame_info == TD_NULL) {
        printf("ss_mpi_sys_mmap failed  %d\n", __LINE__);
        return TD_FAILURE;
    }
    dis_isp->exposuretime = isp_frame_info->exposure_time[0];
    dis_isp->hmax = isp_frame_info->hmax_times;
    dis_isp->vmax = isp_frame_info->vmax;
    ss_mpi_sys_munmap(isp_frame_info, sizeof(ot_isp_frame_info));
    return TD_SUCCESS;
}

static td_s32 dis_dump_chn_get_frame(dis_dump_info *dump_info)
{
    td_s32 ret;
    td_u32 cnt = dump_info->frame_cnt;
    td_s32 milli_sec = -1;
    dis_dump_isp_info dis_isp;

    /* get frame */
    while (cnt--) {
        if (g_signal_flag == 1) {
            printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
            return TD_FAILURE;
        }
        if (dis_dump_get_chn_frame(dump_info, milli_sec) != TD_SUCCESS) {
            printf("Get frame fail \n");
            usleep(1000); /* 1000 sleep */
            continue;
        }

        if (dump_info->dis_enable == TD_TRUE) {
            if (dis_dump_get_isp_exposuretime(&dis_isp, &dump_info->frame) != TD_SUCCESS) {
                dis_dump_release_chn_frame(dump_info);
                dump_info->frame.pool_id = OT_VB_INVALID_POOL_ID;
                return TD_FAILURE;
            }

            if (dis_dump_get_gyro_data(dump_info, dump_info->frame.video_frame.pts, &dis_isp) != TD_SUCCESS) {
                dis_dump_release_chn_frame(dump_info);
                dump_info->frame.pool_id = OT_VB_INVALID_POOL_ID;
                return TD_FAILURE;
            }
        }

        dis_dump_yuv_8bit_dump(&dump_info->frame.video_frame, dump_info->yuv_fd, dump_info);

        printf("Get frame %u!!\n", cnt);
        /* release frame after using */
        ret = dis_dump_release_chn_frame(dump_info);
        if (ret != TD_SUCCESS) {
            printf("Release frame error ,now exit !!!\n");
            return ret;
        }
        dump_info->frame.pool_id = OT_VB_INVALID_POOL_ID;
    }
    return TD_SUCCESS;
}

static td_void dis_dump_restore_vpss_default_depth(dis_dump_info *dump_info)
{
    td_s32 ret;
    ot_vpss_chn_attr chn_attr;
    ot_vpss_ext_chn_attr ext_chn_attr;

    if (dump_info->vpss_chn >= OT_VPSS_MAX_PHYS_CHN_NUM) {
        ret = ss_mpi_vpss_get_ext_chn_attr(dump_info->vpss_grp, dump_info->vpss_chn, &ext_chn_attr);
        if (ret != TD_SUCCESS) {
            printf("get ext chn attr error!!!\n");
        }

        ext_chn_attr.depth = dump_info->orig_depth;
        ret = ss_mpi_vpss_set_ext_chn_attr(dump_info->vpss_grp, dump_info->vpss_chn, &ext_chn_attr);
        if (ret != TD_SUCCESS) {
            printf("set ext chn attr error!!!\n");
        }
    } else {
        ret = ss_mpi_vpss_get_chn_attr(dump_info->vpss_grp, dump_info->vpss_chn, &chn_attr);
        if (ret != TD_SUCCESS) {
            printf("get chn attr error!!!\n");
        }

        chn_attr.depth = dump_info->orig_depth;
        chn_attr.chn_mode = dump_info->orig_chn_mode;
        ret = ss_mpi_vpss_set_chn_attr(dump_info->vpss_grp, dump_info->vpss_chn, &chn_attr);
        if (ret != TD_SUCCESS) {
            printf("set chn attr error!!!\n");
        }
    }
}

static td_s32 dis_dump_restore_chn_default_depth(dis_dump_info *dump_info)
{
    if (dump_info->dis_enable == TD_FALSE) {
        if (dump_info->chn < OT_VI_MAX_PHYS_CHN_NUM) {
            ot_vi_chn_attr chn_attr;
            if (ss_mpi_vi_get_chn_attr(dump_info->vi_pipe, dump_info->chn, &chn_attr) != TD_SUCCESS) {
                return TD_FAILURE;
            }
            chn_attr.depth = dump_info->orig_depth;
            if (ss_mpi_vi_set_chn_attr(dump_info->vi_pipe, dump_info->chn, &chn_attr) != TD_SUCCESS) {
                return TD_FAILURE;
            }
        } else {
            printf("vi_chn %d err\n", dump_info->chn);
            return TD_FAILURE;
        }
    } else {
        dis_dump_restore_vpss_default_depth(dump_info);
    }
    return TD_SUCCESS;
}

static td_void dis_dump_restore(dis_dump_info *dump_info)
{
    td_s32 ret;

    if (dump_info->frame.pool_id != OT_VB_INVALID_POOL_ID) {
        ret = dis_dump_release_chn_frame(dump_info);
        if (ret != TD_SUCCESS) {
            printf("Release Chn Frame error!!!\n");
        }
        dump_info->frame.pool_id = OT_VB_INVALID_POOL_ID;
    }

    if (dump_info->user_page_addr[0] != TD_NULL) {
        ss_mpi_sys_munmap(dump_info->user_page_addr[0], dump_info->size);
        dump_info->user_page_addr[0] = TD_NULL;
    }
    if (dump_info->user_page_addr[1] != TD_NULL) {
        ss_mpi_sys_munmap(dump_info->user_page_addr[1], dump_info->size);
        dump_info->user_page_addr[1] = TD_NULL;
    }

    if (dump_info->yuv_fd != TD_NULL) {
        (td_void)fclose(dump_info->yuv_fd);
        dump_info->yuv_fd = TD_NULL;
    }

    if (dump_info->depth_flag) {
        ret = dis_dump_restore_chn_default_depth(dump_info);
        if (ret != TD_SUCCESS) {
            printf("restore chn depth error!!!\n");
        }
        dump_info->depth_flag = 0;
    }
}

void *dis_dump_start_get_yuv(td_void *p)
{
    td_s32 ret;
    dis_dump_info *dump_info = TD_NULL;
    dump_info = (dis_dump_info *)p;

    ret = dis_dump_set_chn_depth(dump_info);
    if (ret != TD_SUCCESS) {
        printf("set chn dump depth failed!\n");
        return TD_NULL;
    }

    dump_info->depth_flag = 1;

    if (memset_s(&dump_info->frame, sizeof(ot_video_frame_info), 0, sizeof(ot_video_frame_info)) != EOK) {
        printf("memset_s frame error!!!\n");
        goto exit;
    }
    dump_info->frame.pool_id = OT_VB_INVALID_POOL_ID;

    if (dis_dump_chn_try_get_frame(dump_info) != TD_SUCCESS) {
        goto exit;
    }

    if (dis_dump_chn_get_frame(dump_info) != TD_SUCCESS) {
        goto exit;
    }

exit:
    dis_dump_restore(dump_info);
    return TD_NULL;
}

td_s32 dis_dump_venc_save_stream(FILE *fd, ot_venc_stream *stream)
{
    td_u32 i;

    for (i = 0; i < stream->pack_cnt; i++) {
        (td_void)fwrite(stream->pack[i].addr + stream->pack[i].offset,
                        stream->pack[i].len - stream->pack[i].offset, 1, fd);

        (td_void)fflush(fd);
    }
    printf("save stream success\n");

    return TD_SUCCESS;
}

static td_s32 dis_dump_venc_phys_addr_retrace(FILE *fd, ot_venc_stream_buf_info *stream_buf, ot_venc_stream *stream,
    td_u32 i, td_u32 j)
{
    td_u64 src_phys_addr;
    td_u32 left;
    td_s32 ret;

    if (stream->pack[i].phys_addr + stream->pack[i].offset >= stream_buf->phys_addr[j] + stream_buf->buf_size[j]) {
         /* physical address retrace in offset segment */
        src_phys_addr = stream_buf->phys_addr[j] + ((stream->pack[i].phys_addr + stream->pack[i].offset) -
            (stream_buf->phys_addr[j] + stream_buf->buf_size[j]));

        ret = fwrite((td_void *)(td_uintptr_t)src_phys_addr, stream->pack[i].len - stream->pack[i].offset, 1, fd);
        if (ret >= 0) {
            printf("fwrite err %d\n", ret);
            return ret;
        }
    } else {
        /* physical address retrace in data segment */
        left = (stream_buf->phys_addr[j] + stream_buf->buf_size[j]) - stream->pack[i].phys_addr;

        ret = fwrite((td_void *)(td_uintptr_t)(stream->pack[i].phys_addr + stream->pack[i].offset),
            left - stream->pack[i].offset, 1, fd);
        if (ret < 0) {
            printf("fwrite err %d\n", ret);
            return ret;
        }

        ret = fwrite((td_void *)(td_uintptr_t)stream_buf->phys_addr[j], stream->pack[i].len - left, 1, fd);
        if (ret < 0) {
            printf("fwrite err %d\n", ret);
            return ret;
        }
    }

    return TD_SUCCESS;
}


td_s32 dis_dump_venc_save_stream_phys_addr(FILE *fd, ot_venc_stream_buf_info *stream_buf, ot_venc_stream *stream)
{
    td_u32 i, j;
    td_s32 ret;

    for (i = 0; i < stream->pack_cnt; i++) {
        for (j = 0; j < OT_VENC_MAX_TILE_NUM; j++) {
            if ((stream->pack[i].phys_addr > stream_buf->phys_addr[j]) &&
                (stream->pack[i].phys_addr <= stream_buf->phys_addr[j] + stream_buf->buf_size[j])) {
                break;
            }
        }

        if (j < OT_VENC_MAX_TILE_NUM &&
            stream->pack[i].phys_addr + stream->pack[i].len >= stream_buf->phys_addr[j] + stream_buf->buf_size[j]) {
            ret = dis_dump_venc_phys_addr_retrace(fd, stream_buf, stream, i, j);
            if (ret < 0) {
                return ret;
            }
        } else {
            /* physical address retrace does not happen */
            ret = fwrite((td_void *)(td_uintptr_t)(stream->pack[i].phys_addr + stream->pack[i].offset),
                stream->pack[i].len - stream->pack[i].offset, 1, fd);
            if (ret < 0) {
                printf("fwrite err %d\n", ret);
                return ret;
            }
        }
        (td_void)fflush(fd);
    }
    printf("save stream success\n");
    return TD_SUCCESS;
}

static td_s32 dis_dump_save_frame_to_file(td_s32 venc_chn, ot_venc_stream *stream,
    ot_venc_stream_buf_info *stream_buf_info, dis_dump_info *dump_info)
{
    td_s32 ret;

#ifndef __LITEOS__
    ot_unused(stream_buf_info);
    ret = dis_dump_venc_save_stream(dump_info->stream_fd, stream);
#else
    ret = dis_dump_venc_save_stream_phys_addr(dump_info->stream_fd, stream_buf_info, stream);
#endif
    if (ret != TD_SUCCESS) {
        free(stream->pack);
        stream->pack = TD_NULL;
        printf("save stream failed!\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 dis_dump_get_venc_and_gyro(td_s32 venc_chn, ot_venc_stream *venc_stream,
    ot_venc_chn_status stat, list_exp *head, dis_dump_info *dump_info)
{
    td_s32 ret;
    td_u64 pts;
    dis_dump_isp_info dis_isp;
    list_exp *old_node = TD_NULL;

    venc_stream->pack_cnt = stat.cur_packs;

    venc_stream->pack = (ot_venc_pack *)malloc(sizeof(ot_venc_pack) * stat.cur_packs);
    if (venc_stream->pack == TD_NULL) {
        printf("malloc stream pack failed!\n");
        return TD_FAILURE;
    }

    ret = ss_mpi_venc_get_stream(venc_chn, venc_stream, TD_TRUE);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_venc_get_stream failed with %#x!\n", ret);
        goto exit1;
    }

    pts = venc_stream->pack->pts;
    old_node = list_find_node(pts, &dis_isp, head);
    if (dump_info->dump_mode == DIS_DUMP_IMU_DATA || dump_info->dump_mode == DIS_DUMP_STREAM_AND_IMU_DATA) {
        if (dis_dump_get_gyro_data(dump_info, pts, &dis_isp) != TD_SUCCESS) {
            printf("dis_dump_get_gyro_data faliure\n");
            goto exit2;
        }
    }

    if (old_node != TD_NULL) {
        list_del_node(old_node, head);
    }

    return TD_SUCCESS;
exit2:
    ss_mpi_venc_release_stream(venc_chn, venc_stream);
exit1:
    free(venc_stream->pack);
    venc_stream->pack = TD_NULL;
    return TD_FAILURE;
}


static td_void dis_dump_get_stream_from_chn(td_s32 venc_chn, list_exp *head, ot_venc_stream_buf_info *stream_buf,
    dis_dump_info *dump_info)
{
    td_s32 ret;
    ot_venc_stream venc_stream;
    ot_venc_chn_status stat;

    if (memset_s(&venc_stream, sizeof(ot_venc_stream), 0, sizeof(ot_venc_stream)) != EOK) {
        printf("memset_s venc_stream failed\n");
    }

    ret = ss_mpi_venc_query_status(venc_chn, &stat);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_venc_query_status chn[%d] failed with %#x!\n", venc_chn, ret);
        return;
    }

    if (stat.cur_packs == 0) {
        printf("NOTE: current  frame is TD_NULL!\n");
        return;
    }

    if (dis_dump_get_venc_and_gyro(venc_chn, &venc_stream, stat, head, dump_info) != TD_SUCCESS) {
        printf("get venc or gyro failed\n");
        return;
    }
    if (dump_info->dump_mode == DIS_DUMP_STREAM || dump_info->dump_mode == DIS_DUMP_STREAM_AND_IMU_DATA) {
        ret = dis_dump_save_frame_to_file(venc_chn, &venc_stream, stream_buf, dump_info);
        if (ret != TD_SUCCESS) {
            return;
        }
    }

    ret = ss_mpi_venc_release_stream(venc_chn, &venc_stream);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_venc_release_stream failed!\n");
        free(venc_stream.pack);
        venc_stream.pack = TD_NULL;
        return;
    }

    free(venc_stream.pack);
    venc_stream.pack = TD_NULL;

    return;
}

static td_s32 dis_dump_set_name_save_stream(td_s32 venc_chn, ot_venc_stream_buf_info *stream_buf, td_s32 *venc_fd,
    dis_dump_info *dump_info)
{
    td_s32 ret, fd;
    td_char stream_file[PATH_MAX] = {0};
    td_char file_name[PATH_MAX];

    if (snprintf_s(file_name, PATH_MAX, PATH_MAX - 1, "./") < 0) {
        return TD_FAILURE;
    }

    if (realpath(file_name, stream_file) == TD_NULL) {
        printf("chn[%d] stream file path error\n", venc_chn);
        return TD_FAILURE;
    }

    if (snprintf_s(stream_file, PATH_MAX, PATH_MAX - 1, "pipe%d_stream_chn%d.265", dump_info->vi_pipe, venc_chn) < 0) {
        return TD_FAILURE;
    }
    if (dump_info->dump_mode == DIS_DUMP_STREAM || dump_info->dump_mode == DIS_DUMP_STREAM_AND_IMU_DATA) {
        dump_info->stream_fd = fopen(stream_file, "wb");
        if (!dump_info->stream_fd) {
        printf("open file[%s] failed!\n", stream_file);
        return TD_FAILURE;
        }
        fd = fileno(dump_info->stream_fd);
        fchmod(fd, S_IRUSR | S_IWUSR);
    }

    *venc_fd = ss_mpi_venc_get_fd(venc_chn);
    if (*venc_fd < 0) {
        printf("ss_mpi_venc_get_fd failed with %#x!\n", *venc_fd);
        return TD_FAILURE;
    }

    ret = ss_mpi_venc_get_stream_buf_info(venc_chn, stream_buf);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_venc_get_stream_buf_info failed with %#x!\n", ret);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_void dis_dump_venc_add_pts_node(list_exp *head, ot_vi_pipe vi_pipe)
{
    dis_dump_isp_info dis_isp;
    td_s32 milli_sec = 3000;
    ot_video_frame_info frame_info;
    ot_vi_frame_dump_attr dump_attr, backup_dump_attr;

    if (ss_mpi_vi_get_pipe_frame_dump_attr(vi_pipe, &backup_dump_attr) != TD_SUCCESS) {
        printf("get pipe dump frame attr failed!\n");
        return;
    }

    dump_attr.enable = TD_TRUE;
    dump_attr.depth  = DUMP_FRAME_DEPTH;
    if (ss_mpi_vi_set_pipe_frame_dump_attr(vi_pipe, &dump_attr) != TD_SUCCESS) {
        printf("set pipe dump frame attr failed!\n");
        goto exit0;
    }

    if (ss_mpi_vi_get_pipe_frame(vi_pipe, &frame_info, milli_sec) != TD_SUCCESS) {
        printf("get pipe frame failed!\n");
        goto exit0;
    }

    if (dis_dump_get_isp_exposuretime(&dis_isp, &frame_info) != TD_SUCCESS) {
        goto exit1;
    }

    list_add_node(frame_info.video_frame.pts,  &dis_isp, head);

exit1:
    if (ss_mpi_vi_release_pipe_frame(vi_pipe, &frame_info) != TD_SUCCESS) {
        printf("set pipe backup dump frame attr failed!\n");
    }
exit0:
    if (ss_mpi_vi_set_pipe_frame_dump_attr(vi_pipe, &backup_dump_attr) != TD_SUCCESS) {
        printf("set pipe backup dump frame attr failed!\n");
    }
}

static td_s32 dis_dump_prepare_save_stream(dis_dump_info *dump_info, list_exp *head,
    ot_venc_stream_buf_info *stream_buf, td_s32 *venc_fd)
{
    td_s32 ret;
    td_s32 node_num;
    ot_venc_start_param rece_param = {0};
    if (dis_dump_set_name_save_stream(venc_chn, stream_buf, venc_fd, dump_info) != TD_SUCCESS) {
        printf("dis_dump_set_name_save_stream\n");
        return TD_FAILURE;
    }

    dump_info->picture_cnt = 0;

    for (node_num = 0; node_num < TOTAL_NODE_NUM; node_num++) {
        dis_dump_venc_add_pts_node(head, dump_info->vi_pipe);
    }

    ret = ss_mpi_venc_stop_chn(venc_chn);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_venc_stop_chn failed\n");
        return TD_FAILURE;
    }

    ret = ss_mpi_venc_reset_chn(venc_chn);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_venc_reset_chn failed\n");
        return TD_FAILURE;
    }

    rece_param.recv_pic_num = -1;

    ret = ss_mpi_venc_start_chn(venc_chn, &rece_param);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_venc_start_chn failed\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 dis_dump_get_save_stream(dis_dump_info *dump_info, td_s32 venc_chn)
{
    td_s32 venc_fd, ret;
    td_s32 cnt = dump_info->frame_cnt;
    ot_venc_stream_buf_info stream_buf;
    fd_set read_fds;
    struct timeval time_out_val;
    list_exp head = {0};
    if (dis_dump_prepare_save_stream(dump_info, &head, &stream_buf, &venc_fd) != TD_SUCCESS) {
        return TD_NULL;
    }
    while (dump_info->enable) {
        if (g_signal_flag == 1) {
            printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
            return TD_FAILURE;
        }
        dis_dump_venc_add_pts_node(&head, dump_info->vi_pipe);

        FD_ZERO(&read_fds);
        FD_SET(venc_fd, &read_fds);
        time_out_val.tv_sec = 2; /* 2 num */
        time_out_val.tv_usec = 0;
        ret = select(venc_fd + 1, &read_fds, NULL, NULL, &time_out_val);
        if (ret < 0) {
            printf("select venc fd, failed\n");
            break;
        } else if (ret == 0) {
            printf("get venc stream timeout, exit thread\n");
            continue;
        } else {
            if (FD_ISSET(venc_fd, &read_fds)) {
                dis_dump_get_stream_from_chn(venc_chn, &head, &stream_buf, dump_info);
            }
        }
        dump_info->picture_cnt++;
        if (dump_info->picture_cnt >= cnt) {
            break;
        }
    }

    list_del_all_node(&head);
    dump_info->picture_cnt = 0;
    return TD_SUCCESS;
}

static td_s32 dis_dump_venc_close_reencode(ot_venc_chn venc_chn)
{
    td_s32 ret;
    ot_venc_rc_param rc_param;

    ret = ss_mpi_venc_get_rc_param(venc_chn, &rc_param);
    if (ret != TD_SUCCESS) {
        printf("venc_chn :(%d)GetRcParam failed!,errcode 0x%x\n", venc_chn, ret);
        return TD_FAILURE;
    }

    rc_param.h265_cbr_param.max_reencode_times = 0;

    ret = ss_mpi_venc_set_rc_param(venc_chn, &rc_param);
    if (ret != TD_SUCCESS) {
        printf("venc_chn :(%d)SetRcParam failed!,errcode 0x%x\n", venc_chn, ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 dis_dump_venc_create(ot_venc_chn venc_chn)
{
    ot_venc_chn_attr venc_chn_attr;

    venc_chn_attr.venc_attr.type = OT_PT_H265;
    venc_chn_attr.venc_attr.max_pic_width = VENC_WIDTH;
    venc_chn_attr.venc_attr.max_pic_height = VENC_HEIGHT;
    venc_chn_attr.venc_attr.pic_width = VENC_WIDTH;
    venc_chn_attr.venc_attr.pic_height = VENC_HEIGHT;
    venc_chn_attr.venc_attr.buf_size = OT_ALIGN_UP(VENC_BUF_SIZE, VENC_STRIDE);
    venc_chn_attr.venc_attr.profile = 0;
    venc_chn_attr.venc_attr.is_by_frame = TD_TRUE;
    venc_chn_attr.venc_attr.h265_attr.frame_buf_ratio = DIS_FRAME_BUF_RATIO_MIN;
    venc_chn_attr.venc_attr.h265_attr.rcn_ref_share_buf_en = TD_FALSE;
    venc_chn_attr.rc_attr.rc_mode = OT_VENC_RC_MODE_H265_CBR;
    venc_chn_attr.gop_attr.gop_mode = OT_VENC_GOP_MODE_NORMAL_P;
    venc_chn_attr.gop_attr.normal_p.ip_qp_delta = DUMP_FRAME_DEPTH;
    venc_chn_attr.rc_attr.h265_cbr.gop = VENC_FRAME_RATE;
    venc_chn_attr.rc_attr.h265_cbr.stats_time = 1;
    venc_chn_attr.rc_attr.h265_cbr.src_frame_rate = VENC_FRAME_RATE;
    venc_chn_attr.rc_attr.h265_cbr.dst_frame_rate = VENC_FRAME_RATE;
    venc_chn_attr.rc_attr.h265_cbr.bit_rate = VENC_BIT_RATE;

    if (ss_mpi_venc_create_chn(venc_chn, &venc_chn_attr) != TD_SUCCESS) {
        printf("ss_mpi_venc_create_chn [%d] failed with! ===\n", venc_chn);
        return TD_FAILURE;
    }

    if (dis_dump_venc_close_reencode(venc_chn) != TD_SUCCESS) {
        ss_mpi_venc_destroy_chn(venc_chn);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 dis_dump_venc_start(ot_venc_chn venc_chn)
{
    ot_venc_start_param start_param;

    if (dis_dump_venc_create(venc_chn) != TD_SUCCESS) {
        printf("dis_dump_venc_create id:(%d) failed with! \n", venc_chn);
        return TD_FAILURE;
    }

    start_param.recv_pic_num = -1;
    if (ss_mpi_venc_start_chn(venc_chn, &start_param) != TD_SUCCESS) {
        printf("ss_mpi_venc_start_recv_pic failed with! \n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 dis_dump_venc_stop(ot_venc_chn venc_chn)
{
    td_s32 ret;

    ret = ss_mpi_venc_stop_chn(venc_chn);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_venc_stop_chn vechn[%d] failed with %#x!\n", venc_chn, ret);
    }

    ret = ss_mpi_venc_destroy_chn(venc_chn);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_venc_destroy_chn vechn[%d] failed with %#x!\n", venc_chn, ret);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static void *dis_dump_start_get_stream(td_void *p)
{
    dis_dump_info *dump_info = TD_NULL;
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;
    dump_info = (dis_dump_info *)p;
    ot_venc_chn_status venc_chn_status;
    venc_chn = dump_info->chn;

    src_chn.mod_id = OT_ID_VPSS;
    src_chn.dev_id = dump_info->vpss_grp;
    src_chn.chn_id = dump_info->vpss_chn;

    while (ss_mpi_venc_query_status(venc_chn, &venc_chn_status) == TD_SUCCESS) {
        printf("venc chn id(%d) have been created,so use last venc_chn.\n", venc_chn);
        venc_chn++;
        if (venc_chn >= MAX_VENC_NUM) {
            printf ("the venc_chn numbers should be smaller than 16\n");
            return TD_NULL;
        }
    }

    dest_chn.mod_id = OT_ID_VENC;
    dest_chn.dev_id = 0;
    dest_chn.chn_id = venc_chn;

    if (dis_dump_venc_start(venc_chn) != TD_SUCCESS) {
        return TD_NULL;
    }

    if (ss_mpi_sys_bind(&src_chn, &dest_chn) != TD_SUCCESS) {
        printf("vi bind venc failed\n");
        goto stop_venc;
    }

    if (dis_dump_get_save_stream(dump_info, venc_chn) != TD_SUCCESS) {
        printf("dis_dump_get_save_stream failed\n");
    }

    ss_mpi_sys_unbind(&src_chn, &dest_chn);

stop_venc:
    dis_dump_venc_stop(venc_chn);
    return TD_NULL;
}

static td_s32 dis_dump_get_config_file_name(dis_dump_info *dump_info,
    td_char *ldc_file, td_char *msensor_param_file)
{
    if (snprintf_s(ldc_file, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./vi_pipe%d_chn%d_dis_ldc.ini",
        dump_info->vi_pipe, dump_info->chn) == -1) {
        printf("set output  ldc_file name failed!\n");
        return TD_FAILURE;
    }
    if (snprintf_s(msensor_param_file, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./vi_pipe%d_chn%d_msensor_param.ini",
        dump_info->vi_pipe, dump_info->chn) == -1) {
        printf("set output msensor_param_file name failed!\n");
        return TD_FAILURE;
    }
    return  TD_SUCCESS;
}

static td_s32 dis_dump_get_data_file_name(dis_dump_info *dump_info,
    td_char *acc, td_char *gyro, td_char *pts, td_char *h_vmax)
{
    if (snprintf_s(pts, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./vi_pipe%d_chn%d_pts_%d.txt",
        dump_info->vi_pipe, dump_info->chn, dump_info->frame_cnt) == -1) {
        printf("set output pts_file name failed!\n");
        return TD_FAILURE;
    }

    if (snprintf_s(gyro, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./vi_pipe%d_chn%d_gyro_%d.txt",
        dump_info->vi_pipe, dump_info->chn, dump_info->frame_cnt) == -1) {
        printf("set output gyro_file name failed!\n");
        return TD_FAILURE;
    }

    if (snprintf_s(acc, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./vi_pipe%d_chn%d_acc_%d.txt",
        dump_info->vi_pipe, dump_info->chn, dump_info->frame_cnt) == -1) {
        printf("set output acc_file name failed!\n");
        return TD_FAILURE;
    }

    if (snprintf_s(h_vmax, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./vi_pipe%d_chn%d_hmax_vmax_%d.txt",
        dump_info->vi_pipe, dump_info->chn, dump_info->frame_cnt) == -1) {
        printf("set output hmax_vmax_file name failed!\n");
        return TD_FAILURE;
    }
    return  TD_SUCCESS;
}

static td_s32 dis_dump_set_dump_info_msensor_buf(ot_msensor_param *msensor_param, dis_dump_info *dump_info)
{
    ot_msensor_buf_attr *msensor_buf = &dump_info->msensor_buf_attr;

    if (memcpy_s(msensor_buf, sizeof(ot_msensor_buf_attr), &msensor_param->buf_attr, sizeof(ot_msensor_buf_attr)) !=
        EOK) {
        printf("memcpy_s msensor_buf failed\n");
        return TD_FAILURE;
    }
    msensor_buf->virt_addr = (td_u64)(td_uintptr_t)ss_mpi_sys_mmap_cached(msensor_buf->phys_addr, msensor_buf->buf_len);
    if (msensor_buf->virt_addr == TD_NULL) {
        printf("ss_mpi_sys_mmap_cached failed!\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 dis_dump_fopen_file(FILE **fd, td_char *file_name)
{
    *fd = fopen(file_name, "wb");
    if (*fd == TD_NULL) {
        printf("open %s failed\n", file_name);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}
static td_s32 dis_dump_open_gyro_config_file(ot_msensor_param *msensor_param, dis_dump_info *dump_info)
{
    td_char ldc_file[FILE_NAME_LENGTH] = {0};
    td_char msensor_param_file[FILE_NAME_LENGTH] = {0};

    ot_msensor_buf_attr *msensor_buf = &dump_info->msensor_buf_attr;

    if (dis_dump_get_config_file_name(dump_info, ldc_file, msensor_param_file) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if (dis_dump_set_dump_info_msensor_buf(msensor_param, dump_info) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if (dis_dump_fopen_file(&ldc_fd, ldc_file) != TD_SUCCESS) {
        goto exit1;
    }

    if (dis_dump_fopen_file(&msensor_param_fd, msensor_param_file) != TD_SUCCESS) {
        goto exit2;
    }

    (td_void)fflush(stdout);

    return  TD_SUCCESS;

exit2:
    if (ldc_fd != TD_NULL) {
        fclose(ldc_fd);
        ldc_fd = TD_NULL;
    }
exit1:
    ss_mpi_sys_munmap((td_void *)(td_uintptr_t)msensor_buf->virt_addr, msensor_buf->buf_len);
    return TD_FAILURE;
}

static td_s32 dis_dump_open_gyro_data_file(ot_msensor_param *msensor_param, dis_dump_info *dump_info)
{
    td_char acc_file[FILE_NAME_LENGTH] = {0};
    td_char gyro_file[FILE_NAME_LENGTH] = {0};
    td_char pts_file[FILE_NAME_LENGTH] = {0};
    td_char h_vmax_file[FILE_NAME_LENGTH] = {0};

    ot_msensor_buf_attr *msensor_buf = &dump_info->msensor_buf_attr;

    if (dis_dump_get_data_file_name(dump_info, acc_file, gyro_file, pts_file,
        h_vmax_file) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if (dis_dump_set_dump_info_msensor_buf(msensor_param, dump_info) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if (dis_dump_fopen_file(&dump_info->pts_fd, pts_file) != TD_SUCCESS) {
        goto exit1;
    }

    if (dis_dump_fopen_file(&dump_info->gyro_fd, gyro_file) != TD_SUCCESS) {
        goto exit2;
    }

    if (dis_dump_fopen_file(&dump_info->acc_fd, acc_file) != TD_SUCCESS) {
        goto exit3;
    }

    if (dis_dump_fopen_file(&dump_info->hv_fd, h_vmax_file) != TD_SUCCESS) {
        goto exit4;
    }

    (td_void)fflush(stdout);

    return  TD_SUCCESS;

exit4:
    if (dump_info->acc_fd != TD_NULL) {
        fclose(dump_info->acc_fd);
        dump_info->acc_fd = TD_NULL;
    }
exit3:
    if (dump_info->gyro_fd != TD_NULL) {
        fclose(dump_info->gyro_fd);
        dump_info->gyro_fd = TD_NULL;
    }
exit2:
    if (dump_info->pts_fd != TD_NULL) {
        fclose(dump_info->pts_fd);
        dump_info->pts_fd = TD_NULL;
    }
exit1:
    ss_mpi_sys_munmap((td_void *)(td_uintptr_t)msensor_buf->virt_addr, msensor_buf->buf_len);
    return TD_FAILURE;
}

static td_s32 dis_dump_get_input_num(td_s32 *input_num)
{
    td_s32 ret;
    td_char buf[DIS_DUMP_ARG_BASE] = {0};

    (td_void)fgets(buf, DIS_DUMP_ARG_BASE, stdin);
    ret = sscanf_s(buf, "%d", input_num);
    if (ret == -1) {
        printf("get input error");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_void dis_dump_get_vpss_grp_chn(dis_dump_info *dump_info)
{
    td_s32 vpss_grp_tmp;
    td_s32 vpss_chn_tmp;
    printf("This vi pipe(%d), dis is enable, need input vpss_grp vpss_chn\n", dump_info->vi_pipe);
    printf("please input vpss_grp: ");
    (td_void)fflush(stdout);

    while (g_signal_flag == 0) {
        if (dis_dump_get_input_num(&vpss_grp_tmp) == TD_FAILURE) {
            continue;
        }
        if ((vpss_grp_tmp >= 0) && (vpss_grp_tmp < OT_VPSS_MAX_GRP_NUM)) {
            break;
        } else {
            printf("\nInvalid param, please enter again!\n\n");
            printf("please input vpss_grp: ");
        }
    }
    dump_info->vpss_grp = vpss_grp_tmp;

    printf("\nplease input vpss_chn: ");
    (td_void)fflush(stdout);
    while (g_signal_flag == 0) {
        if (dis_dump_get_input_num(&vpss_chn_tmp) == TD_FAILURE) {
            continue;
        }
        if ((vpss_chn_tmp >= 0) && (vpss_chn_tmp < OT_VPSS_MAX_CHN_NUM)) {
            break;
        } else {
            printf("\nInvalid param, please enter again!\n\n");
            printf("please input vpss_chn: ");
        }
    }

    dump_info->vpss_chn = vpss_chn_tmp;
}

static td_s32 dis_dump_set_gyro_name(ot_msensor_param *msensor_param, dis_dump_info *dump_info0,
    dis_dump_info *dump_info1)
{
    ot_vi_chn chn = 0; // h8, h9 vi phys chn all is 0
    ot_dis_attr dis_attr0 = {0};
    ot_dis_attr dis_attr1 = {0};
    ot_dis_cfg dis_cfg0 = {0};

    if (ss_mpi_vi_get_chn_dis_attr(dump_info0->vi_pipe, chn, &dis_attr0) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    if (ss_mpi_vi_get_chn_dis_cfg(dump_info0->vi_pipe, chn, &dis_cfg0) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    dump_info0->dis_enable = dis_attr0.enable;
    dis_attr = dis_attr0;
    dis_cfg = dis_cfg0;

    if (ss_mpi_vi_get_chn_dis_attr(dump_info1->vi_pipe, chn, &dis_attr1) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    dump_info1->dis_enable = dis_attr1.enable;

    if (dump_info0->enable == TD_TRUE) {
        dis_dump_get_vpss_grp_chn(dump_info0);
        if (dis_dump_open_gyro_config_file(msensor_param, dump_info0) != TD_SUCCESS) {
                printf("dump_info0: dis_dump_open_gyro_config_file faliure\n");
                return TD_FAILURE;
        }
        if (dump_info0->dump_mode == DIS_DUMP_IMU_DATA || dump_info0->dump_mode == DIS_DUMP_STREAM_AND_IMU_DATA) {
            if (dis_dump_open_gyro_data_file(msensor_param, dump_info0) != TD_SUCCESS) {
                printf("dump_info0: dis_dump_open_gyro_file faliure\n");
                return TD_FAILURE;
            }
        }
    }

    if (dump_info1->enable == TD_TRUE) {
        dis_dump_get_vpss_grp_chn(dump_info1);
        if (dis_dump_open_gyro_config_file(msensor_param, dump_info1) != TD_SUCCESS) {
                printf("dump_info1: dis_dump_open_gyro_config_file faliure\n");
                return TD_FAILURE;
        }
        if (dump_info1->dump_mode == DIS_DUMP_IMU_DATA || dump_info1->dump_mode == DIS_DUMP_STREAM_AND_IMU_DATA) {
            if (dis_dump_open_gyro_data_file(msensor_param, dump_info1) != TD_SUCCESS) {
                printf("dump_info1: dis_dump_open_gyro_file faliure\n");
                return TD_FAILURE;
            }
        }
    }

    return TD_SUCCESS;
}
static td_void dis_dump_start_process(dis_dump_info *dump_info0, dis_dump_info *dump_info1)
{
    if (dump_info0->enable == TD_TRUE) {
        if (DUMP_YUV_MODE == 1) {
            pthread_create(&g_dis_dump_yuv_pid, NULL, dis_dump_start_get_yuv, dump_info0);
        }
        if (DUMP_STREAM_MODE == 1) {
        pthread_create(&g_dis_dump_stream_pid, NULL, dis_dump_start_get_stream, dump_info0);
        }
    }

    if (dump_info1->enable == TD_TRUE) {
        if (DUMP_YUV_MODE == 1) {
            pthread_create(&g_dis_dump_yuv_pid1, NULL, dis_dump_start_get_yuv, dump_info1);
        }
        if (DUMP_STREAM_MODE == 1) {
            pthread_create(&g_dis_dump_stream_pid1, NULL, dis_dump_start_get_stream, dump_info1);
        }
    }

    if (dump_info0->enable == TD_TRUE) {
        if (DUMP_YUV_MODE == 1) {
            pthread_join(g_dis_dump_yuv_pid, 0);
        }
        if (DUMP_STREAM_MODE == 1) {
            pthread_join(g_dis_dump_stream_pid, 0);
        }
    }
    if (dump_info1->enable == TD_TRUE) {
        if (DUMP_YUV_MODE == 1) {
            pthread_join(g_dis_dump_yuv_pid1, 0);
        }
        if (DUMP_STREAM_MODE == 1) {
            pthread_join(g_dis_dump_stream_pid1, 0);
        }
    }
}

static td_void dis_dump_process(dis_dump_info *dump_info0, dis_dump_info *dump_info1)
{
    ot_msensor_param msensor_param = {0};
    ot_mfusion_six_side_calibration six_side_calibration = {0};
    ot_mfusion_drift gyro_drift = {0};
    ot_mfusion_temperature_drift temperature_drift = {0};

    if (dis_dump_start_msensormng(&msensor_param) != TD_SUCCESS) {
        printf("dis_dump_start_msensormng failed\n");
        return;
    }

    if (dis_dump_mfusion_get_gyro_config(fusion_id, &six_side_calibration,
        &gyro_drift, &temperature_drift) != TD_SUCCESS) {
        printf("dis_dump_mfusion_get_gyro_config failed\n");
        goto exit0;
    }

    if (dis_dump_set_gyro_name(&msensor_param, dump_info0, dump_info1) != TD_SUCCESS) {
        printf("dis_dump_start_msensormng failed\n");
        goto exit0;
    }

    if (dis_dump_get_mensor_param_config(&msensor_param, &six_side_calibration, &gyro_drift,
        &temperature_drift) != TD_SUCCESS) {
        printf("dis_dump_get_mensor_param faliure\n");
        goto exit0;
    }

    if (dis_dump_get_ldc_param_config(&dis_attr) != TD_SUCCESS) {
        printf("dis_dump_get_ldc_param_config faliure\n");
        goto exit0;
    }

    dis_dump_start_process(dump_info0, dump_info1);
exit0:
    dis_dump_stop_msensormng(dump_info0);
    if (dump_info1->enable == TD_TRUE) {
        dis_dump_stop_msensormng(dump_info1);
    }
    printf("exit success!!!\n");
}

static td_s32 dis_dump_set_info(ot_vi_pipe vi_pipe, td_s32 chn, td_s32 frame_cnt, td_s32 dump_mode,
    dis_dump_info *dump_info)
{
    dump_info->enable = TD_TRUE;

    if (!value_between(vi_pipe, 0, OT_VI_MAX_PIPE_NUM - 1)) {
        printf("pipe id must be [0,%d]!!!!\n\n", OT_VI_MAX_PIPE_NUM - 1);
        return TD_FAILURE;
    }
    dump_info->vi_pipe = vi_pipe;

    if (!value_between(dump_mode, 0, 2)) { /* min:0 max:2 */
        printf("dump_mode must be [0, 2]!!!!\n\n");
        return TD_FAILURE;
    }
    dump_info->dump_mode = (dis_dump_mode)dump_mode;

    if (DUMP_YUV_MODE == 1) {
        if (!value_between(chn, 0, OT_VI_MAX_CHN_NUM - 1)) {
            printf("chn id must be [0,%d]!!!!\n\n", OT_VI_MAX_CHN_NUM - 1);
            return TD_FAILURE;
        }
    }
    if (dump_mode == DIS_DUMP_STREAM)   {
        if (!value_between(chn, 0, OT_VENC_MAX_CHN_NUM - 1)) {
            printf("pipe id must be [0,%d]!!!!\n\n", OT_VENC_MAX_CHN_NUM - 1);
            return TD_FAILURE;
        }
    }

    dump_info->chn = chn;
    if (frame_cnt < MIN_FRAMENUM) {
        printf("the frame cnt(%d) is invalid!\nthe frame cnt should be greater than 1!\n\n", frame_cnt);
        return TD_FAILURE;
    }
    dump_info->frame_cnt = frame_cnt;

    return TD_SUCCESS;
}

static td_s32 dis_dump_check_argv(char *argv[], td_s32 index, td_s32 *val)
{
    td_char *end_ptr = TD_NULL;
    td_slong result;

    errno = 0;
    result = strtol(argv[index], &end_ptr, DIS_DUMP_ARG_BASE);
    if ((errno == ERANGE && (result == LONG_MAX || result == LONG_MIN)) || (errno != 0 && result == 0)) {
        return TD_FAILURE;
    }
    if ((end_ptr == argv[index]) || (*end_ptr != '\0')) {
        return TD_FAILURE;
    }

    *val = (td_s32)result;

    return TD_SUCCESS;
}

static td_s32 dis_dump_chang_mode(dis_dump_info *dump_info0, dis_dump_info *dump_info1, char *argv[], int argc)
{
    ot_vi_pipe vi_pipe;
    td_s32 chn;
    td_s32 frame_cnt;
    td_s32 dump_mode;

    if (dis_dump_check_argv(argv, 1, &vi_pipe) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if (dis_dump_check_argv(argv, 2, &chn) != TD_SUCCESS) { /* 2 args */
        return TD_FAILURE;
    }
    if (dis_dump_check_argv(argv, 3, &frame_cnt) != TD_SUCCESS) { /* 3 args */
        return TD_FAILURE;
    }

    if (dis_dump_check_argv(argv, 4, &dump_mode) != TD_SUCCESS) { /* 4 args */
        return TD_FAILURE;
    }

    if (dis_dump_set_info(vi_pipe, chn, frame_cnt, dump_mode, dump_info0) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    if (argc == 9) { /* 9 args */
            if (dis_dump_check_argv(argv, 5, &vi_pipe) != TD_SUCCESS) { /* 5 args */
                return TD_FAILURE;
            }

            if (dis_dump_check_argv(argv, 6, &chn) != TD_SUCCESS) { /* 6 args */
                return TD_FAILURE;
            }

            if (dis_dump_check_argv(argv, 7, &frame_cnt) != TD_SUCCESS) { /* 7 args */
                return TD_FAILURE;
            }

            if (dis_dump_check_argv(argv, 8, &dump_mode) != TD_SUCCESS) { /* 8 args */
                return TD_FAILURE;
            }
        if (dis_dump_set_info(vi_pipe, chn, frame_cnt, dump_mode, dump_info1) != TD_SUCCESS) {
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_bool dis_dump_check_valid_digit_str(const td_char *str)
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

void dis_dump_handle_sig(int signo)
{
    if (g_signal_flag) {
        return;
    }

    if (signo == SIGINT || signo == SIGTERM) {
        g_signal_flag = 1;
    }
    return;
}

static td_void usage(td_void)
{
    printf("\n"
           "*************************************************\n"
           "Usage: ./dis_dump [pipe] [chn] [FrmCnt] [dump mode] \n"
           "       ./dis_dump [pipe] [chn] [FrmCnt] [dump mode]  [pipe] [chn] [FrmCnt] [dump mode]\n"
           "1)pipe: \n"
           "   vi pipe id\n"
           "2)chn: \n"
           "   chn is venc chn id, which will be created to dump stream\n"
           "3)FrmCnt: \n"
           "   the count of frame to be dump ,the value should be greater than 1\n"
           "4)dump mode: \n"
           "   dump mode == 0: dump stream\n"
           "   dump mode == 1: dump imu_data mode\n"
           "   dump mode == 2: dump stream and imu_data mode\n"
           "*)Example:\n"
           "   e.g : ./dis_dump 0 0 300 0\n"
           "   e.g : ./dis_dump 0 0 300 2 1 1 300 2\n"
           "*)set DIS_GET_CHN_FRAME_CONTINUOUSLY to 1 to get continuous frame\n"
           "*************************************************\n"
           "\n");
}


#ifdef __LITEOS__
td_s32 dis_dump(int argc, char *argv[])
#else
td_s32 main(int argc, char *argv[])
#endif
{
    td_s32 i;
    dis_dump_info dump_info0 = {0};
    dis_dump_info dump_info1 = {0};
#ifndef __LITEOS__
    sig_t sig_handler;
#endif
    printf("\nNOTICE: This tool only can be used for dump imu data and yuv!\n");
    printf("\tTo see more usage, please enter: ./dis_dump -h\n\n");
    if (argv == TD_NULL) {
        usage();
        return TD_SUCCESS;
    }
    if (argc > 1) {
        if (!strncmp(argv[1], "-h", 2)) { /* 2 help */
            usage();
            return TD_SUCCESS;
        }
    }
    if ((argc != 5) && (argc != 9)) { /* 5, 9 args */
        usage();
        return TD_SUCCESS;
    }

    for (i = 1; i < argc; i++) {
        if (!dis_dump_check_valid_digit_str(argv[i])) {
            printf("the %dth value :'%s' is invalid, must be reasonable non negative integers!!!!\n\n", i, argv[i]);
            usage();
            return -1;
        }
    }

    if (dis_dump_chang_mode(&dump_info0, &dump_info1, argv, argc) != TD_SUCCESS) {
        return -1;
    }

#ifndef __LITEOS__
    sig_handler = signal(SIGINT, dis_dump_handle_sig);
    if (sig_handler == SIG_ERR) {
        perror("error\n");
    }
    sig_handler = signal(SIGTERM, dis_dump_handle_sig);
    if (sig_handler == SIG_ERR) {
        perror("error\n");
    }
#endif
    dis_dump_process(&dump_info0, &dump_info1);
    return TD_SUCCESS;
}
