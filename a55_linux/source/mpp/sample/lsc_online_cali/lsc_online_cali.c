/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <unistd.h>
#ifndef __LITEOS__
#include <poll.h>
#endif
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "sample_comm.h"

#define DUMP_RAW_AND_SAVE_LSC 1
#define MAX_FRM_CNT 25
#define MAX_FRM_WIDTH 8192
#define PIPE_4 4

static volatile sig_atomic_t g_lsc_sig_flag = 0;

typedef struct {
    sample_vi_cfg vi_config;
    sample_vo_cfg vo_config;
    ot_vo_dev vo_dev;
    ot_vo_chn vo_chn;
    ot_vi_pipe vi_pipe;
    ot_vi_chn vi_chn;
} lsc_cali_prev;

typedef enum {
    ISP_SENSOR_8BIT  = 8,
    ISP_SENSOR_10BIT = 10,
    ISP_SENSOR_12BIT = 12,
    ISP_SENSOR_14BIT = 14,
    ISP_SENSOR_16BIT = 16,
    ISP_SENSOR_32BIT = 32,
    ISP_SENSOR_BUTT
} isp_sensor_bit_width;

static td_void lsc_getchar()
{
    if (g_lsc_sig_flag == 1) {
        return;
    }

    (td_void)getchar();

    if (g_lsc_sig_flag == 1) {
        return;
    }
}

static td_void sample_vi_get_default_vb_config(ot_size *size, ot_vb_cfg *vb_cfg)
{
    ot_vb_calc_cfg calc_cfg;
    ot_pic_buf_attr buf_attr;

    (td_void)memset_s(vb_cfg, sizeof(ot_vb_cfg), 0, sizeof(ot_vb_cfg));
    vb_cfg->max_pool_cnt = 128; /* max pool cnt if 128 */

    buf_attr.width = size->width;
    buf_attr.height = size->height;
    buf_attr.align = OT_DEFAULT_ALIGN;
    buf_attr.bit_width = OT_DATA_BIT_WIDTH_8;
    buf_attr.pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422;
    buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    buf_attr.video_format = OT_VIDEO_FORMAT_LINEAR;
    ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);

    vb_cfg->common_pool[0].blk_size = calc_cfg.vb_size;
    vb_cfg->common_pool[0].blk_cnt = 30;  /* block count 30 */
}

static td_s32 sample_vio_sys_init(ot_vi_vpss_mode_type mode_type, ot_vi_aiisp_mode aiisp_mode)
{
    td_s32 ret;
    ot_size size;
    ot_vb_cfg vb_cfg;
    td_u32 supplement_config;

    sample_comm_vi_get_size_by_sns_type(SENSOR0_TYPE, &size);
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

td_s32 lsc_sample_vio_start_vi_vo(sample_vi_cfg *vi_config, sample_vo_cfg *vo_config)
{
    td_s32  ret;

    ret = sample_comm_vi_start_vi(vi_config);
    if (TD_SUCCESS != ret) {
        sample_print("start vi failed!\n");
        return ret;
    }

    ret = sample_comm_vo_start_vo(vo_config);
    if (TD_SUCCESS != ret) {
        sample_print("SAMPLE_VIO start VO failed with %#x!\n", ret);
        goto EXIT;
    }

    return ret;

EXIT:
    sample_comm_vi_stop_vi(vi_config);

    return ret;
}

static td_s32 lsc_sample_vio_stop_vi_vo(sample_vi_cfg *vi_config, sample_vo_cfg *vo_config)
{
    sample_comm_vo_stop_vo(vo_config);

    sample_comm_vi_stop_vi(vi_config);

    return TD_SUCCESS;
}

td_s32 sample_lsc_cali_start_prev(lsc_cali_prev *lsc_cali_prev)
{
    td_s32 ret;
    sample_sns_type sns_type;
    ot_size in_size;
    ot_vi_vpss_mode_type mast_pipe_mode = OT_VI_ONLINE_VPSS_OFFLINE;
    ot_vi_aiisp_mode aiisp_mode = OT_VI_AIISP_MODE_DEFAULT;

    lsc_cali_prev->vo_dev = SAMPLE_VO_DEV_DHD0;
    lsc_cali_prev->vo_chn = 0;
    lsc_cali_prev->vi_pipe = 0;
    lsc_cali_prev->vi_chn = 0;

    /* ***********************************************
    step1:  get all sensors information
    ************************************************ */

    /* ***********************************************
    step2:  get  input size
    ************************************************ */
    ret = sample_vio_sys_init(mast_pipe_mode, aiisp_mode);
    if (ret != TD_SUCCESS) {
        goto sys_init_failed;
    }

    sns_type = SENSOR0_TYPE;

    sample_comm_vi_get_size_by_sns_type(sns_type, &in_size);
    sample_comm_vi_get_default_vi_cfg(sns_type, &lsc_cali_prev->vi_config);

    /* ***********************************************
    step4:  init VI and VO
    ************************************************ */
    sample_comm_vo_get_def_config(&lsc_cali_prev->vo_config);

    ret = lsc_sample_vio_start_vi_vo(&lsc_cali_prev->vi_config, &lsc_cali_prev->vo_config);
    if (ret != TD_SUCCESS) {
        sample_print("sample_vio_start_vi_vo failed with %d\n", ret);
        goto EXIT;
    }

    /* ***********************************************
    step5:  bind VI and VO
    ************************************************ */
    ret = sample_comm_vi_bind_vo(lsc_cali_prev->vi_pipe, lsc_cali_prev->vi_chn, lsc_cali_prev->vo_dev,
        lsc_cali_prev->vo_chn);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_vi_bind_vo failed with %#x!\n", ret);
        goto EXIT1;
    }

    return ret;

EXIT1:
    lsc_sample_vio_stop_vi_vo(&lsc_cali_prev->vi_config, &lsc_cali_prev->vo_config);
EXIT:
    sample_comm_sys_exit();
sys_init_failed:
    return ret;
}

