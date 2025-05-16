/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <signal.h>
#include <limits.h>
#include "securec.h"
#include "ss_mpi_sys_mem.h"
#include "ss_mpi_vb.h"
#include "ss_mpi_vo.h"
#include "ss_mpi_vgs.h"
#include "ot_buffer.h"

#define FILE_NAME_LENGTH 128
#define MAX_FRM_CNT 64
#define MAX_FRM_WIDTH 8192
#define MAX_FORMAT_LENGTH 10
#define VO_DUMP_ARG_NUMBER_BASE 10  /* 10: decimal scale */

typedef struct {
    ot_vb_blk h_block;
    ot_vb_pool h_pool;
    td_u32 pool_id;

    td_phys_addr_t phy_addr;
    td_phys_addr_t vir_addr;
    td_s32 mdev;
} dump_membuf;

static ot_video_frame_info g_frame = {0};
static char *g_v_buf_virt_y = TD_NULL;
static char *g_v_buf_virt_c = TD_NULL;
static td_u32 g_ysize, g_csize;
static FILE *g_pfd = TD_NULL;
static ot_vb_pool g_h_pool = OT_VB_INVALID_POOL_ID;
static dump_membuf g_mem;
static ot_vo_layer g_vo_layer_id = 0;
static ot_vo_chn g_vo_chn_id = 0;
static volatile sig_atomic_t g_signal_flag = 0;

typedef enum {
    VO_DUMP_ARG_CMD      = 0,   /* dump arg cmd */
    VO_DUMP_ARG_VO_LAYER = 1,   /* dump arg vo_layer */
    VO_DUMP_ARG_VO_CHN   = 2,   /* dump arg vo_chn */
    VO_DUMP_ARG_FRM_CNT  = 3,   /* dump arg frm_cnt */
    VO_DUMP_ARG_BUTT,
} vo_dump_arg_index;

static void usage(void)
{
    printf("\n"
           "*************************************************\n"
#ifndef __LITEOS__
           "Usage: ./vo_chn_dump [vo_layer] [vo_chn] [frm_cnt].\n"
#else
           "Usage: vo_chn_dump [vo_layer] [vo_chn] [frm_cnt].\n"
#endif
           "1)vo_layer: \n"
           "   which layer to be dump\n"
           "   default: 0\n"
           "2)vo_chn: \n"
           "   which channel to be dump\n"
           "   default: 0\n"
           "3)frm_cnt: \n"
           "   the count of frame to be dumped\n"
           "   default: 1\n"
           "*)example:\n"
#ifndef __LITEOS__
           "   e.g : ./vo_chn_dump 0 0 1 (dump one yuv)\n"
#else
           "   e.g : vo_chn_dump 0 0 1 (dump one yuv)\n"
#endif
           "*************************************************\n"
           "\n");
}

#ifndef __LITEOS__
static void vo_chn_handle_sig(td_s32 signo)
{
    if (g_signal_flag == 1) {
        return;
    }

    if ((signo == SIGINT) || (signo == SIGTERM)) {
        g_signal_flag = 1;
    }
}
#endif

static td_void sample_cal_yc_size(const ot_video_frame *v_buf)
{
    ot_pixel_format pixel_format = v_buf->pixel_format;

    g_ysize = (v_buf->stride[0]) * (v_buf->height);
    if ((pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) ||
        (pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420)) {
        g_csize = (v_buf->stride[1]) * (v_buf->height) / 2; /* 1 / 2 */
    } else if ((pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422) ||
        (pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422)) {
        g_csize = (v_buf->stride[1]) * (v_buf->height);
    } else {
        g_csize = 0;
    }
}

static td_void sample_get_uv_height(const ot_video_frame *v_buf, td_u32 *uv_height)
{
    ot_pixel_format pixel_format = v_buf->pixel_format;

    if ((pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) ||
        (pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420)) {
        *uv_height = v_buf->height / 2; /* 1 / 2 */
    } else if ((pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422) ||
        (pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422)) {
        *uv_height = v_buf->height;
    } else {
        *uv_height = 0;
    }
}

