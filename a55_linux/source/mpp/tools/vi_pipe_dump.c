/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <signal.h>
#include "ss_mpi_sys_mem.h"
#include "ss_mpi_vi.h"
#include "securec.h"

#define MAX_DUMP_FRAME_CNT      64
#define DUMP_FRAME_DEPTH        2
#define MAX_FRM_WIDTH           8192
#define FILE_NAME_LENGTH        128
#define PIXEL_FORMAT_STRING_LEN 10

static volatile sig_atomic_t g_signal_flag = 0;

static ot_vi_pipe g_vi_pipe = 0;
static td_char g_file_name[FILE_NAME_LENGTH];
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
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_L10:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_L12:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_L14:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_H10:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_H12:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_H14:
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
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_L10:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_L12:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_L14:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_H10:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_H12:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_H14:
            return "raw";
        default:
            return "na";
    }
}

static td_s32 vi_make_pipe_frame_file_name(ot_vi_pipe vi_pipe, const ot_video_frame_info *frame_info,
                                           td_u32 frame_cnt, td_u32 byte_align)
{
    /* make file name */
    if (snprintf_s(g_file_name, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./vi_pipe%d_%ux%u_%s_%u_%u.%s",
        vi_pipe, frame_info->video_frame.width, frame_info->video_frame.height,
        get_pixel_format_str(frame_info->video_frame.pixel_format),
        frame_cnt,
        byte_align,
        get_file_suffix_name(frame_info->video_frame.pixel_format)) == -1) {
        printf("set output file name failed!\n");
        return TD_FAILURE;
    }

    printf("Dump frame of vi pipe %d to file: \"%s\"\n", vi_pipe, g_file_name);

    return TD_SUCCESS;
}

static td_s32 vi_pipe_dump_create_file_frame(ot_vi_pipe vi_pipe, td_u32 frame_cnt, td_u32 byte_align)
{
    td_s32 ret;
    ot_video_frame_info frame_info;
    td_s32 milli_sec = 3000; /* 3s */

    ret = ss_mpi_vi_get_pipe_frame(vi_pipe, &frame_info, milli_sec);
    if (ret != TD_SUCCESS) {
        printf("get pipe frame failed!\n");
        return TD_FAILURE;
    }

    ret = vi_make_pipe_frame_file_name(vi_pipe, &frame_info, frame_cnt, byte_align);

    if (ss_mpi_vi_release_pipe_frame(vi_pipe, &frame_info) != TD_SUCCESS) {
        printf("release pipe frame failed!\n");
        return TD_FAILURE;
    }

    return ret;
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

static td_void vi_save_8bit_yuv_file(const ot_video_frame *frame, FILE *pfd)
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
        vi_chn_dump_covert_chroma_sp42x_to_planar(frame, pfd, uv_height, is_uv_invert);
    }

    (td_void)fprintf(stderr, "done %u!\n", frame->time_ref);
    (td_void)fflush(stderr);
    ss_mpi_sys_munmap(g_user_page_addr[0], g_size);
    g_user_page_addr[0] = NULL;
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
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_L10:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_L12:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_L14:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_H10:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_H12:
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP_H14:
            bit_width = 16; /* 16: 16bit */
            break;
        default:
            bit_width = 8;  /* 8: 8bit */
            break;
    }

    return bit_width;
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

static td_void vi_save_one_frame(const ot_video_frame *video_frame, td_u32 byte_align, FILE *pfd)
{
    if ((video_frame->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422) ||
        (video_frame->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) ||
        (video_frame->pixel_format == OT_PIXEL_FORMAT_YUV_400)) {
        vi_save_8bit_yuv_file(video_frame, pfd);
    } else if ((video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_8BPP) ||
                (video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_10BPP) ||
                (video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_12BPP) ||
                (video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_14BPP) ||
                (video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_16BPP_L10) ||
                (video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_16BPP_L12) ||
                (video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_16BPP_L14) ||
                (video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_16BPP_H10) ||
                (video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_16BPP_H12) ||
                (video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_16BPP_H14) ||
                (video_frame->pixel_format == OT_PIXEL_FORMAT_RGB_BAYER_16BPP)) {
        vi_save_raw_file(video_frame, byte_align, pfd);
    } else {
        printf("unsupported pixel format (%d)!\n", video_frame->pixel_format);
    }
}

static td_s32 vi_pipe_save_dump_frame(ot_vi_pipe vi_pipe, td_u32 frame_cnt, td_u32 byte_align)
{
    td_s32 ret;
    td_u32 dump_cnt = 0;
    FILE *pfd = TD_NULL;
    ot_video_frame_info frame_info;
    td_s32 milli_sec = -1;

    pfd = fopen(g_file_name, "wb");
    if (pfd == TD_NULL) {
        printf("open file failed, errno %d!\n", errno);
        return TD_FAILURE;
    }

    while ((dump_cnt < frame_cnt) && (g_signal_flag == 0)) {
        ret = ss_mpi_vi_get_pipe_frame(vi_pipe, &frame_info, milli_sec);
        if (ret != TD_SUCCESS) {
            printf("get pipe frame failed!\n");
            goto exit;
        }

        if (frame_info.video_frame.compress_mode == OT_COMPRESS_MODE_NONE) {
            vi_save_one_frame(&frame_info.video_frame, byte_align, pfd);
            dump_cnt++;
        }

        ret = ss_mpi_vi_release_pipe_frame(vi_pipe, &frame_info);
        if (ret != TD_SUCCESS) {
            printf("release pipe frame failed!\n");
            goto exit;
        }
    }

exit:
    (td_void)fflush(pfd);
    (td_void)fclose(pfd);
    return ret;
}