td_void sample_lsc_cali_stop_prev(lsc_cali_prev *lsc_cali_prev)
{
    td_s32 ret;

    ret = sample_comm_vi_un_bind_vo(lsc_cali_prev->vi_pipe, lsc_cali_prev->vi_chn, lsc_cali_prev->vo_dev,
        lsc_cali_prev->vo_chn);
    if (ret != TD_SUCCESS) {
        return;
    }

    ret = lsc_sample_vio_stop_vi_vo(&lsc_cali_prev->vi_config, &lsc_cali_prev->vo_config);
    if (ret != TD_SUCCESS) {
        return;
    }

    sample_comm_sys_exit();

    return;
}

static void sample_lsc_usage(void)
{
    printf("\n"
        "*************************************************\n"
        "usage: ./sample_lsc_online_cali [vi_pipe] [scale] \n"
        "vi_pipe: \n"
        "   0:vi_pipe0 ~~ 3:vi_pipe3\n"
        "scale: \n"
        "   scale value to be used to calculate gain(range:[0,7])\n"
        "e.g : ./sample_lsc_online_cali 0 6\n"
        "*************************************************\n"
        "\n");
}

static td_void sample_lsc_handle_sig(td_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        g_lsc_sig_flag = 1;
    }
}

static td_void sample_lsc_register_sig_handler(td_void (*sig_handle)(td_s32))
{
    struct sigaction sa;

    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sig_handle;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, TD_NULL);
    sigaction(SIGTERM, &sa, TD_NULL);
}

static td_s32 lsc_check_pipe_num(ot_vi_bind_pipe *dev_bind_pipe, ot_wdr_mode in_wdr_mode,
    td_u32 pipe_num, td_u32 *dump_pipe_num)
{
    if (dev_bind_pipe->pipe_num < pipe_num) {
        printf("pipe_num(%u) in_wdr_mode(%d) don't match pipe_num:%u!\n",
            dev_bind_pipe->pipe_num, in_wdr_mode, pipe_num);
        return TD_FAILURE;
    }
    *dump_pipe_num = pipe_num;
    return TD_SUCCESS;
}

