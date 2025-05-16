/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <math.h>
#include <ctype.h>
#include "sample_comm.h"

#define FRAME_NUM_MAX 10000

static volatile sig_atomic_t g_sig_flag = 0;
static sample_sns_type g_sns_type = SENSOR0_TYPE;
static sample_vi_cfg g_vi_video_cfg;

typedef struct {
    td_u16 x;
    td_u16 y;
    td_u16 w;
    td_u16 h;
} sample_rect;

typedef struct {
    ot_vi_pipe vi_pipe;
    pthread_t thread_id;
    td_bool start;
} sample_thread_info;

typedef struct {
    sample_rect rect;
    td_u16 frames;
    td_double *corrcoef;
} sample_corrcoef_result;

static sample_thread_info g_sample_thread_info;

static td_u32 frame_cnt = 10000;
static td_u32 frame_interval = 15;
static sample_rect g_sample_rect = { 0, 0, 40, 40 };
static ot_vi_pipe g_vi_pipe = 0;

static td_void sample_get_char(td_void)
{
    if (g_sig_flag == 1) {
        return;
    }
    sample_pause();
}

static td_void sample_vi_get_default_vb_config(ot_size *size, ot_vb_cfg *vb_cfg)
{
    ot_vb_calc_cfg calc_cfg;
    ot_pic_buf_attr buf_attr;

    (td_void)memset_s(vb_cfg, sizeof(ot_vb_cfg), 0, sizeof(ot_vb_cfg));
    vb_cfg->max_pool_cnt = 128; /* 128 blks */

    buf_attr.width = size->width;
    buf_attr.height = size->height;
    buf_attr.align = OT_DEFAULT_ALIGN;
    buf_attr.bit_width = OT_DATA_BIT_WIDTH_8;
    buf_attr.pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422;
    buf_attr.compress_mode = OT_COMPRESS_MODE_SEG;
    buf_attr.video_format = OT_VIDEO_FORMAT_LINEAR;
    ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);

    vb_cfg->common_pool[0].blk_size = calc_cfg.vb_size;
    vb_cfg->common_pool[0].blk_cnt = 30; /* 30 blks */
}

