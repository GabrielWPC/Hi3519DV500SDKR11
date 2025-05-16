/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <signal.h>
#include <limits.h>
#include "ss_mpi_sys.h"
#include "ss_mpi_sys_mem.h"
#include "securec.h"

#include "ot_common_aiisp.h"
#ifdef OT_AIBNR_SUPPORT
#include "ss_mpi_aibnr.h"
#endif
#ifdef OT_AIDRC_SUPPORT
#include "ss_mpi_aidrc.h"
#endif
#ifdef OT_AI3DNR_SUPPORT
#include "ss_mpi_ai3dnr.h"
#endif

#define MAX_AIISP_TYPE          2
#define MAX_AIISP_DUMP_MODE     2
#define MAX_DUMP_FRAME_CNT      64
#define DUMP_FRAME_DEPTH        2
#define MAX_FRM_WIDTH           8192
#define FILE_NAME_LENGTH        128
#define PIXEL_FORMAT_STRING_LEN 10
#define DUMP_INPUT 0
#define DUMP_OUTPUT 1
#define DUMP_INOUT 2
#define MAX_AIDRC_OUT 2

static volatile sig_atomic_t g_signal_flag = 0;

static td_s32 g_aiisp_pipe = 0;
static td_char g_input_file_name[FILE_NAME_LENGTH];
static td_char g_output_file_name[FILE_NAME_LENGTH];
static td_char g_output_aidrc_file_name[MAX_AIDRC_OUT][FILE_NAME_LENGTH];

static td_char *g_user_page_addr[2] = { TD_NULL, TD_NULL }; /* 2 Y and C */
static td_u32 g_size = 0;
static td_u32 g_c_size = 0;

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

static td_char *get_aiisp_type_name(ot_aiisp_type aiisp_type)
{
    switch (aiisp_type) {
        case OT_AIISP_TYPE_AIBNR:
            return "aibnr";
        case OT_AIISP_TYPE_AIDRC:
            return "aidrc";
        case OT_AIISP_TYPE_AIDM:
            return "aidm";
        case OT_AIISP_TYPE_AI3DNR:
            return "ai3dnr";
        default:
            return "unsupport";
    }
}

static td_s32 aiisp_make_pipe_frame_file_name(td_s32 pipe, const ot_video_frame_info *frame_info,
    ot_aiisp_type aiisp_type, td_s32 mode, td_u32 frame_cnt, td_u32 byte_align)
{
    td_char *file_name = (mode == 0) ? g_input_file_name : g_output_file_name;

    /* make file name */
    if (snprintf_s(file_name, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./%s_pipe%d_%s_%ux%u_%s_%u_%u.%s",
        get_aiisp_type_name(aiisp_type), pipe, (mode == 0) ? "input" : "output",
        frame_info->video_frame.width, frame_info->video_frame.height,
        get_pixel_format_str(frame_info->video_frame.pixel_format),
        frame_cnt,
        byte_align,
        get_file_suffix_name(frame_info->video_frame.pixel_format)) == -1) {
        printf("set output file name failed!\n");
        return TD_FAILURE;
    }

    printf("Dump frame of vi pipe %d to file: \"%s\"\n", pipe, file_name);

    if ((mode == 1) && (aiisp_type == OT_AIISP_TYPE_AIDRC)) {
        if (snprintf_s(g_output_aidrc_file_name[0], FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1,
            "./%s_pipe%d_%ux%u_%u.%s",
            get_aiisp_type_name(aiisp_type), pipe,
            frame_info->video_frame.width, frame_info->video_frame.height,
            frame_cnt,
            "out0") == -1) {
            printf("set detail file name failed!\n");
            return TD_FAILURE;
        }
        printf("Dump frame of vi pipe %d to file: \"%s\"\n", pipe, g_output_aidrc_file_name[0]);

        if (snprintf_s(g_output_aidrc_file_name[1], FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1,
            "./%s_pipe%d_%ux%u_%u.%s",
            get_aiisp_type_name(aiisp_type), pipe,
            frame_info->video_frame.width, frame_info->video_frame.height,
            frame_cnt,
            "out1") == -1) {
            printf("set coef file name failed!\n");
            return TD_FAILURE;
        }

        printf("Dump frame of vi pipe %d to file: \"%s\"\n", pipe, g_output_aidrc_file_name[1]);
    }

    return TD_SUCCESS;
}