static td_s32 get_dump_pipe(ot_vi_dev vi_dev, ot_wdr_mode in_wdr_mode, td_u32 *pipe_num, ot_vi_pipe vi_pipe_id[],
    td_u32 vi_max_pipe_num)
{
    td_s32 ret;
    td_u32 dump_pipe_num, i;
    ot_vi_bind_pipe dev_bind_pipe;

    (td_void)memset_s(&dev_bind_pipe, sizeof(dev_bind_pipe), 0, sizeof(dev_bind_pipe));
    ret = ss_mpi_vi_get_bind_by_dev(vi_dev, &dev_bind_pipe);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_vi_get_dev_bind_pipe error 0x%x !\n", ret);
        return ret;
    }

    dump_pipe_num = 0;

    switch (in_wdr_mode) {
        case OT_WDR_MODE_NONE:
        case OT_WDR_MODE_BUILT_IN:
            ret = lsc_check_pipe_num(&dev_bind_pipe, in_wdr_mode, 1, &dump_pipe_num); // 1 pipe_num
            if (ret != TD_SUCCESS) {
                return ret;
            }
            break;

        case OT_WDR_MODE_2To1_LINE:
        case OT_WDR_MODE_2To1_FRAME:
            ret = lsc_check_pipe_num(&dev_bind_pipe, in_wdr_mode, 2, &dump_pipe_num); // 2 pipe_num
            if (ret != TD_SUCCESS) {
                return ret;
            }

            break;

        case OT_WDR_MODE_3To1_LINE:
        case OT_WDR_MODE_3To1_FRAME:
            ret = lsc_check_pipe_num(&dev_bind_pipe, in_wdr_mode, 3, &dump_pipe_num); // 3 pipe_num
            if (ret != TD_SUCCESS) {
                return ret;
            }
            break;

        case OT_WDR_MODE_4To1_LINE:
        case OT_WDR_MODE_4To1_FRAME:
            ret = lsc_check_pipe_num(&dev_bind_pipe, in_wdr_mode, 4, &dump_pipe_num); // 4 pipe_num
            if (ret != TD_SUCCESS) {
                return ret;
            }
            break;

        default:
            printf("in_wdr_mode(%d) error !\n", in_wdr_mode);
            return TD_FAILURE;
    }
    for (i = 0; i < dump_pipe_num && i < vi_max_pipe_num; i++) {
        vi_pipe_id[i] = dev_bind_pipe.pipe_id[i];
    }

    *pipe_num = dump_pipe_num;

    return TD_SUCCESS;
}

static td_u32 pixel_format2_bit_width(ot_pixel_format *pixel_format)
{
    ot_pixel_format en_pixel_format;
    en_pixel_format = *pixel_format;
    td_u32 bit_width;
    switch (en_pixel_format) {
        case OT_PIXEL_FORMAT_RGB_BAYER_8BPP:
            bit_width = ISP_SENSOR_8BIT;
            break;
        case OT_PIXEL_FORMAT_RGB_BAYER_10BPP:
            bit_width = ISP_SENSOR_10BIT;
            break;
        case OT_PIXEL_FORMAT_RGB_BAYER_12BPP:
            bit_width = ISP_SENSOR_12BIT;
            break;
        case OT_PIXEL_FORMAT_RGB_BAYER_14BPP:
            bit_width = ISP_SENSOR_14BIT;
            break;
        case OT_PIXEL_FORMAT_RGB_BAYER_16BPP:
            bit_width = ISP_SENSOR_16BIT;
            break;
        default:
            bit_width = TD_FAILURE;
            break;
    }

    return bit_width;
}

