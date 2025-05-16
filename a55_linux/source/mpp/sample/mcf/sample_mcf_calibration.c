/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "sample_mcf_calibration.h"
#include "sample_comm.h"

#define MONO_FILE0_NAME "./data/MCF_calibration_1920x1080_8bit_420p_mono.yuv"
#define MONO_FILE1_NAME "./data/MCF_calibration_2560x1440_8bit_420p_mono.yuv"
#define COLOR_FILE_NAME "./data/MCF_calibration_1920x1080_8bit_420p_color.yuv"
#define SCALE_FILE_NAME "./data/MCF_calibration_scale_2560x1440_8bit_420p_color.yuv"
#define MONO_IMG_SAME_SIZE_WIDTH 1920
#define MONO_IMG_SAME_SIZE_HEIGHT 1080
#define MONO_IMG_DIFF_SIZE_WIDTH 2560
#define MONO_IMG_DIFF_SIZE_HEIGHT 1440
#define COLOR_IMG_SIZE_WIDTH 1920
#define COLOR_IMG_SIZE_HEIGHT 1080

typedef struct {
    ot_vb_blk vb_blk;
    ot_vb_pool vb_pool;
    td_u32 pool_id;

    td_phys_addr_t phys_addr;
    td_void *virt_addr;
} sample_mcf_dump_mem;

static ot_vb_pool g_pool = OT_VB_INVALID_POOL_ID;

static td_void safe_free(td_void *memory)
{
    if (memory != TD_NULL) {
        free(memory);
        memory = TD_NULL;
    }
}

static td_void sample_mcf_read_y_data_frame(FILE *fp, const ot_video_frame *video_frame)
{
    td_u8 *dst = TD_NULL;
    td_u32 row;
    td_u32 width = video_frame->width;
    td_u32 height = video_frame->height;
    td_u32 stride = video_frame->stride[0];
    td_u8 *temp = malloc(stride);
    td_s32 ret;
    if (temp == TD_NULL) {
        sample_print("malloc fail\n");
        return;
    }
    dst = video_frame->virt_addr[0];
    for (row = 0; row < height; row++) {
        ret = fread(dst, width, 1, fp);
        if (ret < -1) {
            sample_print("fread fail\n");
            safe_free(temp);
            return;
        }
        dst += stride;
    }

    ret = fread(temp, 1, 1, fp);
    if (ret < -1) {
        sample_print("fread fail\n");
        safe_free(temp);
        return;
    }

    if (feof(fp) != 0) {
        fseek(fp, 0, SEEK_SET);
    } else {
        fseek(fp, ftell(fp) - 1, SEEK_SET);
    }

    safe_free(temp);
    return;
}

static td_void sample_mcf_show_cal_info(ot_mcf_feature_info *feature_info, ot_mcf_calibration *cal_info)
{
    td_u8 i;

    printf("\nshow matrix of calibration:\n");
    for (i = 0; i < 3; i++) {   /* 3 col */
        printf("%12lld,%12lld,%12lld,\n",
            cal_info->correct_coef[i * 3],  /* 3 col */
            cal_info->correct_coef[i * 3 + 1],  /* 3 col */
            cal_info->correct_coef[i * 3 + 2]);  /* 3 col , 2 offset */
    }

    printf("\nshow offset of calibration:\n");
    printf("position: x:%d, y:%d,  w:%u, h:%u\n", cal_info->region.x, cal_info->region.y,
        cal_info->region.width, cal_info->region.height);
    printf("crop_info: x:%d,  y:%d,  w:%u, h:%u\n", cal_info->ratio_crop.x, cal_info->ratio_crop.y,
        cal_info->ratio_crop.width, cal_info->ratio_crop.height);

    printf("\nshow feature num of calibration:\n");
    printf("num of refer feature: %d\n", feature_info->refer_feature_num);
    printf("num of register feature: %d\n", feature_info->register_feature_num);
    printf("num of match feature: %d\n\n", feature_info->match_feature_num);

    return;
}

static td_s32 sample_mcf_set_video_frame(ot_video_frame *video_frame, td_char *file_name, ot_size img_size,
    td_u32 file_name_size)
{
    td_s32 ret;
    td_char *path = TD_NULL;
    td_char stream_file_name[FILE_NAME_LEN];
    td_char *frame_buff = TD_NULL;

    if (file_name_size > FILE_NAME_LEN) {
        sample_print("file name size if large than %d\n", FILE_NAME_LEN);
        return TD_FAILURE;
    }
    if (file_name == TD_NULL) {
        sample_print("file name is null.\n");
        return TD_FAILURE;
    }

    ret = snprintf_s(stream_file_name, FILE_NAME_LEN, FILE_NAME_LEN - 1, file_name);
    if (ret < 0) {
        sample_print("snprintf_s file fail.\n");
        return TD_FAILURE;
    }

    path = realpath(stream_file_name, TD_NULL);
    if (path == TD_NULL) {
        sample_print("realpath file %s failed", stream_file_name);
        return TD_FAILURE;
    }

    FILE *stream = fopen(path, "rb");
    if (stream == TD_NULL) {
        sample_print("can't open file %s\n", stream_file_name);
        return TD_FAILURE;
    }

    frame_buff = malloc(img_size.width * img_size.height);
    if (frame_buff == TD_NULL) {
        ret = TD_FAILURE;
        goto exit;
    }
    video_frame->width = img_size.width;
    video_frame->height = img_size.height;
    video_frame->pixel_format = OT_PIXEL_FORMAT_YUV_400;
    video_frame->stride[0] = img_size.width;
    video_frame->virt_addr[0] = frame_buff;
    sample_mcf_read_y_data_frame(stream, video_frame);
    ret = TD_SUCCESS;
exit:
    if (fclose(stream) != 0) {
        sample_print("fclose error\n");
    }
    return ret;
}

static td_s32 sample_mcf_calibration_show_cal_info(ot_video_frame *mono_frame, ot_video_frame *color_frame,
    ot_mcf_calibration_mode mode, ot_mcf_feature_info *feature_info, ot_mcf_calibration *cal_info)
{
    td_s32 ret;
    ret = ss_mpi_mcf_calibration(mono_frame, color_frame, mode, feature_info, cal_info);
    if (ret != TD_SUCCESS) {
        sample_print("ot_mcf_calibration failed\n");
        goto exit1;
    }
    sample_mcf_show_cal_info(feature_info, cal_info);
    sample_print("sample mcf calibration success\n");

exit1:
    safe_free(color_frame->virt_addr[0]);
    safe_free(mono_frame->virt_addr[0]);
    return ret;
}