static td_s32 aiisp_get_input_frame(td_s32 pipe, ot_aiisp_type aiisp_type, ot_video_frame_info *frame_info,
    td_s32 milli_sec)
{
    td_s32 ret = TD_FAILURE;

    switch (aiisp_type) {
#ifdef OT_AIBNR_SUPPORT
        case OT_AIISP_TYPE_AIBNR:
            ret = ss_mpi_aibnr_get_input_frame(pipe, frame_info, milli_sec);
            break;
#endif
#ifdef OT_AI3DNR_SUPPORT
        case OT_AIISP_TYPE_AI3DNR:
            ret = ss_mpi_ai3dnr_get_input_frame(pipe, frame_info, milli_sec);
            break;
#endif

        default:
            printf("unsupport type:%d\n", aiisp_type);
    }

    return ret;
}

static td_s32 aiisp_release_input_frame(td_s32 pipe, ot_aiisp_type aiisp_type, ot_video_frame_info *frame_info)
{
    td_s32 ret = TD_FAILURE;

    switch (aiisp_type) {
#ifdef OT_AIBNR_SUPPORT
        case OT_AIISP_TYPE_AIBNR:
            ret = ss_mpi_aibnr_release_input_frame(pipe, frame_info);
            break;
#endif
#ifdef OT_AI3DNR_SUPPORT
        case OT_AIISP_TYPE_AI3DNR:
            ret = ss_mpi_ai3dnr_release_input_frame(pipe, frame_info);
            break;
#endif

        default:
            printf("unsupport type:%d\n", aiisp_type);
    }

    return ret;
}

static td_s32 aiisp_get_output_frame(td_s32 pipe, ot_aiisp_type aiisp_type, ot_video_frame_info *frame_info,
    td_s32 milli_sec)
{
    td_s32 ret = TD_FAILURE;

    switch (aiisp_type) {
#ifdef OT_AIBNR_SUPPORT
        case OT_AIISP_TYPE_AIBNR:
            ret = ss_mpi_aibnr_get_output_frame(pipe, frame_info, milli_sec);
            break;
#endif
#ifdef OT_AI3DNR_SUPPORT
        case OT_AIISP_TYPE_AI3DNR:
            ret = ss_mpi_ai3dnr_get_output_frame(pipe, frame_info, milli_sec);
            break;
#endif

        default:
            printf("unsupport type:%d\n", aiisp_type);
    }

    return ret;
}

static td_s32 aiisp_release_output_frame(td_s32 pipe, ot_aiisp_type aiisp_type, ot_video_frame_info *frame_info)
{
    td_s32 ret = TD_FAILURE;

    switch (aiisp_type) {
#ifdef OT_AIBNR_SUPPORT
        case OT_AIISP_TYPE_AIBNR:
            ret = ss_mpi_aibnr_release_output_frame(pipe, frame_info);
            break;
#endif
#ifdef OT_AIDRC_SUPPORT
        case OT_AIISP_TYPE_AIDRC:
            ret = ss_mpi_aidrc_release_output_frame(pipe, frame_info);
            break;
#endif
#ifdef OT_AI3DNR_SUPPORT
        case OT_AIISP_TYPE_AI3DNR:
            ret = ss_mpi_ai3dnr_release_output_frame(pipe, frame_info);
            break;
#endif

        default:
            printf("unsupport type:%d\n", aiisp_type);
    }

    return ret;
}