static td_void convert_bit_pixel(td_u8 *data, td_u32 data_num, td_u32 bit_width, td_u16 *out_data)
{
    td_s32 i, tmp_var, out_cnt;
    td_u32 value;
    td_u64 val;
    td_u8 *tmp = data;

    out_cnt = 0;
    switch (bit_width) {
        case ISP_SENSOR_10BIT:
            tmp_var = data_num / 4; /* 4 pixels consist of 5 bytes  */

            for (i = 0; i < tmp_var; i++) { /* byte4 byte3 byte2 byte1 byte0 */
                tmp = data + 5 * i; /* 4 pixels consist of 5 bytes  */
                val = tmp[0] + ((td_u32)tmp[1] << 8) + ((td_u32)tmp[0x2] << 16) + /* left shift 8, 16 */
                    ((td_u32)tmp[0x3] << 24) + ((td_u64)tmp[0x4] << 32); /* left shift 24, 32 */

                out_data[out_cnt++] = val & 0x3ff;
                out_data[out_cnt++] = (val >> 10) & 0x3ff; /* right shift 10 */
                out_data[out_cnt++] = (val >> 20) & 0x3ff; /* right shift 20 */
                out_data[out_cnt++] = (val >> 30) & 0x3ff; /* right shift 30 */
            }
            break;
        case ISP_SENSOR_12BIT:
            tmp_var = data_num / 2; /* 2 pixels consist of 3 bytes  */

            for (i = 0; i < tmp_var; i++) {
                /* byte2 byte1 byte0 */
                tmp = data + 3 * i; /* 2 pixels consist of 3 bytes  */
                value = tmp[0] + (tmp[1] << 8) + (tmp[0x2] << 16); /* left shift 8, 16 */
                out_data[out_cnt++] = value & 0xfff;
                out_data[out_cnt++] = (value >> 12) & 0xfff; /* right shift 12 */
            }
            break;
        case ISP_SENSOR_14BIT:
            tmp_var = data_num / 4; /* 4 pixels consist of 7 bytes  */

            for (i = 0; i < tmp_var; i++) {
                tmp = data + 7 * i; /* 4 pixels consist of 7 bytes  */
                val = tmp[0] + ((td_u32)tmp[1] << 0x8) + ((td_u32)tmp[0x2] << 0x10) + ((td_u32)tmp[0x3] << 0x18) +
                    ((td_u64)tmp[0x4] << 32) + ((td_u64)tmp[0x5] << 40) + ((td_u64)tmp[0x6] << 48); /* lsh 32 40 48 */

                out_data[out_cnt++] = val & 0x3fff;
                out_data[out_cnt++] = (val >> 14) & 0x3fff; /* right shift 14 */
                out_data[out_cnt++] = (val >> 28) & 0x3fff; /* right shift 28 */
                out_data[out_cnt++] = (val >> 42) & 0x3fff; /* right shift 42 */
            }
            break;
        default:   /* 16bit */
            /* 1 pixels consist of 2 bytes */
            tmp_var = data_num;

            for (i = 0; i < tmp_var; i++) { /* byte1 byte0 */
                tmp = data + 2 * i; /* 1 pixels consist of 2 bytes */
                val = tmp[0] + (tmp[1] << 8); /* left shift 8 */
                out_data[out_cnt++] = val & 0xffff;
            }
            break;
    }
}