static td_s32 sample_correlation_sys_init(ot_vi_vpss_mode_type mode_type, ot_vi_aiisp_mode aiisp_mode,
    sample_sns_type sns_type)
{
    td_s32 ret;
    ot_size size;
    ot_vb_cfg vb_cfg;
    td_u32 supplement_config;

    sample_comm_vi_get_size_by_sns_type(sns_type, &size);
    sample_vi_get_default_vb_config(&size, &vb_cfg);

    supplement_config = OT_VB_SUPPLEMENT_BNR_MOT_MASK;
    ret = sample_comm_sys_init_with_vb_supplement(&vb_cfg, supplement_config);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    ret = sample_comm_vi_set_vi_vpss_mode(mode_type, aiisp_mode);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 sample_correlation_start_video_route(ot_vi_pipe video_pipe)
{
    td_s32 ret;

    sample_comm_vi_get_default_vi_cfg(g_sns_type, &g_vi_video_cfg);
    g_vi_video_cfg.pipe_info[0].pipe_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    ret = sample_comm_vi_start_vi(&g_vi_video_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = ss_mpi_vi_set_pipe_frame_source(video_pipe, OT_VI_PIPE_FRAME_SOURCE_USER);
    if (ret != TD_SUCCESS) {
        sample_print("set pipe frame source failed, ret: 0x%x!\n", ret);
        sample_comm_vi_stop_vi(&g_vi_video_cfg);
        return ret;
    }

    return TD_SUCCESS;
}

static td_void sample_correlation_stop_video_route(ot_vi_pipe video_pipe)
{
    sample_comm_vi_stop_vi(&g_vi_video_cfg);
}

static td_s32 vi_raw_convert_bit_pixel(const td_u8 *data, td_u32 data_num, td_u32 bit_width, td_u16 *out_data)
{
    td_s32 i, tmp_data_num, out_cnt;
    td_u32 u32_val;
    td_u64 u64_val;
    const td_u8 *tmp_data = data;

    out_cnt = 0;
    switch (bit_width) {
        case 10:                         /* 10: 10bit */
            tmp_data_num = data_num / 4; /* 4 pixels consist of 5 bytes  */
            for (i = 0; i < tmp_data_num; i++) {
                tmp_data = data + 5 * i; /* 5: include 5bytes */
                /* 0/8/16/24/32: byte align */
                u64_val = tmp_data[0] + ((td_u32)tmp_data[1] << 8) + ((td_u32)tmp_data[2] << 16) +
                    ((td_u32)tmp_data[3] << 24) + ((td_u64)tmp_data[4] << 32); /* 3/4: index, 24/32: align */

                out_data[out_cnt++] = (td_u16)((u64_val >> 0) & 0x3ff);  /* 0:  10 bit align */
                out_data[out_cnt++] = (td_u16)((u64_val >> 10) & 0x3ff); /* 10: 10 bit align */
                out_data[out_cnt++] = (td_u16)((u64_val >> 20) & 0x3ff); /* 20: 10 bit align */
                out_data[out_cnt++] = (td_u16)((u64_val >> 30) & 0x3ff); /* 30: 10 bit align */
            }
            break;
        case 12:                         /* 12: 12bit */
            tmp_data_num = data_num / 2; /* 2 pixels consist of 3 bytes  */
            for (i = 0; i < tmp_data_num; i++) {
                tmp_data = data + 3 * i;                                          /* 3: include 3bytes */
                u32_val = tmp_data[0] + (tmp_data[1] << 8) + (tmp_data[2] << 16); /* 1/2: index, 8/16: align */
                out_data[out_cnt++] = (td_u16)(u32_val & 0xfff);
                out_data[out_cnt++] = (td_u16)((u32_val >> 12) & 0xfff); /* 12: 12 bit align */
            }
            break;
        case 14:                         /* 14: 14bit */
            tmp_data_num = data_num / 4; /* 4 pixels consist of 7 bytes  */
            for (i = 0; i < tmp_data_num; i++) {
                tmp_data = data + 7 * i; /* 7: include 7bytes */
                u64_val = tmp_data[0] +
                    ((td_u32)tmp_data[1] <<  8) + ((td_u32)tmp_data[2] << 16) + /* 1/2: index, 8/16:  align */
                    ((td_u32)tmp_data[3] << 24) + ((td_u64)tmp_data[4] << 32) + /* 3/4: index, 24/32: align */
                    ((td_u64)tmp_data[5] << 40) + ((td_u64)tmp_data[6] << 48);  /* 5/6: index, 40/48: align */

                out_data[out_cnt++] = (td_u16)((u64_val >> 0) & 0x3fff);  /* 0:  14 bit align */
                out_data[out_cnt++] = (td_u16)((u64_val >> 14) & 0x3fff); /* 14: 14 bit align */
                out_data[out_cnt++] = (td_u16)((u64_val >> 28) & 0x3fff); /* 28: 14 bit align */
                out_data[out_cnt++] = (td_u16)((u64_val >> 42) & 0x3fff); /* 42: 14 bit align */
            }
            break;
        default:
            sample_print("unsuport bit_width: %d\n", bit_width);
            return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_u32 vi_get_raw_bit_width(ot_pixel_format pixel_format)
{
    td_u32 bit_width;

    switch (pixel_format) {
        case OT_PIXEL_FORMAT_RGB_BAYER_8BPP:
            bit_width = 8; /* 8: 8bit */
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
            bit_width = 8; /* 8: 8bit */
            break;
    }

    return bit_width;
}

static td_void sample_output_corrcoef(sample_corrcoef_result *result)
{
    td_u16 w = result->rect.w;
    td_u16 h = result->rect.h;

    td_double avg_corrcoef = 0.0;
    td_double r_max = -1.0;
    td_double r_min = 1.0;
    for (td_u32 index = 0; index < w * h; index++) {
        avg_corrcoef += result->corrcoef[index];
        if (result->corrcoef[index] < r_min) {
            r_min = result->corrcoef[index];
        } else if (result->corrcoef[index] > r_max) {
            r_max = result->corrcoef[index];
        }
    }
    avg_corrcoef /= w * h;

    printf("sample frames: %u\n", result->frames);
    printf("min_corrcoef: %f, max_corrcoef: %f, avg_corrcoef: %f\n", r_min, r_max, avg_corrcoef);
    if (avg_corrcoef >= 0.100000) { // >0.100000 correlation
        printf("There is correlation between horizontal adjacent pixels.");
    } else {
        printf("There is no correlation between horizontal adjacent pixels.");
    }

    return;
}

static td_s32 sample_get_statistic(ot_video_frame *frame, sample_rect rect, td_u16 *statistic_data, td_u16 *length)
{
    td_s32 ret = TD_SUCCESS;
    td_u8 *virt_addr;
    td_u8 *u8_data = TD_NULL;
    td_u32 size;
    td_u16 *u16_data = TD_NULL;
    td_u32 nbit = vi_get_raw_bit_width(frame->pixel_format);

    size = (frame->stride[0]) * (frame->height);
    virt_addr = (td_u8 *)ss_mpi_sys_mmap(frame->phys_addr[0], size);
    if (virt_addr == TD_NULL) {
        sample_print("ss_mpi_sys_mmap failed!\n");
        return TD_FAILURE;
    }

    u8_data = virt_addr;
    if ((nbit != 8) && (nbit != 16)) {                              /* 8/16: bit width */
        u16_data = (td_u16 *)malloc(frame->width * sizeof(td_u16)); /* 2: 2bytes */
        if (u16_data == TD_NULL) {
            sample_print("malloc memory failed\n");
            ret = TD_FAILURE;
            goto exit;
        }
    }

    printf("getting statistic_data frame : %u\n", *length);
    u8_data += frame->stride[0] * rect.y;
    td_u32 start_index = rect.w * rect.h * (*length);
    for (td_u32 height = 0; height < rect.h; height++) {
        td_u32 index = start_index + rect.w * height;
        if ((nbit != 8) && (nbit != 16)) { /* 8/16: bit width */
            vi_raw_convert_bit_pixel(u8_data, frame->width, nbit, u16_data);
            ret = memcpy_s(statistic_data + index, rect.w * 2, u16_data + rect.x, rect.w * 2); /* 2: 2bytes */
        } else {
            td_u32 width_bytes = (rect.w * nbit + 7) / 8; /* 7/8: align */
            ret = memcpy_s(statistic_data + index, width_bytes, u8_data + rect.x, width_bytes);
        }
        if (ret != EOK) {
            goto exit;
        }
        u8_data += frame->stride[0];
    }
    *length += 1;

exit:
    if (u16_data != TD_NULL) {
        free(u16_data);
    }
    ss_mpi_sys_munmap(virt_addr, size);
    virt_addr = TD_NULL;
    return ret;
}

static td_void sample_corr_calc_clear(td_double *avg, td_double *var, sample_corrcoef_result *result)
{
    if (TD_NULL != result->corrcoef) {
        free(result->corrcoef);
    }
    if (TD_NULL != avg) {
        free(avg);
    }
    if (TD_NULL != var) {
        free(var);
    }
}

static td_double safe_divide(td_double r_covar, td_double val_result)
{
    if (val_result < 1E-10) {
        return 1.0;
    } else {
        return r_covar / val_result;
    }
}

static td_void sample_calc_corrcoef(sample_rect rect, td_u16 *data, td_u16 data_length)
{
    td_u16 w = rect.w;
    td_u16 h = rect.h;

    sample_corrcoef_result result;
    result.rect.x = rect.x;
    result.rect.y = rect.y;
    result.rect.w = w - 2; /* rect.w-2: result.rect.w */
    result.rect.h = h;
    result.frames = data_length;

    if (data_length == 0) {
        sample_print("data_length is 0, error\n");
        return;
    }
    result.corrcoef = (td_double *)malloc(result.rect.w * result.rect.h * sizeof(td_double));
    td_double *avg = (td_double *)malloc(w * h * sizeof(td_double));
    td_double *var = (td_double *)malloc(w * h * sizeof(td_double));
    if (var == NULL || avg == NULL || result.corrcoef == NULL) {
        sample_print("malloc memory failed\n");
        sample_corr_calc_clear(avg, var, &result);
        return;
    }

    for (td_u32 i = 0; i < w * h; i++) {
        // average
        td_u64 sum = 0;
        for (td_u32 num = 0; num < data_length; num++) {
            sum += data[w * h * num + i];
        }

        avg[i] = sum / (td_double)data_length;
        // variance
        sum = 0;
        for (td_u32 num = 0; num < data_length; num++) {
            td_double dif = data[w * h * num + i] - avg[i];
            sum += dif * dif;
        }
        var[i] = sum / (td_double)data_length;
    }

    for (td_u32 h0 = 0; h0 < result.rect.h; h0++) {
        for (td_u32 w0 = 0; w0 < result.rect.w; w0++) {
            td_u64 sum_r = 0;
            for (td_u32 num = 0; num < data_length; num++) {
                sum_r += data[w * h * num + h0 * w + w0] * data[w * h * num + h0 * w + (w0 + 2)]; /* 2: r_pixel */
            }
            td_u32 index = h0 * result.rect.w + w0;
            // covariance
            td_double r_covar =
                (sum_r / (td_double)data_length) - (avg[h0 * w + w0] * avg[h0 * w + (w0 + 2)]); /* 2: r_pixel */
            // correlation coefficient
            td_double val_result = sqrt(var[h0 * w + w0]) * sqrt(var[h0 * w + (w0 + 2)]); /* 2: r_pixel */
            result.corrcoef[index] = safe_divide(r_covar, val_result);
        }
    }
    // output corrcoef result
    sample_output_corrcoef(&result);

    sample_corr_calc_clear(avg, var, &result);
}

static td_void sample_correlation_calc_usage(td_void)
{
    printf("\n"
        "*************************************************\n"
        "Usage: ./sample_correlation_calc [g_vi_pipe] [g_sample_rect(x y w h)] [frame_cnt] [frame_interval]\n"
        "g_vi_pipe: \n"
        "   vi pipe id \n"
        "g_sample_rect(x y w h) : \n"
        "   statistics ROI area: x1, y1, width, height, default is 0 0 40 40\n"
        "e.g : ./sample_correlation_calc 0 1000 500 40 40 \n"
        "frame_cnt: \n"
        "   total number of frames to be calculated, default is 10000, max is 10000\n"
        "frame_interval: \n"
        "   every frame_interval frames, get one frame of data, default is 15\n"
        "e.g : ./sample_correlation_calc 0 1000 500 40 40 10000 10 \n"
        "*************************************************\n"
        "\n");
}

static td_s32 sample_arg_check(td_void)
{
    td_s32 ret;
    const td_s32 milli_sec = -1;
    ot_video_frame_info get_frame_info;

    if (g_vi_pipe >= OT_VI_MAX_PIPE_NUM) {
        sample_print("vi_pipe id must be [0,%d).\n", OT_VI_MAX_PIPE_NUM);
        return TD_FAILURE;
    }
    if (frame_cnt < 2 || frame_cnt > FRAME_NUM_MAX) { /* at least 2 frames to calculate the average and var. */
        sample_print("frame cnt must be [2,%d).\n", FRAME_NUM_MAX);
        return TD_FAILURE;
    }

    ret = ss_mpi_vi_get_pipe_frame(g_vi_pipe, &get_frame_info, milli_sec);
    if (ret != TD_SUCCESS) {
        sample_print("get pipe frame failed!\n");
        return TD_FAILURE;
    }
    td_u32 size = (get_frame_info.video_frame.stride[0]) * (get_frame_info.video_frame.height);
    td_u8 *virt_addr = (td_u8 *)ss_mpi_sys_mmap(get_frame_info.video_frame.phys_addr[0], size);
    ss_mpi_sys_munmap(virt_addr, size);
    virt_addr = TD_NULL;
    ret = ss_mpi_vi_release_pipe_frame(g_vi_pipe, &get_frame_info);
    if (ret != TD_SUCCESS) {
        sample_print("release pipe frame failed!\n");
        return TD_FAILURE;
    }

    if (g_sample_rect.w <= 2 || g_sample_rect.h == 0) { /* g_sample_rect.w > 2 */
        sample_print("g_sample_rect error! the statistics ROI area is too small.\n");
        return TD_FAILURE;
    }
    if (get_frame_info.video_frame.width <= g_sample_rect.x + g_sample_rect.w ||
        get_frame_info.video_frame.height <= g_sample_rect.y + g_sample_rect.h) {
        sample_print("g_sample_rect error! the statistics ROI area is out of the frame.\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 read_frame_and_get_statistic(td_void *param, td_u16 *statistic_length, td_u16 *statistic_data)
{
    td_s32 ret;
    td_s32 ret1 = TD_SUCCESS;
    ot_video_frame_info get_frame_info;
    const td_s32 milli_sec = -1;
    sample_thread_info *thread_info = (sample_thread_info *)param;

    td_u32 count = 0;
    while (thread_info->start == TD_TRUE && (*statistic_length) < frame_cnt) {
        ret = ss_mpi_vi_get_pipe_frame(g_vi_pipe, &get_frame_info, milli_sec);
        if (ret != TD_SUCCESS) {
            sample_print("get pipe frame failed!\n");
            return TD_FAILURE;
        }

        count += 1;
        if (count % (frame_interval + 1) == 0) {
            // get statistic_data
            ret1 = sample_get_statistic(&get_frame_info.video_frame, g_sample_rect, statistic_data, statistic_length);
        }

        ret = ss_mpi_vi_release_pipe_frame(g_vi_pipe, &get_frame_info);
        if (ret != TD_SUCCESS) {
            sample_print("release pipe frame failed!\n");
            return TD_FAILURE;
        }
        if (ret1 != TD_SUCCESS) {
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_void *sample_capture_thread(td_void *param)
{
    td_s32 ret = TD_SUCCESS;
    ot_vi_frame_dump_attr dump_attr;

    dump_attr.enable = TD_TRUE;
    dump_attr.depth = 2; /* 2: dump depth set 2 */
    if (ss_mpi_vi_set_pipe_frame_dump_attr(g_vi_pipe, &dump_attr) != TD_SUCCESS) {
        sample_print("set pipe frame dump attr failed! \n");
        return TD_NULL;
    }

    ret = sample_arg_check();
    if (ret != TD_SUCCESS) {
        sample_correlation_calc_usage();
        g_sig_flag = 1;
        return TD_NULL;
    }

    td_u16 statistic_length = 0;
    td_u16 *statistic_data;
    statistic_data = (td_u16 *)malloc(g_sample_rect.w * g_sample_rect.h * frame_cnt * sizeof(td_u16));
    if (statistic_data == TD_NULL) {
        sample_print("malloc statistic_data memory failed, frame_cnt or the ROI area needs to be reduced.\n");
        // The maximum memory is about (80000000 * sizeof(td_u16)).
        return TD_NULL;
    }

    ret = read_frame_and_get_statistic(param, &statistic_length, statistic_data);
    if (ret != TD_SUCCESS) {
        free(statistic_data);
        return TD_NULL;
    }

    // calc correlation coefficient
    printf("start calc correlation coefficient\n");
    sample_calc_corrcoef(g_sample_rect, statistic_data, statistic_length);

    free(statistic_data);

    return TD_NULL;
}

static td_s32 sample_correlation_create_capture_thread(ot_vi_pipe vi_pipe)
{
    td_s32 ret;

    g_sample_thread_info.vi_pipe = vi_pipe;
    ret = pthread_create(&g_sample_thread_info.thread_id, TD_NULL, sample_capture_thread, &g_sample_thread_info);
    if (ret != 0) {
        sample_print("create capture thread failed!\n");
        return TD_FAILURE;
    }
    g_sample_thread_info.start = TD_TRUE;

    return TD_SUCCESS;
}

static td_void sample_correlation_destroy_capture_thread(td_void)
{
    if (g_sample_thread_info.start == TD_TRUE) {
        g_sample_thread_info.start = TD_FALSE;
        pthread_join(g_sample_thread_info.thread_id, NULL);
    }
}

static td_s32 sample_correlation_offline(td_void)
{
    td_s32 ret;
    const ot_vi_vpss_mode_type mode_type = OT_VI_OFFLINE_VPSS_OFFLINE;
    const ot_vi_aiisp_mode aiisp_mode = OT_VI_AIISP_MODE_DEFAULT;

    ret = sample_correlation_sys_init(mode_type, aiisp_mode, g_sns_type);
    if (ret != TD_SUCCESS) {
        goto sys_init_failed;
    }

    ret = sample_correlation_start_video_route(g_vi_pipe);
    if (ret != TD_SUCCESS) {
        goto start_video_route_failed;
    }

    ret = sample_correlation_create_capture_thread(g_vi_pipe);
    if (ret != TD_SUCCESS) {
        goto create_capture_thread_failed;
    }

    sample_get_char();

    sample_correlation_destroy_capture_thread();

create_capture_thread_failed:
    sample_correlation_stop_video_route(g_vi_pipe);
start_video_route_failed:
    sample_comm_sys_exit();
sys_init_failed:
    return ret;
}

static td_void sample_correlation_handle_sig(td_s32 signo)
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

static td_s32 isdigitstr(const td_char *str)
{
    size_t len = strlen(str);
    size_t i;
    for (i = 0; i < len; ++i) {
        if (!isdigit(str[i])) {
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_s32 pre_check_args(td_s32 argc, td_char *argv[])
{
    td_s32 i;
    if (argc < 2 || argc > 8 || (argc > 2 && argc < 6)) { /* 2:arg num, 8:arg num, 6:arg num */
        sample_correlation_calc_usage();
        return TD_FAILURE;
    }
    if (!strncmp(argv[1], "-h", 2)) { /* 2 :arg num */
        sample_correlation_calc_usage();
        return TD_FAILURE;
    }
    for (i = 1; i < argc; ++i) {
        if (isdigitstr(argv[i]) != TD_SUCCESS) {
            sample_correlation_calc_usage();
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_void parse_args(td_s32 argc, td_char *argv[])
{
    td_char *para_stop;
    if (argc == 8) {  /* 8 : arg num */
        frame_interval = (td_u32)strtol(argv[7], &para_stop, 10);  /* 7 : arg index, 10 : decimal */
    }
    if (argc >= 7) {  /* 7: arg num */
        frame_cnt = (td_u32)strtol(argv[6], &para_stop, 10);       /* 6 : arg index, 10 : decimal */
    }
    if (argc >= 6) {  /* 6 :arg num */
        g_sample_rect.x = (td_u16)strtol(argv[2], &para_stop, 10); /* 2 : arg index, 10 : decimal */
        g_sample_rect.y = (td_u16)strtol(argv[3], &para_stop, 10); /* 3 : arg index, 10 : decimal */
        g_sample_rect.w = (td_u16)strtol(argv[4], &para_stop, 10); /* 4 : arg index, 10 : decimal */
        g_sample_rect.h = (td_u16)strtol(argv[5], &para_stop, 10); /* 5 : arg index, 10 : decimal */
    }
    if (argc >= 2) {  /* 2 :arg num */
        g_vi_pipe = (ot_vi_pipe)strtol(argv[1], &para_stop, 10); /* 1 : arg index, 10 : decimal */
    }
}

#ifdef __LITEOS__
td_s32 app_main(td_s32 argc, td_char *argv[])
#else
td_s32 main(td_s32 argc, td_char *argv[])
#endif
{
    td_s32 ret;
    if (pre_check_args(argc, argv) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    parse_args(argc, argv);

#ifndef __LITEOS__
    sample_register_sig_handler(sample_correlation_handle_sig);
#endif
    ret = sample_correlation_offline();
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