static td_s32 aiisp_pipe_dump_create_file_frame(td_s32 pipe, ot_aiisp_type aiisp_type, td_s32 mode,
    td_u32 frame_cnt, td_u32 byte_align)
{
    td_s32 ret = TD_FAILURE;
    ot_video_frame_info frame_info;
    td_s32 milli_sec = 3000; /* 3000 ms */

    if (mode == DUMP_INPUT || mode == DUMP_INOUT) {
        ret = aiisp_get_input_frame(pipe, aiisp_type, &frame_info, milli_sec);
        if (ret != TD_SUCCESS) {
            printf("get pipe frame failed!\n");
            return TD_FAILURE;
        }

        ret = aiisp_make_pipe_frame_file_name(pipe, &frame_info, aiisp_type, 0, frame_cnt, byte_align);

        if (aiisp_release_input_frame(pipe, aiisp_type, &frame_info) != TD_SUCCESS) {
            printf("release pipe frame failed!\n");
            return TD_FAILURE;
        }
    }

    if (mode == DUMP_OUTPUT || mode == DUMP_INOUT) {
        if (aiisp_type == OT_AIISP_TYPE_AIDRC) {
#ifdef OT_AIDRC_SUPPORT
            ot_aiisp_mem_info out_data[MAX_AIDRC_OUT];
            ot_aiisp_mem_info *out[MAX_AIDRC_OUT];

            out[0] = &out_data[0];
            out[1] = &out_data[1];
            ret = ss_mpi_aidrc_get_output_frame(pipe, &frame_info, out, MAX_AIDRC_OUT, milli_sec);
#endif
        } else {
            ret = aiisp_get_output_frame(pipe, aiisp_type, &frame_info, milli_sec);
        }

        if (ret != TD_SUCCESS) {
            printf("get pipe frame failed!\n");
            return TD_FAILURE;
        }

        // 1 means dump output
        ret = aiisp_make_pipe_frame_file_name(pipe, &frame_info, aiisp_type, 1, frame_cnt, byte_align);

        if (aiisp_release_output_frame(pipe, aiisp_type, &frame_info) != TD_SUCCESS) {
            printf("release pipe frame failed!\n");
            return TD_FAILURE;
        }
    }

    return ret;
}

static td_void aiisp_chn_dump_covert_chroma_sp42x_to_planar(const ot_video_frame *frame, FILE *fd,
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

static td_void aiisp_save_8bit_yuv_file(const ot_video_frame *frame, FILE *pfd)
{
    td_u32 h;
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
        (td_void)fwrite(mem_content, frame->width, 1, pfd);
    }

    (td_void)fflush(pfd);
    if (pixel_format != OT_PIXEL_FORMAT_YUV_400) {
        aiisp_chn_dump_covert_chroma_sp42x_to_planar(frame, pfd, uv_height, is_uv_invert);
    }

    (td_void)fprintf(stderr, "done %u!\n", frame->time_ref);
    (td_void)fflush(stderr);
    ss_mpi_sys_munmap(g_user_page_addr[0], g_size);
    g_user_page_addr[0] = NULL;
}

static td_s32 aiisp_raw_convert_bit_pixel(const td_u8 *data, td_u32 data_num, td_u32 bit_width, td_u16 *out_data)
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
            printf("unsuport bit_width: %d\n", bit_width);
            return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_u32 aiisp_get_raw_bit_width(ot_pixel_format pixel_format)
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