static td_void sample_save_y(const ot_video_frame *v_buf)
{
    unsigned int h;
    char *mem_content = TD_NULL;

    /* save Y */
    (void)fprintf(stderr, "saving......Y......");
    (void)fflush(stderr);
    for (h = 0; h < v_buf->height; h++) {
        mem_content = g_v_buf_virt_y + h * v_buf->stride[0];
        (void)fwrite(mem_content, v_buf->width, 1, g_pfd);
    }
    if (g_pfd != TD_NULL) {
        (void)fflush(g_pfd);
    }
}

static td_void sample_save_chroma(const ot_video_frame *v_buf, unsigned int data_offset)
{
    unsigned int w, h;
    char *mem_content = TD_NULL;
    static unsigned char tmp_buff[MAX_FRM_WIDTH];
    td_u32 uv_height = 0;

    sample_get_uv_height(v_buf, &uv_height);

    (void)fflush(stderr);
    for (h = 0; h < uv_height; h++) {
        mem_content = g_v_buf_virt_c + h * v_buf->stride[1];
        mem_content += data_offset;
        for (w = 0; w < v_buf->width / 2; w++) { /* 1 / 2 */
            tmp_buff[w] = *mem_content;
            mem_content += 2; /* 2 bytes */
        }
        (void)fwrite(tmp_buff, v_buf->width / 2, 1, g_pfd); /* 1 / 2 */
    }
    (void)fflush(g_pfd);
}

static td_void sample_save_uv(const ot_video_frame *v_buf)
{
    ot_pixel_format pixel_format = v_buf->pixel_format;
    if ((pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) ||
        (pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422)) {
        /* save U */
        (void)fprintf(stderr, "U......");
        sample_save_chroma(v_buf, 1);
        /* save V */
        (void)fprintf(stderr, "V......");
        sample_save_chroma(v_buf, 0);
    } else if ((pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420) ||
        (pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422)) {
        /* save U */
        (void)fprintf(stderr, "U......");
        sample_save_chroma(v_buf, 0);
        /* save V */
        (void)fprintf(stderr, "V......");
        sample_save_chroma(v_buf, 1);
    }
}

static td_void sample_yuv_dump(const ot_video_frame *v_buf)
{
    td_phys_addr_t phys_addr;
    ot_pixel_format pixel_format = v_buf->pixel_format;

    sample_cal_yc_size(v_buf);

    phys_addr = v_buf->phys_addr[0];
    g_v_buf_virt_y = (td_char *)ss_mpi_sys_mmap(phys_addr, g_ysize);
    if (g_v_buf_virt_y == TD_NULL) {
        return;
    }

    sample_save_y(v_buf);

    ss_mpi_sys_munmap(g_v_buf_virt_y, g_ysize);
    g_v_buf_virt_y = TD_NULL;

    if (pixel_format != OT_PIXEL_FORMAT_YUV_400) {
        g_v_buf_virt_c = (td_char *)ss_mpi_sys_mmap(v_buf->phys_addr[1], g_csize);
        if (g_v_buf_virt_c == TD_NULL) {
            return;
        }
        sample_save_uv(v_buf);
        ss_mpi_sys_munmap(g_v_buf_virt_c, g_csize);
        g_v_buf_virt_c = TD_NULL;
    }
    (void)fprintf(stderr, "done %u!\n", v_buf->time_ref);
    (void)fflush(stderr);
}

static td_void sample_get_cal_config(ot_vb_calc_cfg *cal_config)
{
    td_u32 align;
    ot_pixel_format pixel_format;
    ot_data_bit_width bit_width;
    ot_compress_mode cmp_mode;
    ot_pic_buf_attr buf_attr;
    td_u32 width;
    td_u32 height;

    width = g_frame.video_frame.width;
    height = g_frame.video_frame.height;
    align = 0;
    cmp_mode = OT_COMPRESS_MODE_NONE;
    pixel_format = g_frame.video_frame.pixel_format;
    bit_width = (g_frame.video_frame.dynamic_range == OT_DYNAMIC_RANGE_SDR8) ?
        OT_DATA_BIT_WIDTH_8 : OT_DATA_BIT_WIDTH_10;

    (td_void)memset_s(cal_config, sizeof(ot_vb_calc_cfg), 0, sizeof(ot_vb_calc_cfg));

    buf_attr.width = width;
    buf_attr.height = height;
    buf_attr.pixel_format = pixel_format;
    buf_attr.bit_width = bit_width;
    buf_attr.compress_mode = cmp_mode;
    buf_attr.align = align;
    buf_attr.video_format = g_frame.video_frame.video_format;
    ot_common_get_pic_buf_cfg(&buf_attr, cal_config);
}