#if DUMP_RAW_AND_SAVE_LSC
static td_s32 dump_lsc_raw(td_u8 *user_page_addr[2], td_u32 nbit, const ot_video_frame *v_buf, td_u64 size)
{
    td_u8 *data = NULL;
    td_u16 *data_16bit = NULL;
    td_u32 h;
    FILE *pfd = NULL;

    printf("dump raw frame of vi  to file: \n");

    /* open file */
    pfd = fopen("lsc.raw", "wb");
    if (pfd == NULL) {
        printf("open file failed!\n");
        return TD_FAILURE;
    }

    data = user_page_addr[0];
    if (nbit != ISP_SENSOR_8BIT) {
        data_16bit = (td_u16 *)malloc(v_buf->width * 2); /* 2 bytes */
        if (data_16bit == NULL) {
            fprintf(stderr, "alloc memory failed\n");
            ss_mpi_sys_munmap(user_page_addr[0], size);
            user_page_addr[0] = NULL;
            if (pfd != NULL) {
                fclose(pfd);
            }
            return TD_FAILURE;
        }
    }

    /* save Y ---------------------------------------------------------------- */
    (td_void)fprintf(stderr, "saving......dump data......stride[0]: %u, width: %u\n", v_buf->stride[0], v_buf->width);
    (td_void)fflush(stderr);

    for (h = 0; h < v_buf->height; h++) {
        if (nbit == ISP_SENSOR_8BIT) {
            fwrite(data, v_buf->width, 1, pfd);
        } else if (nbit == ISP_SENSOR_16BIT) {
            fwrite(data, v_buf->width, 2, pfd); /* 2 bytes */
            (td_void)fflush(pfd);
        } else {
            convert_bit_pixel(data, v_buf->width, nbit, data_16bit);
            fwrite(data_16bit, v_buf->width, 2, pfd); /* 2 bytes */
        }
        data += v_buf->stride[0];
    }
    (td_void)fflush(pfd);

    (td_void)fprintf(stderr, "done time_ref: %u!\n", v_buf->time_ref);
    (td_void)fflush(stderr);
    if (pfd != NULL) {
        fclose(pfd);
    }

    if (data_16bit != NULL) {
        free(data_16bit);
    }
    return TD_SUCCESS;
}
#endif
static td_void lsc_set_cali_cfg(ot_vi_pipe vi_pipe, ot_isp_mlsc_calibration_cfg *mlsc_cali_cfg)
{
    ot_isp_pub_attr isp_pub_attr;
    ot_isp_black_level_attr black_level;

    ss_mpi_isp_get_pub_attr(vi_pipe, &isp_pub_attr);
    mlsc_cali_cfg->bayer = isp_pub_attr.bayer_format;

    /* default setting without crop, if need to crop, please set the right crop parameters. */
    mlsc_cali_cfg->dst_img_width = mlsc_cali_cfg->img_width;
    mlsc_cali_cfg->dst_img_height = mlsc_cali_cfg->img_height;
    mlsc_cali_cfg->offset_x = 0;
    mlsc_cali_cfg->offset_y = 0;

    ss_mpi_isp_get_black_level_attr(vi_pipe, &black_level);
    mlsc_cali_cfg->blc_offset_r = black_level.manual_attr.black_level[0][0] >> 2; /* 2bit for 14bit to 12bit blc */
    mlsc_cali_cfg->blc_offset_gr = black_level.manual_attr.black_level[0][1] >> 2; /* 2bit for 14bit to 12bit blc */
    mlsc_cali_cfg->blc_offset_gb = black_level.manual_attr.black_level[0][2] >> 2; /* chn 2 */
    mlsc_cali_cfg->blc_offset_b =
        black_level.manual_attr.black_level[0][3] >> 2; /* 2bit for 14bit to 12bit blc,chn 3 */
}

static td_s32 mesh_calibration_proc(ot_vi_pipe vi_pipe, ot_video_frame *v_buf, td_u32 mesh_scale, td_u32 byte_align,
    ot_isp_mesh_shading_table *mlsc_table)
{
    td_u16 *data_16bit = NULL;
    td_u64 phy_addr, size;
    td_u8 *user_page_addr[2]; /* 2 addr */
    td_u8 *data = NULL;
    td_u32 nbit, h;

    td_s32 ret = TD_SUCCESS;
    ot_pixel_format pixel_format = v_buf->pixel_format;
    ot_isp_mlsc_calibration_cfg mlsc_cali_cfg;

    nbit = pixel_format2_bit_width(&pixel_format);
    if (nbit != ISP_SENSOR_10BIT && nbit != ISP_SENSOR_12BIT && nbit != ISP_SENSOR_14BIT && nbit != ISP_SENSOR_16BIT) {
        printf("can't not support %u bits raw, only support 10bits,12bits,14bits,16bits\n", nbit);
        return TD_FAILURE;
    }

    size = (v_buf->stride[0]) * (v_buf->height);
    phy_addr = v_buf->phys_addr[0];

    user_page_addr[0] = (td_u8 *)ss_mpi_sys_mmap(phy_addr, size);
    if (user_page_addr[0] == NULL) {
        return TD_FAILURE;
    }

#if DUMP_RAW_AND_SAVE_LSC
    check_return(dump_lsc_raw(user_page_addr, nbit, v_buf, size), "dump lsc.raw failed!\n");
#endif

    data = user_page_addr[0];
    data_16bit = (td_u16 *)malloc(sizeof(td_u16) * v_buf->width * v_buf->height);
    if (data_16bit == NULL) {
        printf("alloc memory failed\n");
        ret = TD_FAILURE;
        goto fail_exit;
    }

    for (h = 0; h < v_buf->height; h++) {
        convert_bit_pixel(data, v_buf->width, nbit, data_16bit);
        data += v_buf->stride[0];
        data_16bit += v_buf->width;
    }
    data_16bit -= v_buf->width * v_buf->height;

    /* calibration parameter preset */
    mlsc_cali_cfg.raw_bit = nbit;
    mlsc_cali_cfg.mesh_scale = mesh_scale;
    mlsc_cali_cfg.img_width = v_buf->width;
    mlsc_cali_cfg.img_height = v_buf->height;
    lsc_set_cali_cfg(vi_pipe, &mlsc_cali_cfg);

    ret = ss_mpi_isp_mesh_shading_calibration(vi_pipe, data_16bit, &mlsc_cali_cfg, mlsc_table);
    if (data_16bit != NULL) {
        free(data_16bit);
    }
fail_exit:
    ss_mpi_sys_munmap(user_page_addr[0], size);
    user_page_addr[0] = NULL;
    printf("------done!, ret:%d\n", ret);

    return ret;
}