static td_void aiisp_save_raw_file(const ot_video_frame *v_buf, td_u32 byte_align, FILE *pfd)
{
    td_u32 height;
    td_phys_addr_t phys_addr;
    td_u32 size;
    td_u8 *virt_addr;
    td_u16 *u16_data = TD_NULL;
    td_u8 *u8_data = TD_NULL;
    td_u32 nbit = aiisp_get_raw_bit_width(v_buf->pixel_format);

    printf("nbit:%u\n", nbit);

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
            aiisp_raw_convert_bit_pixel(u8_data, v_buf->width, nbit, u16_data);
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

static td_void aiisp_save_one_frame(const ot_video_frame *video_frame, td_u32 byte_align, FILE *pfd)
{
    if ((video_frame->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422) ||
        (video_frame->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) ||
        (video_frame->pixel_format == OT_PIXEL_FORMAT_YUV_400)) {
        aiisp_save_8bit_yuv_file(video_frame, pfd);
    } else if ((video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_8BPP) ||
               (video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_10BPP) ||
               (video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_12BPP) ||
               (video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_14BPP) ||
               (video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_16BPP)) {
        aiisp_save_raw_file(video_frame, byte_align, pfd);
    } else {
        printf("unsupported pixel format (%d)!\n", video_frame->pixel_format);
    }
}

static td_s32 aiisp_pipe_save_dump_input_frame(td_s32 pipe, ot_aiisp_type aiisp_type, td_u32 byte_align,
    FILE *fd)
{
    td_s32 ret;
    ot_video_frame_info frame_info;
    td_s32 milli_sec = -1;

    ret = aiisp_get_input_frame(pipe, aiisp_type, &frame_info, milli_sec);
    if (ret != TD_SUCCESS) {
        printf("get pipe frame failed!\n");
        return TD_FAILURE;
    }

    if (frame_info.video_frame.compress_mode == OT_COMPRESS_MODE_NONE) {
        aiisp_save_one_frame(&frame_info.video_frame, byte_align, fd);
    }

    ret = aiisp_release_input_frame(pipe, aiisp_type, &frame_info);
    if (ret != TD_SUCCESS) {
        printf("release pipe frame failed!\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 aiisp_pipe_save_dump_output_frame(td_s32 pipe, ot_aiisp_type aiisp_type, td_u32 byte_align,
    FILE *fd)
{
    td_s32 ret;
    ot_video_frame_info frame_info;
    td_s32 milli_sec = -1;

    ret = aiisp_get_output_frame(pipe, aiisp_type, &frame_info, milli_sec);
    if (ret != TD_SUCCESS) {
        printf("get pipe frame failed!\n");
        return TD_FAILURE;
    }

    if (frame_info.video_frame.compress_mode == OT_COMPRESS_MODE_NONE) {
        aiisp_save_one_frame(&frame_info.video_frame, byte_align, fd);
    }

    ret = aiisp_release_output_frame(pipe, aiisp_type, &frame_info);
    if (ret != TD_SUCCESS) {
        printf("release pipe frame failed!\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void aiisp_release_fp(FILE *fp)
{
    (td_void)fflush(fp);
    (td_void)fclose(fp);
}

static td_s32 aiisp_pipe_save_dump_frame(td_s32 pipe, ot_aiisp_type aiisp_type, td_s32 mode,
    td_u32 frame_cnt, td_u32 byte_align)
{
    td_s32 ret;
    td_u32 dump_cnt = 0;
    FILE *input_fd = TD_NULL;
    FILE *output_fd = TD_NULL;
    td_bool dump_input = (mode == DUMP_INPUT || mode == DUMP_INOUT) ? TD_TRUE : TD_FALSE;
    td_bool dump_output = (mode == DUMP_OUTPUT || mode == DUMP_INOUT) ? TD_TRUE : TD_FALSE;

    if (dump_input == TD_TRUE) {
        input_fd = fopen(g_input_file_name, "wb");
        if (input_fd == TD_NULL) {
            printf("open input_file_name file failed, errno %d!\n", errno);
            return TD_FAILURE;
        }
    }

    if (dump_output == TD_TRUE) {
        output_fd = fopen(g_output_file_name, "wb");
        if (output_fd == TD_NULL) {
            aiisp_release_fp(input_fd);
            printf("open output_file_name file failed, errno %d!\n", errno);
            return TD_FAILURE;
        }
    }

    while ((dump_cnt < frame_cnt) && (g_signal_flag == 0)) {
        if (dump_input == TD_TRUE) {
            ret = aiisp_pipe_save_dump_input_frame(pipe, aiisp_type, byte_align, input_fd);
            if (ret != TD_SUCCESS) {
                printf("aiisp_pipe_save_dump_input_frame failed!\n");
                goto exit;
            }
        }

        if (dump_output == TD_TRUE) {
            ret = aiisp_pipe_save_dump_output_frame(pipe, aiisp_type, byte_align, output_fd);
            if (ret != TD_SUCCESS) {
                printf("aiisp_pipe_save_dump_output_frame failed!\n");
                goto exit;
            }
        }

        dump_cnt++;
    }

exit:
    if (dump_input == TD_TRUE) {
        aiisp_release_fp(input_fd);
    }

    if (dump_output == TD_TRUE) {
        aiisp_release_fp(output_fd);
    }

    return ret;
}

#ifdef OT_AIBNR_SUPPORT
static td_void aiisp_aibnr_do_pipe_dump_frame(td_s32 pipe, td_s32 mode, td_u32 frame_cnt, td_u32 byte_align)
{
    ot_aibnr_attr dump_attr, backup_dump_attr;

    if (ss_mpi_aibnr_get_attr(pipe, &backup_dump_attr) != TD_SUCCESS) {
        printf("get pipe dump frame attr failed!\n");
        return;
    }

    (td_void)memcpy_s(&dump_attr, sizeof(ot_aibnr_attr), &backup_dump_attr, sizeof(ot_aibnr_attr));

    dump_attr.input_depth  = (mode == DUMP_INPUT || mode == DUMP_INOUT) ? DUMP_FRAME_DEPTH : 0;
    dump_attr.output_depth  = (mode == DUMP_OUTPUT || mode == DUMP_INOUT) ? DUMP_FRAME_DEPTH : 0;
    if (ss_mpi_aibnr_set_attr(pipe, &dump_attr) != TD_SUCCESS) {
        printf("set pipe dump frame attr failed!\n");
        goto exit0;
    }

    if (aiisp_pipe_dump_create_file_frame(pipe, OT_AIISP_TYPE_AIBNR, mode, frame_cnt, byte_align) != TD_SUCCESS) {
        goto exit0;
    }

    if (aiisp_pipe_save_dump_frame(pipe, OT_AIISP_TYPE_AIBNR, mode, frame_cnt, byte_align) != TD_SUCCESS) {
        goto exit0;
    }

exit0:
    if (ss_mpi_aibnr_set_attr(pipe, &backup_dump_attr) != TD_SUCCESS) {
        printf("set pipe backup dump frame attr failed!\n");
    }
}
#endif

#ifdef OT_AIDRC_SUPPORT
static td_void aiisp_aidrc_save_out_file(const ot_video_frame *vframe, ot_aiisp_mem_info *out_data, FILE *fd)
{
    if (out_data->phys_addr == 0 || out_data->size == 0) {
        return;
    }

    out_data->virt_addr = (td_void *)ss_mpi_sys_mmap_cached(out_data->phys_addr, out_data->size);
    if (out_data->virt_addr == TD_NULL) {
        printf("out_data->phys_addr is null\n");
        return;
    }

    /* save detail */
    (td_void)fwrite(out_data->virt_addr, out_data->size, 1, fd);
    (td_void)fflush(fd);

    printf("saving......aidrc output......time_ref: %u!\n", vframe->time_ref);

    ss_mpi_sys_munmap(out_data->virt_addr, out_data->size);
}

static td_s32 aiisp_aidrc_save_dump_output_frame(td_s32 pipe, td_u32 byte_align,
    FILE *frame_fd, FILE *out_fd[], td_u32 out_fd_num)
{
    td_s32 ret;
    td_s32 i;
    td_s32 milli_sec = -1;
    ot_video_frame_info frame_info;
    ot_aiisp_mem_info out_data[MAX_AIDRC_OUT];
    ot_aiisp_mem_info *out[MAX_AIDRC_OUT];

    out[0] = &out_data[0];
    out[1] = &out_data[1];
    ret = ss_mpi_aidrc_get_output_frame(pipe, &frame_info, out, MAX_AIDRC_OUT, milli_sec);
    if (ret != TD_SUCCESS) {
        printf("get ss_mpi_aidrc_get_output_frame failed!\n");
        return TD_FAILURE;
    }

    if (frame_info.video_frame.compress_mode == OT_COMPRESS_MODE_NONE) {
        aiisp_save_one_frame(&frame_info.video_frame, byte_align, frame_fd);
    }

    for (i = 0; i < MAX_AIDRC_OUT; i++) {
        aiisp_aidrc_save_out_file(&frame_info.video_frame, &out_data[i], out_fd[i]);
    }

    ret = aiisp_release_output_frame(pipe, OT_AIISP_TYPE_AIDRC, &frame_info);
    if (ret != TD_SUCCESS) {
        printf("release pipe frame failed!\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 aiisp_aidrc_save_dump_frame(td_s32 pipe, td_s32 mode,  td_u32 frame_cnt, td_u32 byte_align)
{
    td_s32 ret = TD_FAILURE;
    td_u32 dump_cnt = 0;
    FILE *input_fd = TD_NULL;
    FILE *output_fd = TD_NULL;
    FILE *aidrc_file[MAX_AIDRC_OUT] = {TD_NULL, TD_NULL};
    td_bool dump_output = (mode == DUMP_OUTPUT || mode == DUMP_INOUT) ? TD_TRUE : TD_FALSE;
    td_bool dump_input = (mode == DUMP_INPUT || mode == DUMP_INOUT) ? TD_TRUE : TD_FALSE;
    td_char path[PATH_MAX] = {0};

    if (dump_input == TD_TRUE) {
        if (realpath(g_input_file_name, path) == TD_NULL) {
            printf("g_input_file_name realpath err, invalid file path!.\n");
            return TD_FALSE;
        }
        input_fd = fopen(path, "wb");
        if (input_fd == TD_NULL) {
            printf("open file failed, errno %d!\n", errno);
            return TD_FAILURE;
        }
    }

    if (dump_output == TD_TRUE) {
        if (realpath(g_output_file_name, path) == TD_NULL) {
            printf("g_output_file_name realpath err, invalid file path!.\n");
            goto exit;
        }
        output_fd = fopen(path, "wb");
        if (output_fd == TD_NULL) {
            printf("open file failed, errno %d!\n", errno);
            goto exit;
        }
        if (realpath(g_output_aidrc_file_name[0], path) == TD_NULL) {
            printf("g_output_aidrc_file_name[0] realpath err, invalid file path!.\n");
            goto exit;
        }
        aidrc_file[0] = fopen(path, "wb");
        if (aidrc_file[0] == TD_NULL) {
            printf("open file failed, errno %d!\n", errno);
            goto exit;
        }
        if (realpath(g_output_aidrc_file_name[1], path) == TD_NULL) {
            printf("g_output_aidrc_file_name[0] realpath err, invalid file path!.\n");
            goto exit;
        }
        aidrc_file[1] = fopen(path, "wb");
        if (aidrc_file[1] == TD_NULL) {
            printf("open file failed, errno %d!\n", errno);
            goto exit;
        }
    }

    while ((dump_cnt < frame_cnt) && (g_signal_flag == 0)) {
        if (dump_input == TD_TRUE) {
            ret = aiisp_pipe_save_dump_input_frame(pipe, OT_AIISP_TYPE_AIDRC, byte_align, input_fd);
            if (ret != TD_SUCCESS) {
                printf("aiisp_pipe_save_dump_input_frame failed!\n");
                goto exit;
            }
        }

        if (dump_output == TD_TRUE) {
            ret = aiisp_aidrc_save_dump_output_frame(pipe, byte_align, output_fd, aidrc_file, MAX_AIDRC_OUT);
            if (ret != TD_SUCCESS) {
                printf("aiisp_pipe_save_dump_output_frame failed!\n");
                goto exit;
            }
        }

        dump_cnt++;
    }

exit:
    if (dump_input == TD_TRUE) {
        if (input_fd != TD_NULL) {
            (td_void)fflush(input_fd);
            (td_void)fclose(input_fd);
        }
    }
    if (dump_output == TD_TRUE) {
        if (output_fd != TD_NULL) {
            (td_void)fflush(output_fd);
            (td_void)fclose(output_fd);
        }

        if (aidrc_file[0] != TD_NULL) {
            (td_void)fflush(aidrc_file[0]);
            (td_void)fclose(aidrc_file[0]);
        }

        if (aidrc_file[1]!= TD_NULL) {
            (td_void)fflush(aidrc_file[1]);
            (td_void)fclose(aidrc_file[1]);
        }
    }

    return ret;
}

static td_void aiisp_aidrc_do_pipe_dump_frame(td_s32 pipe, td_s32 mode, td_u32 frame_cnt, td_u32 byte_align)
{
    ot_aidrc_attr dump_attr, backup_dump_attr;

    if (ss_mpi_aidrc_get_attr(pipe, &backup_dump_attr) != TD_SUCCESS) {
        printf("get pipe dump frame attr failed!\n");
        return;
    }

    (td_void)memcpy_s(&dump_attr, sizeof(ot_aidrc_attr), &backup_dump_attr, sizeof(ot_aidrc_attr));

    dump_attr.output_depth  = (mode == DUMP_OUTPUT || mode == DUMP_INOUT) ? DUMP_FRAME_DEPTH : 0;
    if (ss_mpi_aidrc_set_attr(pipe, &dump_attr) != TD_SUCCESS) {
        printf("set pipe dump frame attr failed!\n");
        goto exit0;
    }

    if (aiisp_pipe_dump_create_file_frame(pipe, OT_AIISP_TYPE_AIDRC, mode, frame_cnt, byte_align) != TD_SUCCESS) {
        goto exit0;
    }

    if (aiisp_aidrc_save_dump_frame(pipe,  mode, frame_cnt, byte_align) != TD_SUCCESS) {
        goto exit0;
    }

exit0:
    if (ss_mpi_aidrc_set_attr(pipe, &backup_dump_attr) != TD_SUCCESS) {
        printf("set pipe backup dump frame attr failed!\n");
    }
}
#endif
#ifdef OT_AI3DNR_SUPPORT
static td_void aiisp_ai3dnr_do_pipe_dump_frame(td_s32 pipe, td_s32 mode, td_u32 frame_cnt, td_u32 byte_align)
{
    ot_ai3dnr_attr dump_attr, backup_dump_attr;

    if (ss_mpi_ai3dnr_get_attr(pipe, &backup_dump_attr) != TD_SUCCESS) {
        printf("get pipe dump frame attr failed!\n");
        return;
    }

    (td_void)memcpy_s(&dump_attr, sizeof(ot_ai3dnr_attr), &backup_dump_attr, sizeof(ot_ai3dnr_attr));

    dump_attr.input_depth  = (mode == DUMP_INPUT || mode == DUMP_INOUT) ? DUMP_FRAME_DEPTH : 0;
    dump_attr.output_depth  = (mode == DUMP_OUTPUT || mode == DUMP_INOUT) ? DUMP_FRAME_DEPTH : 0;
    if (ss_mpi_ai3dnr_set_attr(pipe, &dump_attr) != TD_SUCCESS) {
        printf("set pipe dump frame attr failed!\n");
        goto exit0;
    }

    if (aiisp_pipe_dump_create_file_frame(pipe, OT_AIISP_TYPE_AI3DNR, mode, frame_cnt, byte_align) != TD_SUCCESS) {
        goto exit0;
    }

    if (aiisp_pipe_save_dump_frame(pipe, OT_AIISP_TYPE_AI3DNR, mode, frame_cnt, byte_align) != TD_SUCCESS) {
        goto exit0;
    }

exit0:
    if (ss_mpi_ai3dnr_set_attr(pipe, &backup_dump_attr) != TD_SUCCESS) {
        printf("set pipe backup dump frame attr failed!\n");
    }
}
#endif

static td_void aiisp_do_pipe_dump_frame(td_s32 pipe, ot_aiisp_type aiisp_type, td_s32 mode,
    td_u32 frame_cnt, td_u32 byte_align)
{
    switch (aiisp_type) {
#ifdef OT_AIBNR_SUPPORT
        case OT_AIISP_TYPE_AIBNR:
            aiisp_aibnr_do_pipe_dump_frame(pipe, mode, frame_cnt, byte_align);
            break;
#endif
#ifdef OT_AIDRC_SUPPORT
        case OT_AIISP_TYPE_AIDRC:
            aiisp_aidrc_do_pipe_dump_frame(pipe, mode, frame_cnt, byte_align);
            break;
#endif
#ifdef OT_AI3DNR_SUPPORT
        case OT_AIISP_TYPE_AI3DNR:
            aiisp_ai3dnr_do_pipe_dump_frame(pipe, mode, frame_cnt, byte_align);
            break;
#endif

        default:
            printf("unsupport type:%d\n", aiisp_type);
    }

    return;
}

static td_void aiisp_pipe_dump_handle_sig(td_s32 signo)
{
    if (g_signal_flag) {
        return;
    }

    if (signo == SIGINT || signo == SIGTERM) {
        g_signal_flag = 1;
    }
}

static td_void tool_usage(td_void)
{
    printf(
        "\n"
        "*************************************************\n"
        "Usage: ./aiisp_dump [ot_aiisp_pipe] [aiisp_type] [mode] [frame_cnt]\n"
        "ot_aiisp_pipe: \n"
        "   aiisp pipe id\n"
        "aiisp_type: \n"
        "   aiisp alg type\n"
        "   0:aibnr\n"
        "   1:aidrc\n"
        "   2:ai3dnr\n"
        "mode: \n"
        "   0:input\n"
        "   1:output\n"
        "   2:input and output\n"
        "frame_cnt: \n"
        "   the count of frame to be dump\n"
        "e.g : ./aiisp_dump 0 1 0 1\n"
        "*************************************************\n"
        "\n");
}

static const td_char *aiisp_pipe_dump_get_argv_name(td_s32 index)
{
    const td_char *argv_name[3] = {"ot_aiisp_pipe", "frame_cnt", "byte_align"}; /* 3: arg nums */

    if (index >= 3 || index < 0) { /* 3: arg nums */
        return "-";
    }

    return argv_name[index];
}

static td_s32 aiisp_pipe_dump_get_argv_val(char *argv[], td_s32 index, td_s32 min_val, td_s32 max_val, td_s32 *val)
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
            index, aiisp_pipe_dump_get_argv_name(index - 1), result, min_val, max_val);
        return TD_FAILURE;
    }

    *val = result;
    return TD_SUCCESS;
}

#ifdef __LITEOS__
td_s32 aiisp_dump(int argc, char *argv[])
#else
td_s32 main(int argc, char *argv[])
#endif
{
    td_s32 frame_cnt = 0;
    td_s32 byte_align = 1;
    td_s32 aiisp_type = OT_AIISP_TYPE_BUTT;
    td_s32 mode = 0;
    td_s32 type = 0;

    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("\tTo see more usage, please enter: ./aiisp_dump -h\n\n");

    if ((argc > 1) && !strncmp(argv[1], "-h", 2)) { /* 2 help */
        tool_usage();
        return TD_SUCCESS;
    }

    if (argc != 5) { /* 5: arg num */
        tool_usage();
        return TD_FAILURE;
    }

    if (aiisp_pipe_dump_get_argv_val(argv, 1, 0, OT_VI_MAX_PIPE_NUM - 1, &g_aiisp_pipe) != TD_SUCCESS) { /* arg 1 */
        tool_usage();
        return TD_FAILURE;
    }

    if (aiisp_pipe_dump_get_argv_val(argv, 2, 0, MAX_AIISP_TYPE, &type) != TD_SUCCESS) { /* arg 2 */
        tool_usage();
        return TD_FAILURE;
    }

    if (aiisp_pipe_dump_get_argv_val(argv, 3, 0, MAX_AIISP_DUMP_MODE, &mode) != TD_SUCCESS) { /* arg 3 */
        tool_usage();
        return TD_FAILURE;
    }

    if (aiisp_pipe_dump_get_argv_val(argv, 4, 0, MAX_DUMP_FRAME_CNT, &frame_cnt) != TD_SUCCESS) { /* arg 4 */
        tool_usage();
        return TD_FAILURE;
    }

    g_signal_flag = 0;

#ifndef __LITEOS__
    (td_void)signal(SIGINT, aiisp_pipe_dump_handle_sig);
    (td_void)signal(SIGTERM, aiisp_pipe_dump_handle_sig);
#endif

    aiisp_type = (type == 0) ? OT_AIISP_TYPE_AIBNR : (type == 1) ? OT_AIISP_TYPE_AIDRC : OT_AIISP_TYPE_AI3DNR;

    aiisp_do_pipe_dump_frame(g_aiisp_pipe, (ot_aiisp_type)aiisp_type, mode, frame_cnt, byte_align);

    return TD_SUCCESS;
}