static td_bool sample_create_vb_pool(td_void)
{
    ot_vb_calc_cfg cal_config;
    ot_vb_pool_cfg vb_pool_cfg;
    td_u32 blk_size;

    sample_get_cal_config(&cal_config);

    blk_size = cal_config.vb_size;

    (td_void)memset_s(&vb_pool_cfg, sizeof(ot_vb_pool_cfg), 0, sizeof(ot_vb_pool_cfg));
    vb_pool_cfg.blk_size = blk_size;
    vb_pool_cfg.blk_cnt = 1;
    vb_pool_cfg.remap_mode = OT_VB_REMAP_MODE_NONE;
    g_h_pool = ss_mpi_vb_create_pool(&vb_pool_cfg);
    if (g_h_pool == OT_VB_INVALID_POOL_ID) {
        printf("ss_mpi_vb_create_pool blk_size = %u failed! \n", blk_size);
        return TD_FAILURE;
    }

    g_mem.h_pool = g_h_pool;
    return TD_SUCCESS;
}

static td_void sample_set_frame_info(ot_video_frame_info *frm_info, td_s32 i)
{
    td_u32 width;
    td_u32 height;
    ot_vb_calc_cfg cal_config;

    sample_get_cal_config(&cal_config);

    width = g_frame.video_frame.width;
    height = g_frame.video_frame.height;

    (td_void)memset_s(&frm_info->video_frame, sizeof(ot_video_frame), 0, sizeof(ot_video_frame));
    frm_info->video_frame.width = width;
    frm_info->video_frame.height = height;
    frm_info->video_frame.dynamic_range = g_frame.video_frame.dynamic_range;
    frm_info->video_frame.field = OT_VIDEO_FIELD_FRAME;
    frm_info->video_frame.phys_addr[0] = g_mem.phy_addr;
    frm_info->video_frame.phys_addr[1] = frm_info->video_frame.phys_addr[0] + cal_config.main_y_size;

    frm_info->video_frame.stride[0] = cal_config.main_stride;
    frm_info->video_frame.stride[1] = cal_config.main_stride;

    frm_info->video_frame.compress_mode = OT_COMPRESS_MODE_NONE;
    frm_info->video_frame.pixel_format = g_frame.video_frame.pixel_format;
    frm_info->video_frame.video_format = OT_VIDEO_FORMAT_LINEAR;

    frm_info->video_frame.pts = (i * 40);     /* make time ref i * 40 */
    frm_info->video_frame.time_ref = (i * 2); /* make time ref i * 2  */

    frm_info->mod_id = OT_ID_VGS;
    frm_info->pool_id = g_h_pool;
}