/* for diff img scale */
typedef struct {
    td_bool scale;
    ot_vgs_scale_coef_mode *vgs_scl_coef_mode;
    sample_vb_base_info *in_img_vb_info;
    sample_vb_base_info *out_img_vb_info;
    td_s32 sample_num;
    td_char in_file_name[FILE_NAME_LEN];
    td_char out_file_name[FILE_NAME_LEN];
} sample_mcf_vgs_func_param;

typedef struct {
    ot_vb_blk vb_handle;
    td_void *virt_addr;
    td_u32 vb_size;
    td_bool vb_used;
} sample_mcf_vgs_vb_info;

sample_mcf_vgs_vb_info g_in_img_vb_info;
sample_mcf_vgs_vb_info g_out_img_vb_info;
td_u8 *g_temp = TD_NULL;

static td_void sample_mcf_vgs_get_yuv_buffer_cfg(const sample_vb_base_info *vb_base_info,
    ot_vb_calc_cfg *vb_cal_config)
{
    ot_pic_buf_attr buf_attr;
    buf_attr.width = vb_base_info->width;
    buf_attr.height = vb_base_info->height;
    buf_attr.video_format = OT_VIDEO_FORMAT_LINEAR;
    buf_attr.pixel_format = vb_base_info->pixel_format;
    buf_attr.bit_width = OT_DATA_BIT_WIDTH_8;
    buf_attr.compress_mode = vb_base_info->compress_mode;
    buf_attr.align = vb_base_info->align;
    ot_common_get_pic_buf_cfg(&buf_attr, vb_cal_config);
    return;
}

static td_void sample_mcf_vgs_set_frame_info(const ot_vb_calc_cfg *vb_cal_config, const sample_vb_base_info *vb_info,
    const sample_mcf_vgs_vb_info *vgs_vb_info, td_phys_addr_t phys_addr, ot_video_frame_info *frame_info)
{
    frame_info->mod_id = OT_ID_VGS;
    frame_info->pool_id = ss_mpi_vb_handle_to_pool_id(vgs_vb_info->vb_handle);

    frame_info->video_frame.width       = vb_info->width;
    frame_info->video_frame.height      = vb_info->height;
    frame_info->video_frame.field        = OT_VIDEO_FIELD_FRAME;
    frame_info->video_frame.pixel_format  = vb_info->pixel_format;
    frame_info->video_frame.video_format  = vb_info->video_format;
    frame_info->video_frame.compress_mode = vb_info->compress_mode;
    frame_info->video_frame.dynamic_range = OT_DYNAMIC_RANGE_SDR8;
    frame_info->video_frame.color_gamut   = OT_COLOR_GAMUT_BT601;

    frame_info->video_frame.header_stride[0]  = vb_cal_config->head_stride;
    frame_info->video_frame.header_stride[1]  = vb_cal_config->head_stride;
    frame_info->video_frame.header_phys_addr[0] = phys_addr;
    frame_info->video_frame.header_phys_addr[1] = frame_info->video_frame.header_phys_addr[0] +
                                                  vb_cal_config->head_y_size;
    frame_info->video_frame.header_virt_addr[0] = vgs_vb_info->virt_addr;
    frame_info->video_frame.header_virt_addr[1] = frame_info->video_frame.header_virt_addr[0] +
                                                  vb_cal_config->head_y_size;

    frame_info->video_frame.stride[0]  = vb_cal_config->main_stride;
    frame_info->video_frame.stride[1]  = vb_cal_config->main_stride;
    frame_info->video_frame.phys_addr[0] = frame_info->video_frame.header_phys_addr[0] + vb_cal_config->head_size;
    frame_info->video_frame.phys_addr[1] = frame_info->video_frame.phys_addr[0] + vb_cal_config->main_y_size;
    frame_info->video_frame.virt_addr[0] = frame_info->video_frame.header_virt_addr[0] + vb_cal_config->head_size;
    frame_info->video_frame.virt_addr[1] = frame_info->video_frame.virt_addr[0] + vb_cal_config->main_y_size;

    return;
}

static td_s32 sample_mcf_vgs_get_frame_vb(const sample_vb_base_info *vb_info, const ot_vb_calc_cfg *vb_cal_config,
    ot_video_frame_info *frame_info, sample_mcf_vgs_vb_info *vgs_vb_info)
{
    td_phys_addr_t phys_addr;

    vgs_vb_info->vb_handle = ss_mpi_vb_get_blk(OT_VB_INVALID_POOL_ID, vb_cal_config->vb_size, TD_NULL);
    if (vgs_vb_info->vb_handle == OT_VB_INVALID_HANDLE) {
        sample_print("ss_mpi_vb_get_block failed!\n");
        return TD_FAILURE;
    }
    vgs_vb_info->vb_used = TD_TRUE;

    phys_addr = ss_mpi_vb_handle_to_phys_addr(vgs_vb_info->vb_handle);
    if (phys_addr == 0) {
        sample_print("ss_mpi_vb_handle2_phys_addr failed!\n");
        ss_mpi_vb_release_blk(vgs_vb_info->vb_handle);
        vgs_vb_info->vb_used = TD_FALSE;
        return TD_FAILURE;
    }

    vgs_vb_info->virt_addr = (td_u8*)ss_mpi_sys_mmap(phys_addr, vb_cal_config->vb_size);
    if (vgs_vb_info->virt_addr == TD_NULL) {
        sample_print("ss_mpi_sys_mmap failed!\n");
        ss_mpi_vb_release_blk(vgs_vb_info->vb_handle);
        vgs_vb_info->vb_used = TD_FALSE;
        return TD_FAILURE;
    }
    vgs_vb_info->vb_size = vb_cal_config->vb_size;

    sample_mcf_vgs_set_frame_info(vb_cal_config, vb_info, vgs_vb_info, phys_addr, frame_info);

    return TD_SUCCESS;
}