static ot_wdr_mode sample_comm_vi_get_wdr_mode_by_sns_type(sample_sns_type sns_type)
{
    switch (sns_type) {
        default:
            return OT_WDR_MODE_NONE;
    }
}

#if DUMP_RAW_AND_SAVE_LSC
static td_s32 save_lsc_output(ot_isp_mesh_shading_table *isp_mlsc_table)
{
    td_s32 i, j;

    FILE *file = fopen("gain.txt", "wb");
    if (file == TD_NULL) {
        printf("create file fails\n");
        return TD_FAILURE;
    }
    (td_void)fprintf(file, "isp_sharding_table.au32_x_grid_width = ");
    for (i = 0; i < OT_ISP_MLSC_X_HALF_GRID_NUM; i++) {
        (td_void)fprintf(file, "%u,", isp_mlsc_table->x_grid_width[i]);
    }
    (td_void)fprintf(file, "\n");
    (td_void)fprintf(file, "isp_sharding_table.au32_y_grid_height = ");
    for (i = 0; i < OT_ISP_MLSC_Y_HALF_GRID_NUM; i++) {
        (td_void)fprintf(file, "%d,", isp_mlsc_table->y_grid_width[i]);
    }
    (td_void)fprintf(file, "\n");
    (td_void)fprintf(file, "R = \n");
    for (i = 0; i < OT_ISP_LSC_GRID_ROW; i++) {
        for (j = 0; j < OT_ISP_LSC_GRID_COL; j++) {
            (td_void)fprintf(file, "%d,", isp_mlsc_table->lsc_gain_lut.r_gain[i * OT_ISP_LSC_GRID_COL + j]);
        }
        (td_void)fprintf(file, "\n");
    }
    (td_void)fprintf(file, "gr\n");
    for (i = 0; i < OT_ISP_LSC_GRID_ROW; i++) {
        for (j = 0; j < OT_ISP_LSC_GRID_COL; j++) {
            (td_void)fprintf(file, "%d,", isp_mlsc_table->lsc_gain_lut.gr_gain[i * OT_ISP_LSC_GRID_COL + j]);
        }
        (td_void)fprintf(file, "\n");
    }
    (td_void)fprintf(file, "gb\n");
    for (i = 0; i < OT_ISP_LSC_GRID_ROW; i++) {
        for (j = 0; j < OT_ISP_LSC_GRID_COL; j++) {
            (td_void)fprintf(file, "%d,", isp_mlsc_table->lsc_gain_lut.gb_gain[i * OT_ISP_LSC_GRID_COL + j]);
        }
        (td_void)fprintf(file, "\n");
    }
    (td_void)fprintf(file, "B\n");
    for (i = 0; i < OT_ISP_LSC_GRID_ROW; i++) {
        for (j = 0; j < OT_ISP_LSC_GRID_COL; j++) {
            (td_void)fprintf(file, "%d,", isp_mlsc_table->lsc_gain_lut.b_gain[i * OT_ISP_LSC_GRID_COL + j]);
        }
        (td_void)fprintf(file, "\n");
    }
    (td_void)fclose(file);
    return TD_SUCCESS;
}
#endif