static td_bool sample_is_send_to_vgs(ot_video_frame_info *frm_info)
{
    if ((frm_info->video_frame.compress_mode > 0) || (frm_info->video_frame.video_format > 0)) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

static td_s32 sample_vgs_process(const ot_video_frame_info *frm_info)
{
    ot_vgs_handle h_handle = -1;
    ot_vgs_task_attr task;
    td_s32 ret;

    ret = ss_mpi_vgs_begin_job(&h_handle);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_vgs_begin_job failed\n");
        ss_mpi_vb_release_blk(g_mem.h_block);
        g_mem.h_block = OT_VB_INVALID_HANDLE;
        return TD_FAILURE;
    }

    (td_void)memcpy_s(&task.img_in, sizeof(ot_video_frame_info), &g_frame, sizeof(ot_video_frame_info));
    (td_void)memcpy_s(&task.img_out, sizeof(ot_video_frame_info), frm_info, sizeof(ot_video_frame_info));
    ret = ss_mpi_vgs_add_scale_task(h_handle, &task, OT_VGS_SCALE_COEF_NORM);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_vgs_add_scale_task failed\n");
        ss_mpi_vgs_cancel_job(h_handle);
        ss_mpi_vb_release_blk(g_mem.h_block);
        g_mem.h_block = OT_VB_INVALID_HANDLE;
        return TD_FAILURE;
    }

    ret = ss_mpi_vgs_end_job(h_handle);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_vgs_end_job failed\n");
        ss_mpi_vgs_cancel_job(h_handle);
        ss_mpi_vb_release_blk(g_mem.h_block);
        g_mem.h_block = OT_VB_INVALID_HANDLE;
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void sample_vo_clear_frame(td_void)
{
    (td_void)memset_s(&g_frame, sizeof(ot_video_frame_info), 0, sizeof(ot_video_frame_info));
}