static td_s32 sample_mcf_vgs_init_sys_and_vb(const sample_mcf_vgs_func_param *param, FILE **file_write)
{
    ot_vb_calc_cfg in_img_vb_cal_config = {0};
    ot_vb_cfg vb_conf = {0};
    td_char *path = TD_NULL;
    td_s32 ret;

    vb_conf.max_pool_cnt = 2; /* there are 2 pools in 1 vb */
    sample_mcf_vgs_get_yuv_buffer_cfg(param->out_img_vb_info, &in_img_vb_cal_config);
    vb_conf.common_pool[0].blk_size = in_img_vb_cal_config.vb_size;
    vb_conf.common_pool[0].blk_cnt = 2; /* there are 2 blks in 1 pool */
    ret = sample_comm_sys_vb_init(&vb_conf);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_sys_vb_init failed, ret:0x%x\n", ret);
        return ret;
    }

    path = realpath(param->out_file_name, TD_NULL);
    if (path == TD_NULL) {
        sample_print("realpath file %s failed\n", param->out_file_name);
        return TD_FAILURE;
    }

    *file_write = fopen(path, "w+");
    if (*file_write == TD_NULL) {
        sample_print("can't open file %s\n", param->out_file_name);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 sample_mcf_vgs_convert_chroma_planar_to_sp42x(FILE *file, td_u8 *chroma_data,
    td_u32 luma_stride, td_u32 chroma_width, td_u32 chroma_height)
{
    td_u32 chroma_stride = luma_stride >> 1;
    td_u8 *dst = TD_NULL;
    td_u32 row;
    td_u32 list;
    td_s32 ret;
    g_temp = (td_u8*)malloc(chroma_stride);
    if (g_temp == TD_NULL) {
        sample_print("vgs malloc failed!\n");
        return TD_FAILURE;
    }
    (td_void)memset_s(g_temp, chroma_stride, 0, chroma_stride);
    /* U */
    dst = chroma_data + 1;
    for (row = 0; row < chroma_height; ++row) {
        ret = fread(g_temp, chroma_width, 1, file); /* sp420 U-component data starts 1/2 way from the beginning */
        if (ret < -1) {
            sample_print("fread error\n");
            safe_free(g_temp);
            return TD_FAILURE;
        }
        for (list = 0; list < chroma_stride; ++list) {
            *dst = *(g_temp + list);
            dst += 2; /* traverse 2 steps away to the next U-component data */
        }
        dst = chroma_data + 1;
        dst += (row + 1) * luma_stride;
    }

    /* V */
    dst = chroma_data;
    for (row = 0; row < chroma_height; ++row) {
        ret = fread(g_temp, chroma_width, 1, file); /* sp420 V-component data starts 1/2 way from the beginning */
        if (ret < -1) {
            sample_print("fread error\n");
            safe_free(g_temp);
            return TD_FAILURE;
        }
        for (list = 0; list < chroma_stride; ++list) {
            *dst = *(g_temp + list);
            dst += 2; /* traverse 2 steps away to the next V-component data */
        }
        dst = chroma_data;
        dst += (row + 1) * luma_stride;
    }

    safe_free(g_temp);
    return TD_SUCCESS;
}

static td_s32 sample_mcf_vgs_read_file_to_sp42x(FILE *file, ot_video_frame *frame)
{
    td_u8 *luma = (td_u8*)(td_uintptr_t)frame->virt_addr[0];
    td_u8 *chroma = (td_u8*)(td_uintptr_t)frame->virt_addr[1];
    const td_u32 luma_width = frame->width;
    const td_u32 chroma_width = luma_width >> 1;
    const td_u32 luma_height = frame->height;
    td_u32 chroma_height = luma_height;
    const td_u32 luma_stride = frame->stride[0];

    td_u32 row;

    if (frame->video_format == OT_VIDEO_FORMAT_LINEAR) {
        /* Y */
        td_u8 *dst = luma;
        for (row = 0; row < luma_height; ++row) {
            fread(dst, luma_width, 1, file);
            dst += luma_stride;
        }

        if (OT_PIXEL_FORMAT_YUV_400 == frame->pixel_format) {
            return TD_SUCCESS;
        } else if (OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420 == frame->pixel_format) {
            chroma_height = chroma_height >> 1;
        }
        if (sample_mcf_vgs_convert_chroma_planar_to_sp42x(
            file, chroma, luma_stride, chroma_width, chroma_height) != TD_SUCCESS) {
            return TD_FAILURE;
        }
    } else {
        sample_print("not support tile format\n");
    }
    return TD_SUCCESS;
}

static td_s32 sample_mcf_vgs_get_frame_from_file(sample_mcf_vgs_func_param *param,
    ot_vb_calc_cfg *in_img_vb_cal_config, ot_video_frame_info *frame_info)
{
    td_s32 ret = TD_FAILURE;
    td_char *path = TD_NULL;
    FILE *file_read = TD_NULL;

    path = realpath(param->in_file_name, TD_NULL);
    if (path == TD_NULL) {
        sample_print("realpath file %s failed", param->in_file_name);
        goto EXIT;
    }

    file_read = fopen(path, "rb");
    if (file_read == TD_NULL) {
        sample_print("can't open file %s\n", param->in_file_name);
        goto EXIT;
    }

    ret = sample_mcf_vgs_get_frame_vb(param->in_img_vb_info, in_img_vb_cal_config, frame_info, &g_in_img_vb_info);
    if (ret != TD_SUCCESS) {
        goto EXIT1;
    }

    ret = sample_mcf_vgs_read_file_to_sp42x(file_read, &frame_info->video_frame);
    if (ret != TD_SUCCESS) {
        goto EXIT2;
    } else {
        goto EXIT1;
    }

EXIT2:
    ss_mpi_sys_munmap(frame_info->video_frame.header_virt_addr[0], in_img_vb_cal_config->vb_size);
    ss_mpi_vb_release_blk(g_in_img_vb_info.vb_handle);
    g_in_img_vb_info.vb_used = TD_FALSE;
EXIT1:
    if (fclose(file_read) != 0) {
        sample_print("fclose fail\n");
    }
EXIT:
    return ret;
}

static td_s32 sample_mcf_vgs_get_in_out_frame(sample_mcf_vgs_func_param *param, ot_vgs_task_attr *vgs_task_attr)
{
    ot_vb_calc_cfg in_img_vb_cal_config = {0};
    ot_vb_calc_cfg out_img_vb_cal_config = {0};
    td_s32 ret;

    sample_mcf_vgs_get_yuv_buffer_cfg(param->in_img_vb_info, &in_img_vb_cal_config);
    ret = sample_mcf_vgs_get_frame_from_file(param, &in_img_vb_cal_config, &vgs_task_attr->img_in);
    if (ret != TD_SUCCESS) {
        sample_print("sample_mcf_vgs_get_frame_from_file failed, ret:0x%x\n", ret);
        return TD_FAILURE;
    }

    sample_mcf_vgs_get_yuv_buffer_cfg(param->out_img_vb_info, &out_img_vb_cal_config);
    ret = sample_mcf_vgs_get_frame_vb(param->out_img_vb_info, &out_img_vb_cal_config,
                                      &vgs_task_attr->img_out, &g_out_img_vb_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_void sample_mcf_vgs_common_function_exit_release(const sample_mcf_vgs_func_param *param,
    ot_vgs_task_attr *vgs_task_attr, FILE **file_write)
{
    ot_vb_calc_cfg in_img_vb_cal_config = {0};
    ot_vb_calc_cfg out_img_vb_cal_config = {0};
    if (g_out_img_vb_info.vb_used == TD_TRUE) {
        sample_mcf_vgs_get_yuv_buffer_cfg(param->out_img_vb_info, &out_img_vb_cal_config);
        ss_mpi_sys_munmap(vgs_task_attr->img_out.video_frame.header_virt_addr[0], out_img_vb_cal_config.vb_size);
        ss_mpi_vb_release_blk(g_out_img_vb_info.vb_handle);
        g_out_img_vb_info.vb_used = TD_FALSE;
    }
    if (*file_write != TD_NULL) {
        fclose(*file_write);
        *file_write = TD_NULL;
    }
    if (g_in_img_vb_info.vb_used == TD_TRUE) {
        sample_mcf_vgs_get_yuv_buffer_cfg(param->in_img_vb_info, &in_img_vb_cal_config);
        ss_mpi_sys_munmap(vgs_task_attr->img_in.video_frame.header_virt_addr[0], in_img_vb_cal_config.vb_size);
        ss_mpi_vb_release_blk(g_in_img_vb_info.vb_handle);
        g_in_img_vb_info.vb_used = TD_FALSE;
    }
}

static td_s32 sample_mcf_vgs_add_task(const sample_mcf_vgs_func_param *param, ot_vgs_handle h_handle,
    ot_vgs_task_attr *vgs_task_attr)
{
    td_s32 ret;
    if (param->scale) {
        ret = ss_mpi_vgs_add_scale_task(h_handle, vgs_task_attr, *param->vgs_scl_coef_mode);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_vgs_add_scale_task failed, ret:0x%x", ret);
            return ret;
        }
    }
    return TD_SUCCESS;
}

static td_s32 sample_mcf_vgs_save_planar_uv_data(FILE *file, td_u8 *dst, td_u32 chroma_width)
{
    td_u32 list;

    for (list = 0; list < chroma_width; ++list) {
        *(g_temp + list) = *dst;
        dst += 2; /* traverse 2 steps away to the next U/V component data */
    }

    if (fwrite(g_temp, 1, chroma_width, file) != chroma_width) {
        sample_print("fwrite error\n");
        safe_free(g_temp);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 sample_mcf_vgs_convert_chroma_sp42x_to_planar(FILE *file, td_u8 *chroma_data,
    td_u32 luma_stride, td_u32 chroma_width, td_u32 chroma_height)
{
    td_u32 row;
    g_temp = (td_u8*)malloc(chroma_width);
    if (g_temp == TD_NULL) {
        sample_print("vgs malloc failed!.\n");
        return TD_FAILURE;
    }
    (td_void)memset_s(g_temp, chroma_width, 0, chroma_width);

    /* U */
    for (row = 0; row < chroma_height; ++row) {
        td_u8 *dst = chroma_data + row * luma_stride + 1;
        if (sample_mcf_vgs_save_planar_uv_data(file, dst, chroma_width) != TD_SUCCESS) {
            return TD_FAILURE;
        }
    }

    /* V */
    for (row = 0; row < chroma_height; ++row) {
        td_u8 *dst = chroma_data + row * luma_stride;
        if (sample_mcf_vgs_save_planar_uv_data(file, dst, chroma_width) != TD_SUCCESS) {
            return TD_FAILURE;
        }
    }

    safe_free(g_temp);
    return TD_SUCCESS;
}

static td_s32 sample_mcf_vgs_save_sp42x_to_planar(FILE *file, ot_video_frame *frame)
{
    const td_u32 luma_width = frame->width;
    const td_u32 chroma_width = luma_width >> 1;
    const td_u32 luma_height = frame->height;
    td_u32 chroma_height = luma_height;
    const td_u32 luma_stride = frame->stride[0];
    td_u8 *luma = (td_u8*)(td_uintptr_t)frame->virt_addr[0];
    td_u8 *chroma = (td_u8*)(td_uintptr_t)frame->virt_addr[1];

    td_u8 *dst = TD_NULL;
    td_u32 row;
    td_u32 ret;
    /* Y */
    dst = luma;
    for (row = 0; row < luma_height; ++row) {
        ret = fwrite(dst, 1, luma_width, file);
        if (ret != luma_width) {
            sample_print("fwrite error\n");
            return TD_FAILURE;
        }
        dst += luma_stride;
    }

    if (OT_PIXEL_FORMAT_YUV_400 == frame->pixel_format) {
        return TD_SUCCESS;
    } else if (OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420 == frame->pixel_format) {
        chroma_height = chroma_height >> 1;
    }

    if (sample_mcf_vgs_convert_chroma_sp42x_to_planar(
        file, chroma, luma_stride, chroma_width, chroma_height) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 sample_mcf_vgs_common_function(sample_mcf_vgs_func_param *param)
{
    td_s32 ret;
    ot_vgs_handle h_handle = -1;
    ot_vgs_task_attr vgs_task_attr = {0};
    FILE *file_write = TD_NULL;

    if (param == TD_NULL || param->in_img_vb_info == TD_NULL) {
        return TD_FAILURE;
    }

    /* step1: init SYS and common VB */
    ret = sample_mcf_vgs_init_sys_and_vb(param, &file_write);
    if (ret != TD_SUCCESS) {
        goto exit_and_release;
    }

    /* step2: get frame */
    ret = sample_mcf_vgs_get_in_out_frame(param, &vgs_task_attr);
    if (ret != TD_SUCCESS) {
        goto exit_and_release;
    }

    /* step3: create VGS job */
    ret = ss_mpi_vgs_begin_job(&h_handle);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_vgs_begin_job failed, ret:0x%x", ret);
        goto exit_and_release;
    }

    /* step4: add VGS task */
    ret = sample_mcf_vgs_add_task(param, h_handle, &vgs_task_attr);
    if (ret != TD_SUCCESS) {
        ss_mpi_vgs_cancel_job(h_handle);
        goto exit_and_release;
    }

    /* step5: start VGS work */
    ret = ss_mpi_vgs_end_job(h_handle);
    if (ret != TD_SUCCESS) {
        ss_mpi_vgs_cancel_job(h_handle);
        sample_print("ss_mpi_vgs_end_job failed, ret:0x%x", ret);
        goto exit_and_release;
    }

    /* step6: save the frame to file */
    ret = sample_mcf_vgs_save_sp42x_to_planar(file_write, &vgs_task_attr.img_out.video_frame);
    if (ret != TD_SUCCESS) {
        goto exit_and_release;
    }
    if (fflush(file_write) != 0) {
        sample_print("fflush fail\n");
    }

    /* step7: exit */
exit_and_release:
    sample_mcf_vgs_common_function_exit_release(param, &vgs_task_attr, &file_write);
    sample_comm_sys_exit();
    return ret;
}

static td_s32 sample_mcf_scale_img(sample_comm_mcf_scale_img_param *scale_img_param)
{
    td_s32 ret;
    sample_mcf_vgs_func_param vgs_func_param = {0};
    sample_vb_base_info in_img_vb_info;
    sample_vb_base_info out_img_vb_info;
    ot_vgs_scale_coef_mode vgs_scl_coef_mode = OT_VGS_SCALE_COEF_NORM;
    td_char *input_file_name = TD_NULL;
    ot_size input_img_size, output_img_size;
    td_char *output_file_name = TD_NULL;

    input_file_name = scale_img_param->input_file_name;
    output_file_name = scale_img_param->output_file_name;
    input_img_size = scale_img_param->input_img_size;
    output_img_size = scale_img_param->output_img_size;

    in_img_vb_info.video_format = OT_VIDEO_FORMAT_LINEAR;
    in_img_vb_info.pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    in_img_vb_info.width = input_img_size.width;
    in_img_vb_info.height = input_img_size.height;
    in_img_vb_info.align = 0;
    in_img_vb_info.compress_mode = OT_COMPRESS_MODE_NONE;

    if (memcpy_s(&out_img_vb_info, sizeof(sample_vb_base_info), &in_img_vb_info, sizeof(sample_vb_base_info)) != EOK) {
        sample_print("memcpy_s failed\n");
        return TD_FAILURE;
    }
    out_img_vb_info.width = output_img_size.width;
    out_img_vb_info.height = output_img_size.height;

    vgs_func_param.scale = TD_TRUE;
    vgs_func_param.vgs_scl_coef_mode = &vgs_scl_coef_mode;
    vgs_func_param.in_img_vb_info = &in_img_vb_info;
    vgs_func_param.out_img_vb_info = &out_img_vb_info;
    if (memcpy_s(vgs_func_param.in_file_name, FILE_NAME_LEN, input_file_name, FILE_NAME_LEN) != EOK) {
        sample_print("memcpy_s failed.\n");
        return TD_FAILURE;
    }

    if (memcpy_s(vgs_func_param.out_file_name, FILE_NAME_LEN, output_file_name, FILE_NAME_LEN) != EOK) {
        sample_print("memcpy_s failed.\n");
        return TD_FAILURE;
    }

    ret = sample_mcf_vgs_common_function(&vgs_func_param);
    if (ret != TD_SUCCESS) {
        sample_print("VGS sample %d failed, ret:0x%x\n", vgs_func_param.sample_num, ret);
    }
    return ret;
}

td_s32 sample_mcf_calibration(td_void)
{
    td_s32 ret;
    ot_video_frame mono_frame, color_frame;
    ot_video_supplement_misc mono_misc, color_misc;
    ot_mcf_feature_info feature_info;
    ot_mcf_calibration cal_info;
    ot_mcf_calibration_mode mode = OT_MCF_CALIBRATION_AFFINE;
    ot_size img_size;

    (td_void)memset_s(&mono_frame, sizeof(ot_video_frame), 0, sizeof(ot_video_frame));
    (td_void)memset_s(&color_frame, sizeof(ot_video_frame), 0, sizeof(ot_video_frame));
    (td_void)memset_s(&mono_misc, sizeof(ot_video_supplement_misc), 0, sizeof(ot_video_supplement_misc));
    (td_void)memset_s(&color_misc, sizeof(ot_video_supplement_misc), 0, sizeof(ot_video_supplement_misc));
    img_size.width = MONO_IMG_SAME_SIZE_WIDTH;      /* same img width */
    img_size.height = MONO_IMG_SAME_SIZE_HEIGHT;      /* same img height */
    mono_frame.supplement.misc_info_virt_addr = &mono_misc;
    color_frame.supplement.misc_info_virt_addr = &color_misc;

    ret = sample_mcf_set_video_frame(&mono_frame, MONO_FILE0_NAME, img_size, sizeof(MONO_FILE0_NAME));
    if (ret != TD_SUCCESS) {
        sample_print("read mono calibration file failed\n");
        return ret;
    }

    ret = sample_mcf_set_video_frame(&color_frame, COLOR_FILE_NAME, img_size, sizeof(COLOR_FILE_NAME));
    if (ret != TD_SUCCESS) {
        sample_print("read color calibration file failed\n");
        safe_free(mono_frame.virt_addr[0]);
        return ret;
    }
    ret = sample_mcf_calibration_show_cal_info(&mono_frame, &color_frame, mode, &feature_info, &cal_info);
    return ret;
}

td_s32 sample_mcf_diff_img_size_calibration(td_void)
{
    td_s32 ret;
    ot_video_frame mono_frame, color_frame;
    ot_mcf_feature_info feature_info;
    ot_mcf_calibration cal_info;
    ot_mcf_calibration_mode mode = OT_MCF_CALIBRATION_AFFINE;
    ot_size mono_img_size, color_img_size;
    sample_comm_mcf_scale_img_param scale_img_param;
    (td_void)memset_s(&mono_frame, sizeof(ot_video_frame), 0, sizeof(ot_video_frame));
    (td_void)memset_s(&color_frame, sizeof(ot_video_frame), 0, sizeof(ot_video_frame));
    color_img_size.width = COLOR_IMG_SIZE_WIDTH;      /* color img width */
    color_img_size.height = COLOR_IMG_SIZE_HEIGHT;      /* color img height */
    mono_img_size.width = MONO_IMG_DIFF_SIZE_WIDTH;      /* mono img width */
    mono_img_size.height = MONO_IMG_DIFF_SIZE_HEIGHT;      /* mono img height */

    scale_img_param.input_file_name = COLOR_FILE_NAME;
    scale_img_param.input_img_size = color_img_size;
    scale_img_param.output_file_name = SCALE_FILE_NAME;
    scale_img_param.output_img_size = mono_img_size;
    ret = sample_mcf_scale_img(&scale_img_param);
    if (ret != TD_SUCCESS) {
        sample_print("mcf scale file failed\n");
        return TD_FAILURE;
    }

    ret = sample_mcf_set_video_frame(&mono_frame, MONO_FILE1_NAME, mono_img_size, sizeof(MONO_FILE1_NAME));
    if (ret != TD_SUCCESS) {
        sample_print("read mono calibration file failed\n");
        return ret;
    }

    ret = sample_mcf_set_video_frame(&color_frame, SCALE_FILE_NAME, mono_img_size, sizeof(SCALE_FILE_NAME));
    if (ret != TD_SUCCESS) {
        sample_print("read color calibration file failed\n");
        safe_free(mono_frame.virt_addr[0]);
        return ret;
    }
    ret = sample_mcf_calibration_show_cal_info(&mono_frame, &color_frame, mode, &feature_info, &cal_info);
    return ret;
}

/* *** mcf calibrate online *** */
static td_s32 sample_mcf_change_vi_chn_frame_depth(const ot_vi_chn vi_chn, td_u32 *orig_depth, const td_u32 num)
{
    if (num > OT_MCF_PIPE_NUM) {
        return TD_FAILURE;
    }

    ot_vi_chn_attr chn_attr;

    if (ss_mpi_vi_get_chn_attr(VI_MONO_PIPE, vi_chn, &chn_attr) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    orig_depth[0] = chn_attr.depth;
    chn_attr.depth = 2; /* depth 2 */
    if (ss_mpi_vi_set_chn_attr(VI_MONO_PIPE, vi_chn, &chn_attr) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if (ss_mpi_vi_get_chn_attr(VI_COLOR_PIPE, vi_chn, &chn_attr) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    orig_depth[1] = chn_attr.depth;
    chn_attr.depth = 2; /* depth 2 */
    if (ss_mpi_vi_set_chn_attr(VI_COLOR_PIPE, vi_chn, &chn_attr) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 sample_mcf_restore_vi_chn_frame_default_depth(const ot_vi_chn vi_chn, td_u32 *orig_depth,
    const td_u32 num)
{
    if (num > OT_MCF_PIPE_NUM) {
        return TD_FAILURE;
    }

    ot_vi_chn_attr chn_attr;

    if (ss_mpi_vi_get_chn_attr(VI_MONO_PIPE, vi_chn, &chn_attr) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    chn_attr.depth = orig_depth[0];
    if (ss_mpi_vi_set_chn_attr(VI_MONO_PIPE, vi_chn, &chn_attr) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if (ss_mpi_vi_get_chn_attr(VI_COLOR_PIPE, vi_chn, &chn_attr) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    chn_attr.depth = orig_depth[1];
    if (ss_mpi_vi_set_chn_attr(VI_COLOR_PIPE, vi_chn, &chn_attr) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 sample_mcf_get_vi_chn_frame(ot_mcf_grp_attr *mcf_grp_attr, const ot_vi_chn vi_chn,
    ot_video_frame_info *mono_frame_info, ot_video_frame_info *color_frame_info)
{
    td_s32 ret = TD_SUCCESS;
    const td_s32 milli_sec = 4000; /* 4000ms */

    ret = ss_mpi_vi_get_chn_frame(VI_MONO_PIPE, vi_chn, mono_frame_info, milli_sec);
    if (ret != TD_SUCCESS) {
        sample_print("vi_mono_pipe get chn frame failed with %#x!\n", ret);
        return ret;
    }

    ret = ss_mpi_vi_get_chn_frame(VI_COLOR_PIPE, vi_chn, color_frame_info, milli_sec);
    if (ret != TD_SUCCESS) {
        sample_print("vi_color_pipe get chn framefailed with %#x!\n", ret);
        ss_mpi_vi_release_chn_frame(VI_MONO_PIPE, vi_chn, mono_frame_info);
        mono_frame_info->pool_id = OT_VB_INVALID_POOL_ID;
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 sample_mcf_release_vi_chn(ot_mcf_grp_attr *mcf_grp_attr, const ot_vi_chn vi_chn,
    ot_video_frame_info *mono_frame_info, ot_video_frame_info *color_frame_info)
{
    td_s32 ret = TD_SUCCESS;

    ret = ss_mpi_vi_release_chn_frame(VI_MONO_PIPE, vi_chn, mono_frame_info);
    if (ret != TD_SUCCESS) {
        sample_print("vi_mono_pipe release chn frame of failed with %#x!\n", ret);
    }

    ret = ss_mpi_vi_release_chn_frame(VI_COLOR_PIPE, vi_chn, color_frame_info);
    if (ret != TD_SUCCESS) {
        sample_print("vi_color_pipe release chn frame of failed with %#x!\n", ret);
        return ret;
    }

    mono_frame_info->pool_id = OT_VB_INVALID_POOL_ID;
    color_frame_info->pool_id = OT_VB_INVALID_POOL_ID;

    return TD_SUCCESS;
}

static td_s32 sample_mcf_sys_mmap_frame_virt_addr(const td_u32 blk_size, ot_video_frame_info *mono_frame_info,
    ot_video_frame_info *color_frame_info)
{
    td_s32 ret = TD_SUCCESS;
    mono_frame_info->video_frame.virt_addr[0] =
        (td_u64 *)ss_mpi_sys_mmap(mono_frame_info->video_frame.phys_addr[0], blk_size);
    if (mono_frame_info->video_frame.virt_addr[0] == TD_NULL) {
        sample_print("sys mmap mono frame virt addr failed!\n");
        ret = TD_FAILURE;
    }

    color_frame_info->video_frame.virt_addr[0] =
        (td_u64 *)ss_mpi_sys_mmap(color_frame_info->video_frame.phys_addr[0], blk_size);
    if (color_frame_info->video_frame.virt_addr[0] == TD_NULL) {
        sample_print("sys mmap color frame virt addr failed!\n");
        ss_mpi_sys_munmap((td_u64 *)mono_frame_info->video_frame.virt_addr[0], blk_size);
        return TD_FAILURE;
    }

    return ret;
}

static td_s32 sample_mcf_sys_munmap_frame_virt_addr(const td_u32 blk_size, ot_video_frame_info *mono_frame_info,
    ot_video_frame_info *color_frame_info)
{
    td_s32 ret;
    ret = ss_mpi_sys_munmap((td_u64 *)mono_frame_info->video_frame.virt_addr[0], blk_size);
    if (ret != TD_SUCCESS) {
        sample_print("mpi sys munmap mono frame virt addr failed, ret:0x%x\n", ret);
    }

    ret = ss_mpi_sys_munmap((td_u64 *)color_frame_info->video_frame.virt_addr[0], blk_size);
    if (ret != TD_SUCCESS) {
        sample_print("mpi sys munmap color frame virt addr failed, ret:0x%x\n", ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 sample_mcf_init_vgs_pool(sample_mcf_dump_mem *dump_mem, ot_vb_calc_cfg *vb_calc_cfg,
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

static td_void sample_mcf_set_vgs_frame_info(ot_video_frame_info *vgs_frame_info, const sample_mcf_dump_mem *dump_mem,
    const ot_vb_calc_cfg *vb_calc_cfg, const ot_video_frame_info *frame_in, const ot_video_frame_info *frame_out)
{
    if ((frame_in == TD_NULL) || (vb_calc_cfg == TD_NULL) ||
        (dump_mem == TD_NULL) || (vgs_frame_info == TD_NULL)) {
        return;
    }

    vgs_frame_info->video_frame.width = frame_out->video_frame.width;
    vgs_frame_info->video_frame.height = frame_out->video_frame.height;

    vgs_frame_info->video_frame.phys_addr[0] = dump_mem->phys_addr;
    vgs_frame_info->video_frame.phys_addr[1] = vgs_frame_info->video_frame.phys_addr[0] + vb_calc_cfg->main_y_size;

    vgs_frame_info->video_frame.stride[0] = vb_calc_cfg->main_stride;
    vgs_frame_info->video_frame.stride[1] = vb_calc_cfg->main_stride;
    vgs_frame_info->video_frame.compress_mode = OT_COMPRESS_MODE_NONE;
    vgs_frame_info->video_frame.pixel_format = frame_out->video_frame.pixel_format;
    vgs_frame_info->video_frame.video_format = OT_VIDEO_FORMAT_LINEAR;
    vgs_frame_info->video_frame.field = OT_VIDEO_FIELD_FRAME;
    vgs_frame_info->video_frame.dynamic_range = frame_out->video_frame.dynamic_range;
    vgs_frame_info->video_frame.pts = 0;
    vgs_frame_info->video_frame.time_ref = 0;
    vgs_frame_info->pool_id = dump_mem->vb_pool;
    vgs_frame_info->mod_id = OT_ID_VGS;
}

static td_s32 sample_mcf_send_vgs_of_vi_chn(ot_video_frame_info *src_frame_info, ot_video_frame_info *dst_frame_info,
    ot_vb_calc_cfg *vb_calc_cfg, sample_mcf_dump_mem *dump_mem)
{
    ot_vgs_task_attr vgs_task_attr;
    ot_video_frame_info vgs_frame_info = {0};
    ot_vgs_handle h_handle = -1;

    if (sample_mcf_init_vgs_pool(dump_mem, vb_calc_cfg, dst_frame_info) != TD_SUCCESS) {
        printf("init vgs pool failed\n");
        return TD_FAILURE;
    }

    sample_mcf_set_vgs_frame_info(&vgs_frame_info, dump_mem, vb_calc_cfg, src_frame_info, dst_frame_info);

    if (ss_mpi_vgs_begin_job(&h_handle) != TD_SUCCESS) {
        printf("ss_mpi_vgs_begin_job failed\n");
        return TD_FAILURE;
    }

    if (memcpy_s(&vgs_task_attr.img_in, sizeof(ot_video_frame_info),
        src_frame_info, sizeof(ot_video_frame_info)) != EOK) {
        printf("memcpy_s img_in failed\n");
        goto err_exit;
    }

    if (memcpy_s(&vgs_task_attr.img_out, sizeof(ot_video_frame_info),
        &vgs_frame_info, sizeof(ot_video_frame_info)) != EOK) {
        printf("memcpy_s img_out failed\n");
        goto err_exit;
    }

    if (ss_mpi_vgs_add_scale_task(h_handle, &vgs_task_attr, OT_VGS_SCALE_COEF_NORM) != TD_SUCCESS) {
        printf("ss_mpi_vgs_add_scale_task failed\n");
        goto err_exit;
    }

    if (ss_mpi_vgs_end_job(h_handle) != TD_SUCCESS) {
        printf("ss_mpi_vgs_end_job failed\n");
        goto err_exit;
    }

    if (memcpy_s(src_frame_info, sizeof(ot_video_frame_info),
        &vgs_task_attr.img_out, sizeof(ot_video_frame_info)) != EOK) {
        printf("memcpy_s frame_in failed\n");
        goto err_exit;
    }

    dump_mem->vb_pool = OT_VB_INVALID_POOL_ID;
    if (g_pool != OT_VB_INVALID_POOL_ID) {
        ss_mpi_vb_destroy_pool(g_pool);
        g_pool = OT_VB_INVALID_POOL_ID;
    }

    h_handle = -1;
    return TD_SUCCESS;
err_exit:
    ss_mpi_vgs_cancel_job(h_handle);
    h_handle = -1;
    return TD_FAILURE;
}

static td_s32 sample_mcf_do_calibration(ot_video_frame_info *mono_frame_info, ot_video_frame_info *color_frame_info,
    ot_mcf_feature_info *feature_info, ot_mcf_calibration *cal_info, td_u32 blk_size)
{
    td_s32 ret;
    ot_mcf_calibration_mode mode = OT_MCF_CALIBRATION_AFFINE;

    ret = sample_mcf_sys_mmap_frame_virt_addr(blk_size, mono_frame_info, color_frame_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    mono_frame_info->video_frame.supplement.misc_info_virt_addr = TD_NULL;
    color_frame_info->video_frame.supplement.misc_info_virt_addr = TD_NULL;
    ret = ss_mpi_mcf_calibration(&mono_frame_info->video_frame, &color_frame_info->video_frame, mode, feature_info,
        cal_info);
    if (ret != TD_SUCCESS) {
        sample_print("mcf calibration failed, ret:0x%x\n", ret);
        goto exit;
    }
    return TD_SUCCESS;

exit:
    sample_mcf_sys_munmap_frame_virt_addr(blk_size, mono_frame_info, color_frame_info);
    return TD_FAILURE;
}

static td_void sample_mcf_restore(sample_mcf_dump_mem *dump_mem)
{
    if (dump_mem->vb_blk != 0) {
        ss_mpi_vb_release_blk(dump_mem->vb_blk);
    }
    if (g_pool != OT_VB_INVALID_POOL_ID) {
        ss_mpi_vb_destroy_pool(g_pool);
        g_pool = OT_VB_INVALID_POOL_ID;
    }
}

static td_s32 sample_mcf_do_calibration_of_vi_chn(ot_mcf_grp_attr *mcf_grp_attr, ot_vi_chn vi_chn,
    ot_mcf_calibration *cal_info)
{
    td_s32 ret;
    ot_video_frame_info mono_frame_info, color_frame_info, scale_frame_info;
    ot_mcf_feature_info feature_info;
    ot_vb_calc_cfg vb_calc_cfg = {0};
    sample_mcf_dump_mem dump_mem = {0};

    (td_void)memset_s(&mono_frame_info, sizeof(ot_video_frame_info), 0, sizeof(ot_video_frame_info));
    (td_void)memset_s(&color_frame_info, sizeof(ot_video_frame_info), 0, sizeof(ot_video_frame_info));

    ret = sample_mcf_get_vi_chn_frame(mcf_grp_attr, vi_chn, &mono_frame_info, &color_frame_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (mcf_grp_attr->mono_pipe_attr.height != mcf_grp_attr->color_pipe_attr.height ||
        mcf_grp_attr->mono_pipe_attr.width != mcf_grp_attr->color_pipe_attr.width) {
        (td_void)memcpy_s(&scale_frame_info, sizeof(ot_video_frame_info),
                          &color_frame_info, sizeof(ot_video_frame_info));
        ret = sample_mcf_send_vgs_of_vi_chn(&scale_frame_info, &mono_frame_info, &vb_calc_cfg, &dump_mem);
        if (ret != TD_SUCCESS) {
            sample_print("mcf send vgs failed, ret:0x%x\n", ret);
            goto exit;
        }
        ret = sample_mcf_do_calibration(&mono_frame_info, &scale_frame_info, &feature_info, cal_info,
            vb_calc_cfg.vb_size);
        if (ret != TD_SUCCESS) {
            sample_print("mcf calibration failed, ret:0x%x\n", ret);
            goto exit;
        }
    } else {
        /* 2: double page */
        vb_calc_cfg.vb_size = mono_frame_info.video_frame.stride[0] * mono_frame_info.video_frame.height * 2;
        ret = sample_mcf_do_calibration(&mono_frame_info, &color_frame_info, &feature_info, cal_info,
            vb_calc_cfg.vb_size);
        if (ret != TD_SUCCESS) {
            sample_print("mcf calibration failed, ret:0x%x\n", ret);
            goto exit;
        }
    }

    sample_mcf_show_cal_info(&feature_info, cal_info);

exit:
    sample_mcf_release_vi_chn(mcf_grp_attr, vi_chn, &mono_frame_info, &color_frame_info);
    sample_mcf_restore(&dump_mem);
    return ret;
}

td_s32 sample_mcf_calibrate_online(ot_mcf_grp mcf_grp, ot_mcf_grp_attr *mcf_grp_attr, ot_mcf_crop_info *grp_crop,
    ot_fov_attr *fov_correction_attr)
{
    td_s32 ret;
    ot_mcf_calibration cal_info;
    const ot_vi_chn vi_chn = 0;
    td_u32 orig_depth[OT_MCF_PIPE_NUM] = { 0 };

    ret = sample_mcf_change_vi_chn_frame_depth(vi_chn, orig_depth, OT_MCF_PIPE_NUM);
    if (ret != TD_SUCCESS) {
        sample_print("set vi chn attr depth failed with %#x!\n", ret);
        goto exit0;
    }

    ret = sample_mcf_do_calibration_of_vi_chn(mcf_grp_attr, vi_chn, &cal_info);
    if (ret != TD_SUCCESS) {
        sample_print("mcf do calibration of vi chn failed, ret:0x%x\n", ret);
        goto exit1;
    }

    /* cal pmf info */
    if (memcpy_s(&fov_correction_attr->fov_coef[0], sizeof(td_s64) * OT_GDC_COMMON_COEF_NUM,
        &cal_info.correct_coef[0], sizeof(td_s64) * OT_GDC_COMMON_COEF_NUM) != EOK) {
        sample_print("memcpy_s failed.\n");
        goto exit1;
    }

    /* cal crop info */
    grp_crop->enable = TD_TRUE;
    grp_crop->crop_mode = OT_COORD_ABS;
    if (memcpy_s(&grp_crop->crop_rect, sizeof(ot_rect), &cal_info.region, sizeof(ot_rect)) != EOK) {
        sample_print("memcpy_s failed.\n");
        goto exit1;
    }

exit1:
    sample_mcf_restore_vi_chn_frame_default_depth(vi_chn, orig_depth, OT_MCF_PIPE_NUM);
exit0:
    return ret;
}