static td_s32 lsc_online_cali_proc(ot_vi_pipe vi_pipe, td_u32 mesh_scale, td_u32 cnt, td_u32 byte_align)
{
    td_u32 i, j;
    td_s32 ret;
    const td_s32 milli_sec = 4000; /* 4000 milli_sec */
    td_u32 cap_cnt;
    ot_video_frame_info ast_frame[MAX_FRM_CNT];
    ot_isp_mesh_shading_table isp_mlsc_table;

    /* get VI frame  */
    for (i = 0; i < cnt; i++) {
        if (ss_mpi_vi_get_pipe_frame(vi_pipe, &ast_frame[i], milli_sec) != TD_SUCCESS) {
            printf("get vi pipe %d frame err\n", vi_pipe);
            printf("only get %u frame\n", i);
            break;
        }

        printf("get vi pipe %d frame num %u ok\n", vi_pipe, i);
    }

    cap_cnt = i;

    if (cap_cnt == 0) {
        return TD_FAILURE;
    }

    /* dump file */
    for (j = 0; j < cap_cnt; j++) {
        /* save VI frame to file */
        ret = mesh_calibration_proc(vi_pipe, &ast_frame[j].video_frame, mesh_scale, byte_align, &isp_mlsc_table);
        if (ret != TD_SUCCESS) {
            for (; j < cap_cnt; j++) {
                ss_mpi_vi_release_pipe_frame(vi_pipe, &ast_frame[j]);
            }
            return TD_FAILURE;
        }
        /* release frame after using */
        ss_mpi_vi_release_pipe_frame(vi_pipe, &ast_frame[j]);
    }

#if DUMP_RAW_AND_SAVE_LSC
    ret = save_lsc_output(&isp_mlsc_table);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }
#endif

    return TD_SUCCESS;
}

static td_s32 sample_lsc_prepare_vi(ot_vi_pipe vi_pipe, td_u32 pipe_num, ot_vi_pipe vi_pipe_id[PIPE_4],
    ot_vi_frame_dump_attr *raw_dump_attr, ot_vi_pipe_attr ast_back_up_pipe_attr[])
{
    td_s32 ret;
    td_u32 i;
    ot_vi_frame_dump_attr dump_attr;
    ot_vi_pipe_attr pipe_attr;

    for (i = 0; i < pipe_num; i++) {
        ret = ss_mpi_vi_get_pipe_frame_dump_attr(vi_pipe_id[i], raw_dump_attr);
        if (ret != TD_SUCCESS) {
            printf("get pipe %d dump attr failed!\n", vi_pipe);
            return TD_FAILURE;
        }

        (td_void)memcpy_s(&dump_attr, sizeof(ot_vi_frame_dump_attr), raw_dump_attr, sizeof(ot_vi_frame_dump_attr));
        dump_attr.enable = TD_TRUE;
        dump_attr.depth = 2; /* 2 raw depth */

        ret = ss_mpi_vi_set_pipe_frame_dump_attr(vi_pipe_id[i], &dump_attr);
        if (ret != TD_SUCCESS) {
            printf("set pipe %d dump attr failed!\n", vi_pipe_id[i]);
            return TD_FAILURE;
        }

        ret = ss_mpi_vi_get_pipe_attr(vi_pipe_id[i], &ast_back_up_pipe_attr[i]);
        if (ret != TD_SUCCESS) {
            printf("get pipe %d attr failed!\n", vi_pipe);
            return TD_FAILURE;
        }

        (td_void)memcpy_s(&pipe_attr, sizeof(ot_vi_pipe_attr), &ast_back_up_pipe_attr[i], sizeof(ot_vi_pipe_attr));
        pipe_attr.compress_mode = OT_COMPRESS_MODE_NONE;
        ret = ss_mpi_vi_set_pipe_attr(vi_pipe_id[i], &pipe_attr);
        if (ret != TD_SUCCESS) {
            printf("set pipe %d attr failed!\n", vi_pipe);
            return TD_FAILURE;
        }
    }

    sleep(1);
    printf("--> pipe_num = %u\n", pipe_num);
    return TD_SUCCESS;
}