static td_s32 sample_make_yuv_file_name(ot_vo_layer vo_layer, td_s32 cnt)
{
    td_s32 ret;
    td_char sz_pix_frm[MAX_FORMAT_LENGTH] = {0};
    size_t sz_pix_len;
    td_char sz_yuv_name[FILE_NAME_LENGTH] = {0};

    if (g_signal_flag == 1) {
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
        return TD_FAILURE;
    }
    /* get frame to make file name */
    ret = ss_mpi_vo_get_chn_frame(vo_layer, g_vo_chn_id, &g_frame, 0);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_vo(%d)_get_chn_frame errno %#x\n", vo_layer, ret);
        return TD_FAILURE;
    }
    ret = ss_mpi_vo_release_chn_frame(vo_layer, g_vo_chn_id, &g_frame);
    if (ret != TD_SUCCESS) {
        printf("vo_chn(%d,%d) ss_mpi_vo_release_chn_frame errno %#x\n", vo_layer, g_vo_chn_id, ret);
        return TD_FAILURE;
    }
    /* make file name */
    sz_pix_len = sizeof(sz_pix_frm) / sizeof(td_char);
    if ((g_frame.video_frame.pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) ||
        (g_frame.video_frame.pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420)) {
        ret = snprintf_s(sz_pix_frm, sz_pix_len, sz_pix_len - 1, "p420");
    } else if ((g_frame.video_frame.pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422) ||
        (g_frame.video_frame.pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422)) {
        ret = snprintf_s(sz_pix_frm, sz_pix_len, sz_pix_len - 1, "p422");
    } else if (g_frame.video_frame.pixel_format == OT_PIXEL_FORMAT_YUV_400) {
        ret = snprintf_s(sz_pix_frm, sz_pix_len, sz_pix_len - 1, "p400");
    } else {
        printf("pixel_format %d is not support\n", g_frame.video_frame.pixel_format);
        return TD_FAILURE;
    }

    if (ret < 0) {
        printf("get frame pixel format name fail!\n");
        return TD_FAILURE;
    }

    ret = snprintf_s(sz_yuv_name, FILE_NAME_LENGTH, FILE_NAME_LENGTH - 1, "./vo_layer%d_chn%d_%ux%u_%s_%d.yuv",
        vo_layer, g_vo_chn_id, g_frame.video_frame.width, g_frame.video_frame.height, sz_pix_frm, cnt);
    if (ret < 0) {
        printf("get yuv file name fail!\n");
        return TD_FAILURE;
    }

    printf("dump YUV frame of vo(%d,%d) to file: \"%s\"\n", vo_layer, g_vo_chn_id, sz_yuv_name);

    /* open file */
    g_pfd = fopen(sz_yuv_name, "wb");
    if (g_pfd == TD_NULL) {
        printf("open file failed, errno %d!\n", errno);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_bool sample_vo_release_one_frame(ot_vo_layer layer, ot_vo_chn chn, const ot_video_frame_info *frame_info)
{
    if (ss_mpi_vo_release_chn_frame(layer, chn, frame_info) != TD_SUCCESS) {
        printf("chn(%d, %d) release frame failed.\n", layer, chn);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_bool sample_vo_get_block(td_u32 blk_size)
{
    g_mem.h_block = ss_mpi_vb_get_blk(g_mem.h_pool, blk_size, TD_NULL);
    if (g_mem.h_block == OT_VB_INVALID_HANDLE) {
        printf("get vb blk fail\n");
        return TD_FAILURE;
    }
    g_mem.phy_addr = ss_mpi_vb_handle_to_phys_addr(g_mem.h_block);
    return TD_SUCCESS;
}

static td_void sample_vo_release_block(td_void)
{
    ss_mpi_vb_release_blk(g_mem.h_block);
    g_mem.h_block = OT_VB_INVALID_HANDLE;
}

static td_bool sample_dump_one_frm(ot_vo_layer vo_layer, td_s32 i)
{
    td_s32 ret;
    td_u32 blk_size;
    ot_video_frame_info frm_info;
    ot_vb_calc_cfg cal_config;

    ret = ss_mpi_vo_get_chn_frame(vo_layer, g_vo_chn_id, &g_frame, 20); /* 20ms */
    if (ret != TD_SUCCESS) {
        printf("get vo(%d,%d) frame err, only get %d frame\n", vo_layer, g_vo_chn_id, i);
        return TD_FAILURE;
    }

    sample_get_cal_config(&cal_config);
    blk_size = cal_config.vb_size;

    if (sample_is_send_to_vgs(&g_frame) == TD_TRUE) {
        if (sample_vo_get_block(blk_size) != TD_SUCCESS) {
            if (sample_vo_release_one_frame(vo_layer, g_vo_chn_id, &g_frame) == TD_SUCCESS) {
                sample_vo_clear_frame();
            }
            return TD_FAILURE;
        }

        sample_set_frame_info(&frm_info, i);

        if (sample_vgs_process(&frm_info) != TD_SUCCESS) {
            sample_vo_release_block();
            if (sample_vo_release_one_frame(vo_layer, g_vo_chn_id, &g_frame) == TD_SUCCESS) {
                sample_vo_clear_frame();
            }
            return TD_FAILURE;
        }

        /* save VO frame to file */
        sample_yuv_dump(&frm_info.video_frame);
        sample_vo_release_block();
    } else {
        /* save VO frame to file */
        sample_yuv_dump(&g_frame.video_frame);
    }

    /* release frame after using */
    if (sample_vo_release_one_frame(vo_layer, g_vo_chn_id, &g_frame) != TD_SUCCESS) {
        printf("vo(%d,%d) only get %d frame\n", vo_layer, g_vo_chn_id, i);
        return TD_FAILURE;
    }

    sample_vo_clear_frame();

    return TD_SUCCESS;
}

static td_void sample_vo_close_fd(td_void)
{
    if (g_pfd != TD_NULL) {
        fclose(g_pfd);
        g_pfd = TD_NULL;
    }
}

static td_void sample_misc_vo_dump(ot_vo_layer vo_layer, td_s32 cnt)
{
    td_s32 i;

    sample_vo_clear_frame();
    if (sample_make_yuv_file_name(vo_layer, cnt) != TD_SUCCESS) {
        return;
    }

    if (sample_is_send_to_vgs(&g_frame) == TD_TRUE) {
        if (sample_create_vb_pool() != TD_SUCCESS) {
            sample_vo_close_fd();
            return;
        }
    }
    sample_vo_clear_frame();

    /* get VO frame  */
    for (i = 0; i < cnt; i++) {
        if (g_signal_flag == 1) {
            printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
            break;
        }

        if (sample_dump_one_frm(vo_layer, i) != TD_SUCCESS) {
            break;
        }
    }

    if (g_h_pool != OT_VB_INVALID_POOL_ID) {
        ss_mpi_vb_destroy_pool(g_h_pool);
        g_h_pool = OT_VB_INVALID_POOL_ID;
    }

    sample_vo_clear_frame();
    sample_vo_close_fd();
}

static td_s32 vo_dump_get_argv_val(char *argv[], vo_dump_arg_index index, td_slong *val)
{
    td_char *end_ptr = TD_NULL;
    td_slong result;

    errno = 0;
    result = strtol(argv[index], &end_ptr, VO_DUMP_ARG_NUMBER_BASE);
    if ((end_ptr == argv[index]) || (*end_ptr != '\0')) {
        return TD_FAILURE;
    } else if (((result == LONG_MIN) || (result == LONG_MAX)) &&
        (errno == ERANGE)) {
        return TD_FAILURE;
    }

    *val = result;
    return TD_SUCCESS;
}

static const td_char *vo_dump_get_argv_name(vo_dump_arg_index index)
{
    const td_char *argv_name[VO_DUMP_ARG_BUTT] = {"vo_chn_dump", "vo_layer", "vo_chn", "frm_cnt"};

    if (index >= VO_DUMP_ARG_BUTT) {
        return "-";
    }

    return argv_name[index];
}

static td_void vo_dump_print_frm_cnt_notice(vo_dump_arg_index index)
{
    if (index == VO_DUMP_ARG_FRM_CNT) {
        printf("notice:if you want to dump more than %d cnt, can modify the tool MAX_FRM_CNT value!\n", MAX_FRM_CNT);
    }
}

static td_s32 vo_dump_get_valid_argv_val(char *argv[], vo_dump_arg_index index,
    td_s32 min_val, td_s32 max_val, td_s32 *val)
{
    td_s32 ret;
    td_slong result = 0;

    ret = vo_dump_get_argv_val(argv, index, &result);
    if (ret != TD_SUCCESS) {
        printf("Failure: input arg index(%u) name(%s) arg is invalid!\n",
            index, vo_dump_get_argv_name(index));
        usage();
        return ret;
    }

    if ((result < min_val) || (result > max_val)) {
        printf("Failure: input arg index(%u) name(%s) arg val (%ld) is wrong. should be [%d, %d]!\n",
            index, vo_dump_get_argv_name(index), result, min_val, max_val);
        vo_dump_print_frm_cnt_notice(index);
        usage();
        return TD_FAILURE;
    }

    *val = (td_s32)result;
    return TD_SUCCESS;
}

#ifdef __LITEOS__
td_s32 vo_chn_dump(int argc, char *argv[])
#else
td_s32 main(int argc, char *argv[])
#endif
{
    td_s32 ret;
    td_s32 frm_cnt = 1;

    printf("\nnotice: this tool only can be used for TESTING !!!\n");
#ifndef __LITEOS__
    printf("\tTo see more usage, please enter: ./vo_chn_dump -h\n\n");
#else
    printf("\tTo see more usage, please enter: vo_chn_dump -h\n\n");
#endif

    if (argc > VO_DUMP_ARG_BUTT) {
        printf("Too many parameters!\n");
        usage();
        return TD_FAILURE;
    }

    /* VO video layer ID */
    if (argc > VO_DUMP_ARG_VO_LAYER) {
        if (!strncmp(argv[1], "-h", 2)) { /* 2 chars */
            usage();
            return TD_SUCCESS;
        }

        ret = vo_dump_get_valid_argv_val(argv, VO_DUMP_ARG_VO_LAYER, 0, OT_VO_MAX_LAYER_NUM - 1, &g_vo_layer_id);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    /* VO chn id */
    if (argc > VO_DUMP_ARG_VO_CHN) {
        ret = vo_dump_get_valid_argv_val(argv, VO_DUMP_ARG_VO_CHN, 0, OT_VO_MAX_CHN_NUM - 1, &g_vo_chn_id);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    /* need frmcnt */
    if (argc > VO_DUMP_ARG_FRM_CNT) {
        ret = vo_dump_get_valid_argv_val(argv, VO_DUMP_ARG_FRM_CNT, 1, MAX_FRM_CNT, &frm_cnt);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    g_signal_flag = 0;

#ifndef __LITEOS__
    (void)signal(SIGINT, vo_chn_handle_sig);
    (void)signal(SIGTERM, vo_chn_handle_sig);
#endif

    sample_misc_vo_dump(g_vo_layer_id, frm_cnt);

    return TD_SUCCESS;
}