static td_void vi_do_pipe_dump_frame(ot_vi_pipe vi_pipe, td_u32 frame_cnt, td_u32 byte_align)
{
    ot_vi_frame_dump_attr dump_attr, backup_dump_attr;
    ot_vi_pipe_attr pipe_attr, backup_pipe_attr;

    if (ss_mpi_vi_get_pipe_frame_dump_attr(vi_pipe, &backup_dump_attr) != TD_SUCCESS) {
        printf("get pipe dump frame attr failed!\n");
        return;
    }

    if (ss_mpi_vi_get_pipe_attr(vi_pipe, &backup_pipe_attr) != TD_SUCCESS) {
        printf("get pipe attr failed!\n");
        goto exit0;
    }

    (td_void)memcpy_s(&pipe_attr, sizeof(ot_vi_pipe_attr), &backup_pipe_attr, sizeof(ot_vi_pipe_attr));
    pipe_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    if (ss_mpi_vi_set_pipe_attr(vi_pipe, &pipe_attr) != TD_SUCCESS) {
        printf("set pipe attr failed!\n");
        goto exit1;
    }

    dump_attr.enable = TD_TRUE;
    dump_attr.depth  = DUMP_FRAME_DEPTH;
    if (ss_mpi_vi_set_pipe_frame_dump_attr(vi_pipe, &dump_attr) != TD_SUCCESS) {
        printf("set pipe dump frame attr failed!\n");
        goto exit1;
    }

    if (vi_pipe_dump_create_file_frame(vi_pipe, frame_cnt, byte_align) != TD_SUCCESS) {
        goto exit1;
    }

    if (vi_pipe_save_dump_frame(vi_pipe, frame_cnt, byte_align) != TD_SUCCESS) {
        goto exit1;
    }

exit1:
    if (ss_mpi_vi_set_pipe_attr(vi_pipe, &backup_pipe_attr) != TD_SUCCESS) {
        printf("set pipe backup attr failed!\n");
    }
exit0:
    if (ss_mpi_vi_set_pipe_frame_dump_attr(vi_pipe, &backup_dump_attr) != TD_SUCCESS) {
        printf("set pipe backup dump frame attr failed!\n");
    }
}

static td_void vi_pipe_dump_handle_sig(td_s32 signo)
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
        "Usage: ./vi_pipe_dump [ot_vi_pipe] [frame_cnt] [byte_align]\n"
        "ot_vi_pipe: \n"
        "   vi pipe id\n"
        "frame_cnt: \n"
        "   the count of frame to be dump\n"
        "byte_align: \n"
        "   whether convert to byte align, default is 1\n"
        "e.g : ./vi_pipe_dump 0 1\n"
        "e.g : ./vi_pipe_dump 0 2 0\n"
        "*************************************************\n"
        "\n");
}

static const td_char *vi_pipe_dump_get_argv_name(td_s32 index)
{
    const td_char *argv_name[3] = {"ot_vi_pipe", "frame_cnt", "byte_align"}; /* 3: arg nums */

    if (index >= 3 || index < 0) { /* 3: arg nums */
        return "-";
    }

    return argv_name[index];
}

static td_s32 vi_pipe_dump_get_argv_val(char *argv[], td_s32 index, td_s32 min_val, td_s32 max_val, td_s32 *val)
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
            index, vi_pipe_dump_get_argv_name(index - 1), result, min_val, max_val);
        return TD_FAILURE;
    }

    *val = result;
    return TD_SUCCESS;
}

#ifdef __LITEOS__
td_s32 vi_pipe_dump(int argc, char *argv[])
#else
td_s32 main(int argc, char *argv[])
#endif
{
    td_s32 frame_cnt;
    td_s32 byte_align = 1;

    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("\tTo see more usage, please enter: ./vi_pipe_dump -h\n\n");

    if ((argc > 1) && !strncmp(argv[1], "-h", 2)) { /* 2 help */
        tool_usage();
        return TD_SUCCESS;
    }

    if (argc < 3) { /* 3: 3 arg num */
        tool_usage();
        return TD_FAILURE;
    }

    if (vi_pipe_dump_get_argv_val(argv, 1, 0, OT_VI_MAX_PIPE_NUM - 1, &g_vi_pipe) != TD_SUCCESS) { /* arg 1 */
        tool_usage();
        return TD_FAILURE;
    }

    if (vi_pipe_dump_get_argv_val(argv, 2, 0, MAX_DUMP_FRAME_CNT, &frame_cnt) != TD_SUCCESS) { /* arg 2 */
        tool_usage();
        return TD_FAILURE;
    }

    if (argc > 3) { /* 3: 3 arg num */
        if (vi_pipe_dump_get_argv_val(argv, 3, 0, 1, &byte_align) != TD_SUCCESS) { /* arg 3 */
            tool_usage();
            return TD_FAILURE;
        }
    }

    g_signal_flag = 0;

#ifndef __LITEOS__
    (td_void)signal(SIGINT, vi_pipe_dump_handle_sig);
    (td_void)signal(SIGTERM, vi_pipe_dump_handle_sig);
#endif

    vi_do_pipe_dump_frame(g_vi_pipe, frame_cnt, byte_align);

    return TD_SUCCESS;
}