static td_s32 sample_lsc_run(ot_vi_pipe vi_pipe, td_u32 mesh_scale)
{
    td_s32 ret;
    td_u32 i, byte_align;
    td_u32 pipe_num = 0; /* line_mode -> 1, wdr_mode -> 2~3 */
    ot_vi_pipe vi_pipe_id[PIPE_4] = {0};
    ot_vi_frame_dump_attr raw_dump_attr;
    ot_vi_pipe_attr ast_back_up_pipe_attr[PIPE_4];
    const ot_vi_dev vi_dev = 0;
    ot_vi_dev_attr dev_attr;
    sample_sns_type sns_type = SENSOR0_TYPE;

    byte_align = 1; /* convert to byte align */

    ot_wdr_mode wdr_mode = sample_comm_vi_get_wdr_mode_by_sns_type(sns_type);

    check_return(ss_mpi_vi_get_dev_attr(vi_dev, &dev_attr), "get dev attr failed!\n");
    check_return(get_dump_pipe(vi_dev, wdr_mode, &pipe_num, vi_pipe_id, PIPE_4), "get_dump_pipe failed!\n");

    ret = sample_lsc_prepare_vi(vi_pipe, pipe_num, vi_pipe_id, &raw_dump_attr, ast_back_up_pipe_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (pipe_num == 1) {
        printf("setting parameter ==> mesh_scale =%u\n", mesh_scale);
        lsc_online_cali_proc(vi_pipe, mesh_scale, 1, byte_align);
    } else {
        printf("please check if pipe_num is equal to 1!\n");
        return ret;
    }

    for (i = 0; i < pipe_num; i++) {
        ret = ss_mpi_vi_set_pipe_attr(vi_pipe_id[i], &ast_back_up_pipe_attr[i]);
        if (ret != TD_SUCCESS) {
            printf("set pipe %d attr failed!\n", vi_pipe);
            return ret;
        }

        ret = ss_mpi_vi_set_pipe_frame_dump_attr(vi_pipe_id[i], &raw_dump_attr);
        if (ret != TD_SUCCESS) {
            printf("set pipe %d dump attr failed!\n", vi_pipe);
            return ret;
        }
    }

    return ret;
}

#ifdef __LITEOS__
td_s32 app_main(int argc, char *argv[])
#else
td_s32 main(int argc, char *argv[])
#endif
{
    ot_vi_pipe vi_pipe;
    td_s32 ret;
    td_u32 mesh_scale;
    td_char *ptr;

    printf("\n_notice: this tool can only be used for TESTING, to see more usage, enter: "
        "./sample_lsc_online_cali -h\n\n");

    if ((argc != 3) || (!strncmp(argv[1], "-h", 2))) { /* 3: args, 2: arg num */
        sample_lsc_usage();
        return TD_FAILURE;
    }
    if (strlen(argv[1]) != 1 || !check_digit(argv[1][0]) ||
        strlen(argv[2]) != 1 || !check_digit(argv[2][0])) { /* 2:arg len */
        sample_lsc_usage();
        return TD_FAILURE;
    }

    vi_pipe = (ot_vi_pipe)strtol(argv[1], &ptr, 10);    /* pipe, 10 dec */
    mesh_scale = (td_u32)strtol(argv[2], &ptr, 10); /* 2: scale value of mesh calibration, 10 dec */
    if (mesh_scale > OT_ISP_LSC_MESHSCALE_NUM - 1) {
        printf("can't not support scale mode %u, can choose only from 0~7!\n", mesh_scale);
        sample_lsc_usage();
        return TD_FAILURE;
    }

#ifndef __LITEOS__
        sample_lsc_register_sig_handler(sample_lsc_handle_sig);
#endif

    lsc_cali_prev lsc_cali_prev;
    ret = sample_lsc_cali_start_prev(&lsc_cali_prev);
    if (ret == TD_SUCCESS) {
        sample_print("ISP is now running normally\n");
    } else {
        sample_print("ISP is not running normally!please check it\n");
        return -1;
    }
    printf("input anything to continue....\n");
    lsc_getchar();

    ret = sample_lsc_run(vi_pipe, mesh_scale);
    if (g_lsc_sig_flag != 0) {
        printf("\033[0;32mprogram cannot be terminated using Ctrl + C!\033[0;39m\n");
    }

    if (ret == TD_SUCCESS) {
        printf("\033[0;32mprogram exit normally!\033[0;39m\n");
    }

    printf("input anything to exit....\n");
    lsc_getchar();
    sample_lsc_cali_stop_prev(&lsc_cali_prev);

    return ret;
}
