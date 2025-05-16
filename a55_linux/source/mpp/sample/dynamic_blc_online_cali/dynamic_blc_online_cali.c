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
#include <math.h>
#include "sample_comm.h"
#include "ss_mpi_ae.h"
#include "ot_sns_ctrl.h"

#define CALI_ISO_NUM 3
#define OT_ISP_DYNAMIC_BLC_CALI_MAX_FRAME         30
#define OT_ISP_DYNAMIC_BLC_CALI_MAX_ISO_NUM       16

#define DUMP_RAW_AND_SAVE_DYNAMIC_BLC 0
#define MAX_FRM_CNT 42
#define MAX_FRM_WIDTH 8192
#define PIPE_4 4
#define DYNABLC_ISO_NUM_CALI 16
#define max_2(x, y)     ((x) > (y) ? (x) : (y))

static volatile sig_atomic_t g_sig_flag = 0;

#define VB_LINEAR_RAW_CNT   40
#define VB_YUV_ROUTE_CNT    8

static sample_vo_cfg g_dynablc_vo_cfg = {
    .vo_dev            = SAMPLE_VO_DEV_UHD,
    .vo_intf_type      = OT_VO_INTF_BT1120,
    .intf_sync         = OT_VO_OUT_1080P60,
    .bg_color          = COLOR_RGB_BLACK,
    .pix_format        = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .disp_rect         = {0, 0, 1920, 1080},
    .image_size        = {1920, 1080},
    .vo_part_mode      = OT_VO_PARTITION_MODE_SINGLE,
    .dis_buf_len       = 3, /* 3: def buf len for single */
    .dst_dynamic_range = OT_DYNAMIC_RANGE_SDR8,
    .vo_mode           = VO_MODE_1MUX,
    .compress_mode     = OT_COMPRESS_MODE_NONE,
};

static sample_comm_venc_chn_param g_dynablc_venc_chn_param = {
    .frame_rate           = 30, /* 30 is a number */
    .stats_time           = 2,  /* 1 is a number */
    .gop                  = 60, /* 30 is a number */
    .venc_size            = {1920, 1080},
    .size                 = -1,
    .profile              = 0,
    .is_rcn_ref_share_buf = TD_FALSE,
    .gop_attr             = {
        .gop_mode = OT_VENC_GOP_MODE_NORMAL_P,
        .normal_p = {2},
    },
    .type                 = OT_PT_H265,
    .rc_mode              = SAMPLE_RC_CBR,
};

typedef enum {
    ISP_SENSOR_8BIT  = 8,
    ISP_SENSOR_10BIT = 10,
    ISP_SENSOR_12BIT = 12,
    ISP_SENSOR_14BIT = 14,
    ISP_SENSOR_16BIT = 16,
    ISP_SENSOR_32BIT = 32,
    ISP_SENSOR_BUTT
} isp_sensor_bit_width;

typedef struct {
    td_u64 op_type;
    td_u64 exp_time_op_type;
    td_u64 a_gain_op_type;
    td_u64 d_gain_op_type;
    td_u64 ispd_gain_op_type;
    td_u64 exp_time;
    td_u64 a_gain;
    td_u64 d_gain;
    td_u64 isp_d_gain;
}ae_attr;

typedef struct {
    td_u64 ob_blc;
    td_u64 light_blc;
    td_u64 offset;
    td_u64 offset_final;
    td_u64 iso;
    td_u64 frame_cnt;
    td_u64 bayer_format;
    td_u32 iso_num;
    td_u32 y;
    td_u16 bit_width;
    td_u16 pre_bit_width;
    td_u64 light_blc_sum;
    td_u64 light_pix_cnt;
    ae_attr ae_para;
    ot_rect light_area;
}blc_res;

static td_u64 dynablc_iso_real_cali[DYNABLC_ISO_NUM_CALI] = {
    100, 200, 399, 796, 1588, 3169, 5701, 11403, 22752, 45398, 90579, 180736, 360620, 819200, 1638400, 3276800
};

static const td_u64 dynablc_iso_samcali[DYNABLC_ISO_NUM_CALI] = {
    100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800
};

static td_u8 get_iso_blc_index_cali(td_u32 iso)
{
    td_u8 index;
    for (index = 0; index < DYNABLC_ISO_NUM_CALI - 1; index++) {
        if (iso <= dynablc_iso_samcali[index]) {
            break;
        }
    }
    return index;
}

static td_s32 linear_inter_cali(td_s32 v, td_s32 x0, td_s32 y0, td_s32 x1, td_s32 y1)
{
    td_s32 res;

    if (v <= x0) {
        return y0;
    }
    if (v >= x1) {
        return y1;
    }

    res = (td_s64)(y1 - y0) * (td_s64)(v - x0) / div_0_to_1((x1 - x0) + y0);
    return res;
}

static td_void dynablc_offset_inter_cali(td_u64 iso, blc_res *blc_stat, td_u8 m)
{
    td_u8 iso_index_up, iso_index_low;
    td_u16 iso1, iso2;
    td_s16 offset1, offset2;
    iso_index_up = get_iso_blc_index_cali(iso);
    iso_index_low = max_2((td_s8)iso_index_up - 1, 0);
    iso1          = dynablc_iso_real_cali[iso_index_low];
    iso2          = dynablc_iso_real_cali[iso_index_up];
    offset1       = blc_stat[iso_index_low].offset;
    offset2       = blc_stat[iso_index_up].offset;

    blc_stat[m].offset_final = (td_s16)linear_inter_cali(iso, iso1, offset1, iso2, offset2);
}

/* one frame */

static void usage(void)
{
    printf("\n"
        "*************************************************\n"
        "usage: ./sample_dynamic_blc_online_cali [vi_pipe] [frame_cnt] [iso_num]"
        "[light_stt_area.x] [light_stt_area.y] [light_stt_area.width] [light_stt_area.height]\n"
        "vi_pipe: \n"
        "   0:vi_pipe0 ~~ 3:vi_pipe3\n"
        "frame_cnt: \n"
        "   frame_cnt value to be used to calculate black level(range:(0,30])\n"
        "iso_num: \n"
        "   iso_num value to be used to set again(range:(0,16])\n"
        "light_stt_area.x \n"
        "X coordinates of the light statistical region(range:[0, image_width])\n"
        "light_stt_area.y \n"
        "Y coordinates of the light statistical region(range:[ob_height, image_height])\n"
        "light_stt_area.width \n"
        "width of the light statistical region(range:[0,image_width - light_stt_area.x])\n"
        "light_stt_area.height \n"
        "height of the light statistical region(range:[0, image_height - light_stt_area.y])\n"
        ": \n"
        "e.g : ./sample_dynamic_blc_online_cali 0 30 7 100 100 100 100\n"
        "*************************************************\n"
        "\n");
}

static td_s32 pixel_format2_bit_width(ot_pixel_format *pixel_format)
{
    ot_pixel_format en_pixel_format;
    en_pixel_format = *pixel_format;
    td_s32 bit_width;
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
        case ISP_SENSOR_12BIT:
            tmp_var = data_num / 2; /* 2 pixels consist of 3 bytes */

            for (i = 0; i < tmp_var; i++) {
                /* byte2 byte1 byte0 */
                tmp = data + 3 * i; /* 2 pixels consist of 3 bytes */
                value = tmp[0] + (tmp[1] << 8) + (tmp[0x2] << 16); /* left shift 8, 16 */
                out_data[out_cnt++] = value & 0xfff;
                out_data[out_cnt++] = (value >> 12) & 0xfff; /* right shift 12 */
            }
            break;
        case ISP_SENSOR_14BIT:
            tmp_var = data_num / 4; /* 4 pixels consist of 7 bytes */

            for (i = 0; i < tmp_var; i++) {
                tmp = data + 7 * i; /* 4 pixels consist of 7 bytes */
                val = tmp[0] + ((td_u32)tmp[1] << 0x8) + ((td_u32)tmp[0x2] << 0x10) + ((td_u32)tmp[0x3] << 0x18) +
                    ((td_u64)tmp[0x4] << 32) + ((td_u64)tmp[0x5] << 40) + ((td_u64)tmp[0x6] << 48); /* lsh 32 40 48 */

                out_data[out_cnt++] = val & 0x3fff;
                out_data[out_cnt++] = (val >> 14) & 0x3fff; /* right shift 14 */
                out_data[out_cnt++] = (val >> 28) & 0x3fff; /* right shift 28 */
                out_data[out_cnt++] = (val >> 42) & 0x3fff; /* right shift 42 */
            }
            break;
        default: /* 16bit */
            /* 1 pixels consist of 2 bytes */
            tmp_var = data_num;

            for (i = 0; i < tmp_var; i++) { /* byte1 byte0 */
                tmp = data + 2 * i; /* 1 pixels consist of 2 bytes */
                val = tmp[0] + (tmp[1] << 8); /* left shift 8 */
                out_data[out_cnt++] = val & 0xffff;
            }
            break;
    }

    return;
}

static td_s32 dump_raw_proc(td_u8 *data, const ot_video_frame *v_buf,
    td_u16 *data_16bit, td_u32 nbit, FILE *pfd)
{
    td_s32 ret = TD_SUCCESS;
    td_u32 h;

    for (h = 0; h < v_buf->height; h++) {
        if (nbit == ISP_SENSOR_8BIT) {
            fwrite(data, v_buf->width, 1, pfd);
        } else if (nbit == ISP_SENSOR_16BIT) {
            fwrite(data, v_buf->width, 2, pfd); /* 2 bytes */
            ret = fflush(pfd);
            if (ret != TD_SUCCESS) {
                printf("flush failed!!!\n");
            }
        } else {
            if (data_16bit == NULL) {
                printf("data_16bit is null!\n");
                return TD_FAILURE;
            }
            convert_bit_pixel(data, v_buf->width, nbit, data_16bit);
            fwrite(data_16bit, v_buf->width, 2, pfd); /* 2 bytes */
        }
        data += v_buf->stride[0];
    }
    return ret;
}

static td_s32 dump_dynamic_blc_raw(td_u8 *user_page_addr, td_u32 nbit, const ot_video_frame *v_buf,
    td_u64 size, FILE *pfd)
{
    td_s32 ret;
    td_u16 *data_16bit = NULL;

    printf("dump raw frame of vi  to file: \n");

    /* open file */
    if (pfd == NULL) {
        printf("open file failed!\n");
        return TD_FAILURE;
    }

    if (nbit != ISP_SENSOR_8BIT) {
        data_16bit = (td_u16 *)malloc(v_buf->width * 2); /* 2 bytes */
        if (data_16bit == NULL) {
            printf("alloc memory failed\n");
            return TD_FAILURE;
        }
    }

    /* save Y ---------------------------------------------------------------- */
    printf("saving......dump data......stride[0]: %u, width: %u\n", v_buf->stride[0], v_buf->width);

    ret = dump_raw_proc(user_page_addr, v_buf, data_16bit, nbit, pfd);
    if (ret != TD_SUCCESS) {
        goto end;
    }

    printf("done time_ref: %u!\n", v_buf->time_ref);

end:
    if (data_16bit != NULL) {
        free(data_16bit);
    }
    return ret;
}

static td_void dyna_blc_stat_light(const ot_video_frame *v_buf, ot_isp_black_level_attr *ob_rect,
    td_u16 *data_16bit, blc_res *res)
{
    td_u64 i, j;
    td_u64 x_start, y_start;
    x_start = res->light_area.x;
    y_start = res->light_area.y;

    for (i = res->light_area.y; i < (y_start + res->light_area.height); i++) {
        for (j = res->light_area.x; j < (x_start + res->light_area.width); j++) {
            if ((ob_rect->dynamic_attr.low_threshold < data_16bit[i*v_buf->width + j]) &&
                (data_16bit[i*v_buf->width + j] < (ob_rect->dynamic_attr.high_threshold << 4))) { /* left shift 4bit */
                (res->light_blc_sum) += data_16bit[i*v_buf->width + j];
                (res->light_pix_cnt)++;
            }
        }
    }
}

static td_void get_stat_res(blc_res *res)
{
    res->ob_blc = res->ob_blc / div_0_to_1(res->frame_cnt);
    res->light_blc = (res->light_blc_sum / div_0_to_1(res->light_pix_cnt)) >> 2; /* right shift 2 bit */
    res->offset = res->light_blc - res->ob_blc;
    res->light_blc_sum = 0;
    res->light_pix_cnt = 0;
}

static td_s32 dyna_blc_stat(td_u16 *user_page_addr, const ot_video_frame *v_buf,
    ot_isp_black_level_attr *ob_rect, blc_res *res, int cur_frame_cnt)
{
    dyna_blc_stat_light(v_buf, ob_rect, user_page_addr, res);

    if ((td_u64)cur_frame_cnt == (res->frame_cnt - 1)) {
        get_stat_res(res);
    }

    return TD_SUCCESS;
}

static td_s32 dynamic_blc_calibration_proc(ot_vi_pipe vi_pipe, ot_video_frame *v_buf,
    blc_res *res, td_u8 cur_frame_cnt)
{
    td_u64 phy_addr, size, info_blc;
    td_u16 *user_page_addr = NULL; /* addr */
    td_s32 nbit;
    ot_isp_black_level_attr black_level_attr;
    td_s32 ret;
    ot_pixel_format pixel_format = v_buf->pixel_format;
    ot_isp_inner_state_info inner_state_info;
    ret = ss_mpi_isp_query_inner_state_info(vi_pipe, &inner_state_info);
    if (ret != TD_SUCCESS) {
        printf("get isp_info failed!!!\n");
    }

    ret = ss_mpi_isp_get_black_level_attr(vi_pipe, &black_level_attr);
    if (ret != TD_SUCCESS) {
        printf("get ob_attr failed!!!\n");
    }

    nbit = pixel_format2_bit_width(&pixel_format);
    if (nbit != ISP_SENSOR_10BIT && nbit != ISP_SENSOR_12BIT && nbit != ISP_SENSOR_14BIT && nbit != ISP_SENSOR_16BIT) {
        printf("can't not support %d bits raw, only support 10bits,12bits,14bits,16bits\n", nbit);
        return TD_FAILURE;
    }

    size = (v_buf->stride[0]) * (v_buf->height);
    phy_addr = v_buf->phys_addr[0];

    user_page_addr = (td_u16 *)ss_mpi_sys_mmap(phy_addr, size);
    if (user_page_addr == NULL) {
        printf("user_page_addr == NULL");
        return TD_FAILURE;
    }
    info_blc = (inner_state_info.sns_black_level[0][OT_ISP_CHN_R] + inner_state_info.sns_black_level[0][OT_ISP_CHN_GR]
                + inner_state_info.sns_black_level[0][OT_ISP_CHN_GB]
                + inner_state_info.sns_black_level[0][OT_ISP_CHN_B]) >> 2; /* right shift 2 bits */
    res->ob_blc += info_blc;
    res->bit_width = nbit;
    ret = dyna_blc_stat(user_page_addr, v_buf, &black_level_attr, res, cur_frame_cnt);
    if (ret != TD_SUCCESS) {
        printf("calculate offset failed!\n");
        ss_mpi_sys_munmap(user_page_addr, size);
        return ret;
    }

    ss_mpi_sys_munmap(user_page_addr, size);
    return TD_SUCCESS;
}

static td_s32 save_dynamic_blc_output(ot_isp_black_level_attr *blc_res)
{
    td_s32 i;
    td_s32 ret;

    FILE *file = fopen("dynamic_blc_offset.txt", "wb");
    if (file == TD_NULL) {
        printf("create file fails\n");
        return TD_FAILURE;
    }
    ret = fprintf(file, "dynamic blc offset = ");
    if (ret == TD_FAILURE) {
        printf("print failed!!!\n");
    }

    for (i = 0; i < DYNABLC_ISO_NUM_CALI; i++) {
        ret = fprintf(file, "%d,", blc_res->dynamic_attr.offset[i]);
        if (ret == TD_FAILURE) {
            printf("print failed!!!\n");
        }
    }
    ret = fprintf(file, "\n");
    if (ret == TD_FAILURE) {
        printf("print failed!!!\n");
    }

    ret = fprintf(file, "dynamic blc cali black level = ");
    if (ret == TD_FAILURE) {
        printf("print failed!!!\n");
    }

    for (i = 0; i < DYNABLC_ISO_NUM_CALI; i++) {
        ret = fprintf(file, "%d,", blc_res->dynamic_attr.calibration_black_level[i]);
        if (ret == TD_FAILURE) {
            printf("print failed!!!\n");
        }
    }

    ret = fclose(file);
    if (ret != TD_SUCCESS) {
        printf("close failed!!!\n");
    }

    return TD_SUCCESS;
}

static td_void blc_stat_init(blc_res *blc_stat, td_u32 iso_num, td_u32 frame_cnt, ot_rect *light_area)
{
    td_u32 i;
    for (i = 0; i < DYNABLC_ISO_NUM_CALI; i++) {
        blc_stat[i].iso = 100;      /* 100 is init iso */
        blc_stat[i].frame_cnt = frame_cnt;
        blc_stat[i].light_blc = 0;
        blc_stat[i].ob_blc = 0;
        blc_stat[i].offset = 0;
        blc_stat[i].offset_final = 0;
        blc_stat[i].bayer_format = 21; /* 21 bayer format */
        blc_stat[i].iso_num = iso_num;
        blc_stat[i].y = 0;
        blc_stat[i].bit_width = 0;
        blc_stat[i].pre_bit_width = 0;
        blc_stat[i].ae_para.op_type = 0;
        blc_stat[i].ae_para.exp_time_op_type = 0;
        blc_stat[i].ae_para.a_gain_op_type = 0;
        blc_stat[i].ae_para.d_gain_op_type = 0;
        blc_stat[i].ae_para.ispd_gain_op_type = 0;
        blc_stat[i].ae_para.exp_time = 0;
        blc_stat[i].ae_para.a_gain = 0;
        blc_stat[i].ae_para.d_gain = 0;
        blc_stat[i].ae_para.isp_d_gain = 0;
        blc_stat[i].light_blc_sum = 0;
        blc_stat[i].light_pix_cnt = 0;
        blc_stat[i].light_area.x = light_area->x;
        blc_stat[i].light_area.y = light_area->y;
        blc_stat[i].light_area.width  = light_area->width;
        blc_stat[i].light_area.height = light_area->height;
    }
}

static td_void cali_change_pub_attr(ot_vi_pipe vi_pipe, blc_res *blc_stat, td_u32 iso_num)
{
    td_s32 ret;
    ot_isp_pub_attr pub_attr;

    ret = ss_mpi_isp_get_pub_attr(vi_pipe, &pub_attr);
    if (ret != TD_SUCCESS) {
        printf("get pub_attr failed!!!\n");
    }

    blc_stat[iso_num].y = pub_attr.wnd_rect.y;
    printf("%s %d blc_stat[%u].y = %u\n", __FUNCTION__, __LINE__, iso_num, blc_stat[iso_num].y);
    pub_attr.wnd_rect.y = 0;

    ret = ss_mpi_isp_set_pub_attr(vi_pipe, &pub_attr);
    if (ret != TD_SUCCESS) {
        printf("set pub_attr failed!!!\n");
    }
}

static td_void save_ori_ae_attr(blc_res *blc_stat, ot_isp_exposure_attr *exp_attr)
{
    blc_stat[0].ae_para.op_type = exp_attr->op_type;
    blc_stat[0].ae_para.exp_time_op_type = exp_attr->manual_attr.exp_time_op_type;
    blc_stat[0].ae_para.a_gain_op_type = exp_attr->manual_attr.a_gain_op_type;
    blc_stat[0].ae_para.d_gain_op_type = exp_attr->manual_attr.d_gain_op_type;
    blc_stat[0].ae_para.ispd_gain_op_type = exp_attr->manual_attr.ispd_gain_op_type;
    blc_stat[0].ae_para.exp_time = exp_attr->manual_attr.exp_time;
    blc_stat[0].ae_para.a_gain = exp_attr->manual_attr.a_gain;
    blc_stat[0].ae_para.d_gain = exp_attr->manual_attr.d_gain;
    blc_stat[0].ae_para.isp_d_gain = exp_attr->manual_attr.isp_d_gain;
}

static td_void set_gain_acor_sensor(ot_isp_exposure_attr *exp_attr,  td_s32 k)
{
    sample_sns_type sns_type = SENSOR0_TYPE;
    isp_err_trace("sns_type = %d\n", sns_type);

    switch (sns_type) {
        case SONY_IMX347_SLAVE_MIPI_4M_30FPS_12BIT:
            exp_attr->manual_attr.a_gain = 1024 * pow(2, k); /* 1024 means gain multiplied by one */
            exp_attr->manual_attr.d_gain = 1024; /* 1024 means gain multiplied by one */
            break;
        case OV_OS08A20_MIPI_8M_30FPS_12BIT:
            if (k < 4) { /* 4 is iso num */
                exp_attr->manual_attr.a_gain = 1024 * pow(2, k); /* 1024 means gain multiplied by one */
                exp_attr->manual_attr.d_gain = 1024; /* 1024 means gain multiplied by one */
            } else {
                exp_attr->manual_attr.d_gain = 1024 * pow(2, k); /* 1024 means gain multiplied by one */
            }
            break;
        default:
            break;
    }

    return;
}

static td_void cali_change_exp_attr(ot_vi_pipe vi_pipe, blc_res *blc_stat, td_s32 k)
{
    td_s32 ret;
    ot_isp_exposure_attr   exp_attr;

    ret = ss_mpi_isp_get_exposure_attr(vi_pipe, &exp_attr);
    if (ret != TD_SUCCESS) {
        printf("get exp_attr failed!!!\n");
    }
    if (k == 0) {
        save_ori_ae_attr(blc_stat, &exp_attr);
    }
    exp_attr.op_type        = OT_OP_MODE_MANUAL;
    exp_attr.manual_attr.exp_time_op_type = OT_OP_MODE_MANUAL;
    exp_attr.manual_attr.a_gain_op_type = OT_OP_MODE_MANUAL;
    exp_attr.manual_attr.d_gain_op_type = OT_OP_MODE_MANUAL;
    exp_attr.manual_attr.ispd_gain_op_type = OT_OP_MODE_MANUAL;
    set_gain_acor_sensor(&exp_attr, k);
    isp_err_trace("exp_attr.manual_attr.a_gain = %u\n", exp_attr.manual_attr.a_gain);
    exp_attr.manual_attr.isp_d_gain = 1024; /* 1024 means gain multiplied by one */
    ret = ss_mpi_isp_set_exposure_attr(vi_pipe, &exp_attr);
    if (ret != TD_SUCCESS) {
        printf("set exp_attr failed!!!\n");
    }
}

static td_void release_pipe_frame(ot_vi_pipe vi_pipe, ot_video_frame_info *ast_frame, td_s32 num)
{
    td_s32 ret;
    for (; num >= 0; num--) {
        ret = ss_mpi_vi_release_pipe_frame(vi_pipe, &ast_frame[num]);
        if (ret != TD_SUCCESS) {
            printf("release frame error\n");
            break;
        }
    }
}

static td_s32 get_vi_frame(ot_vi_pipe vi_pipe, ot_video_frame_info *ast_frame, td_u32 frame_cnt)
{
    td_u32 i;
    const td_s32 milli_sec = 4000; /* 4000 milli_sec */
    sleep(1);
    for (i = 0; i < frame_cnt; i++) {
        if (ss_mpi_vi_get_pipe_frame(vi_pipe, &ast_frame[i], milli_sec) != TD_SUCCESS) {
            printf("get vi pipe %d frame err\n", vi_pipe);
            printf("only get %u frame\n", i);
            printf("video buffer is not enought!!!\n");
            release_pipe_frame(vi_pipe, ast_frame, i);
            return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

static td_void get_exp_info(ot_vi_pipe vi_pipe, blc_res *blc_stat, td_s32 k)
{
    td_s32 ret;
    ot_isp_exp_info exp_info;

    ret = ss_mpi_isp_query_exposure_info(vi_pipe, &exp_info);
    if (ret != TD_SUCCESS) {
        printf("get exp_info failed!!!\n");
    }

    blc_stat[k].iso = exp_info.iso;
}

static td_void clear_pfd_mem(FILE *pfd)
{
    td_s32 ret;
    ret = fflush(pfd);
    if (ret != TD_SUCCESS) {
        printf("flush failed!!!\n");
    }

    if (pfd != NULL) {
        ret = fclose(pfd);
        if (ret != TD_SUCCESS) {
            printf("close failed!!!\n");
        }
    }
}

static td_void dyna_close_pfd(FILE *pfd)
{
    td_s32 ret;
    if (pfd != NULL) {
        ret = fclose(pfd);
        if (ret != TD_SUCCESS) {
            printf("close failed!!!\n");
        }
    }
}

static td_s32 pass_res_to_mpi(ot_vi_pipe vi_pipe, ot_isp_black_level_attr *black_level_attr,
    blc_res *blc_stat)
{
    td_u32 i;
    td_s32 nbit;
    td_s32 ret;

    ot_vi_pipe_attr pipe_attr;

    ret = ss_mpi_vi_get_pipe_attr(vi_pipe, &pipe_attr);
    if (ret != TD_SUCCESS) {
        printf("get pipe %d attr failed!\n", vi_pipe);
        return TD_FAILURE;
    }
    printf("pipe_attr.pixel_format = %d\n", pipe_attr.pixel_format);

    nbit = pixel_format2_bit_width(&pipe_attr.pixel_format);
    printf("nbit = %d\n", nbit);

    ret = ss_mpi_isp_get_black_level_attr(vi_pipe, black_level_attr);
    if (ret != TD_SUCCESS) {
        printf("get ob_attr failed!!!\n");
    }

    for (i = 0; i < blc_stat[0].iso_num; i++) {
        dynablc_iso_real_cali[i] = blc_stat[i].iso;
    }

    for (i = 0; i < blc_stat[0].iso_num; i++) {
        dynablc_offset_inter_cali(dynablc_iso_samcali[i], blc_stat, i);

        black_level_attr->dynamic_attr.offset[i] = blc_stat[i].offset;
        printf("black_level_attr->dynamic_attr.offset[%u] = %d\n", i, black_level_attr->dynamic_attr.offset[i]);

        black_level_attr->dynamic_attr.calibration_black_level[i] = blc_stat[i].ob_blc;
        printf("black_level_attr->dynamic_attr.calibration_black_level[%u] = %u\n", i,
            black_level_attr->dynamic_attr.calibration_black_level[i]);
    }

    ret = ss_mpi_isp_set_black_level_attr(vi_pipe, black_level_attr);
    if (ret != TD_SUCCESS) {
        printf("set ob_attr failed!!!\n");
    }

    ret = ss_mpi_isp_get_black_level_attr(vi_pipe, black_level_attr);
    if (ret != TD_SUCCESS) {
        printf("set ob_attr failed!!!\n");
    }

    ret = save_dynamic_blc_output(black_level_attr);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return ret;
}

static td_s32 save_raw_to_file(ot_video_frame_info *ast_frame, blc_res *blc_stat, td_u32 k)
{
#if DUMP_RAW_AND_SAVE_DYNAMIC_BLC == 0
    return TD_SUCCESS;
#endif
    if (k != (blc_stat[0].iso_num - 1)) {
        return TD_SUCCESS;
    }
    /* dump  */
    td_u64 phy_addr, size;
    td_u8 *user_page_addr = NULL; /* addr */
    td_u32 nbit;
    td_s32 ret;
    td_u32 j;
    FILE *pfd = NULL;
    pfd = fopen("dynamic_blc.raw", "wb");

    for (j = 0; j < blc_stat[0].frame_cnt; j++) {
        ot_pixel_format pixel_format = ast_frame[j].video_frame.pixel_format;

        nbit = pixel_format2_bit_width(&pixel_format);
        if (nbit != ISP_SENSOR_10BIT && nbit != ISP_SENSOR_12BIT && nbit != ISP_SENSOR_14BIT
            && nbit != ISP_SENSOR_16BIT) {
            printf("can't not support %d bits raw, only support 10bits,12bits,14bits,16bits\n", nbit);
            dyna_close_pfd(pfd);
            return TD_FAILURE;
        }

        size = (ast_frame[j].video_frame.stride[0]) * (ast_frame[j].video_frame.height);
        phy_addr = ast_frame[j].video_frame.phys_addr[0];

        user_page_addr = (td_u8 *)ss_mpi_sys_mmap(phy_addr, size);
        if (user_page_addr == NULL) {
            dyna_close_pfd(pfd);
            return TD_FAILURE;
        }

        ret = dump_dynamic_blc_raw(user_page_addr, nbit, &ast_frame[j].video_frame, size, pfd);
        if (ret != TD_SUCCESS) {
            printf("dump dynamic blc.raw failed!\n");
            clear_pfd_mem(pfd);
            goto free_mem;
        }
        ss_mpi_sys_munmap(user_page_addr, size);
    }
    clear_pfd_mem(pfd);
    return TD_SUCCESS;
free_mem:
    ss_mpi_sys_munmap(user_page_addr, size);
    return ret;
}

static td_void release_all_dump_fram(ot_vi_pipe vi_pipe, ot_video_frame_info *frame_buf, td_s32 frame_cnt)
{
    td_s32 i;
    for (i = 0; i < frame_cnt; i++) {
        ss_mpi_vi_release_pipe_frame(vi_pipe, &frame_buf[i]);
    }
}
static td_s32 dynamic_blc_online_cali_proc(ot_vi_pipe vi_pipe, blc_res *blc_stat)
{
    td_u32 j, k;
    td_s32 ret;
    ot_video_frame_info *ast_frame = NULL;

    ast_frame = (ot_video_frame_info*)malloc(blc_stat[0].frame_cnt * sizeof(ot_video_frame_info));
    if (ast_frame == TD_NULL) {
        return TD_FAILURE;
    }
    for (k = 0; k < blc_stat[0].iso_num; k++) {
        cali_change_pub_attr(vi_pipe, blc_stat, k);
        cali_change_exp_attr(vi_pipe, blc_stat, k);

        (td_void)memset_s(ast_frame, blc_stat[0].frame_cnt * sizeof(ot_video_frame_info), 0,
            blc_stat[0].frame_cnt * sizeof(ot_video_frame_info));
        /* get VI frame  */
        ret = get_vi_frame(vi_pipe, ast_frame, blc_stat[0].frame_cnt);
        if (ret != TD_SUCCESS) {
            goto free_mem;
        }
        ret = save_raw_to_file(ast_frame, blc_stat, k);
        if (ret != TD_SUCCESS) {
            release_all_dump_fram(vi_pipe, ast_frame, blc_stat[0].frame_cnt);
            goto free_mem;
        }

        /* calc  */
        for (j = 0; j < blc_stat[0].frame_cnt; j++) {
            /* save VI frame to file */
            ret = dynamic_blc_calibration_proc(vi_pipe, &ast_frame[j].video_frame, &blc_stat[k], j);
            if (ret != TD_SUCCESS) {
                release_all_dump_fram(vi_pipe, ast_frame, blc_stat[0].frame_cnt);
                goto free_mem;
            }

            if (j == (blc_stat[0].frame_cnt - 1)) {
                get_exp_info(vi_pipe, blc_stat, k);
            }
        }
        release_all_dump_fram(vi_pipe, ast_frame, blc_stat[0].frame_cnt);
    }
free_mem:
    free(ast_frame);
    return ret;
}

static td_void dynamic_blc_cali_sensor_cfg(ot_vi_pipe vi_pipe, sample_sns_type sns_type)
{
    ot_isp_sns_blc_clamp sns_blc_clamp;

    sns_blc_clamp.blc_clamp_en = TD_FALSE;

    switch (sns_type) {
        case SONY_IMX347_SLAVE_MIPI_4M_30FPS_12BIT:
            g_sns_imx347_slave_obj.pfn_mirror_flip(vi_pipe, ISP_SNS_MIRROR);
            g_sns_imx347_slave_obj.pfn_set_blc_clamp(vi_pipe, sns_blc_clamp);
            break;
        case OV_OS08A20_MIPI_8M_30FPS_12BIT:
            g_sns_os08a20_obj.pfn_set_blc_clamp(vi_pipe, sns_blc_clamp);
            break;
        default:
            break;
    }

    return;
}

static td_s32 sample_dynamic_blc_prepare_vi(ot_vi_pipe vi_pipe,
    ot_vi_frame_dump_attr *raw_dump_attr, ot_vi_pipe_attr *ast_back_up_pipe_attr, blc_res *blc_stat)
{
    td_s32 ret;
    ot_vi_frame_dump_attr dump_attr;
    ot_vi_pipe_attr pipe_attr;
    sample_sns_type sns_type = SENSOR0_TYPE;

    dynamic_blc_cali_sensor_cfg(vi_pipe, sns_type);

    ret = ss_mpi_vi_get_pipe_frame_dump_attr(vi_pipe, raw_dump_attr);
    if (ret != TD_SUCCESS) {
        printf("get pipe %d dump attr failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    (td_void)memcpy_s(&dump_attr, sizeof(ot_vi_frame_dump_attr), raw_dump_attr, sizeof(ot_vi_frame_dump_attr));
    dump_attr.enable = TD_TRUE;
    dump_attr.depth = 2; /* 2 raw depth */

    ret = ss_mpi_vi_set_pipe_frame_dump_attr(vi_pipe, &dump_attr);
    if (ret != TD_SUCCESS) {
        printf("set pipe %d dump attr failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    ret = ss_mpi_vi_get_pipe_attr(vi_pipe, ast_back_up_pipe_attr);
    if (ret != TD_SUCCESS) {
        printf("get pipe %d attr failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    (td_void)memcpy_s(&pipe_attr, sizeof(ot_vi_pipe_attr), ast_back_up_pipe_attr, sizeof(ot_vi_pipe_attr));
    pipe_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    blc_stat[0].pre_bit_width = pipe_attr.pixel_format;
    pipe_attr.pixel_format = OT_PIXEL_FORMAT_RGB_BAYER_16BPP;
    ret = ss_mpi_vi_set_pipe_attr(vi_pipe, &pipe_attr);
    if (ret != TD_SUCCESS) {
        printf("set pipe %d attr failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void recovery_ae_and_pub_attr(ot_vi_pipe vi_pipe, blc_res *blc_stat)
{
    td_s32 ret;
    ot_isp_pub_attr pub_attr;
    ot_isp_exposure_attr   exp_attr;

    ret = ss_mpi_isp_get_pub_attr(vi_pipe, &pub_attr);
    if (ret != TD_SUCCESS) {
        printf("get pub_attr failed!!!\n");
    }
    printf("blc_stat[0].y = %u\n", blc_stat[0].y);
    pub_attr.wnd_rect.y = blc_stat[0].y;

    ret = ss_mpi_isp_set_pub_attr(vi_pipe, &pub_attr);
    if (ret != TD_SUCCESS) {
        printf("set pub_attr failed!!!\n");
    }

    ret = ss_mpi_isp_get_exposure_attr(vi_pipe, &exp_attr);
    if (ret != TD_SUCCESS) {
        printf("get exp_attr failed!!!\n");
    }
    exp_attr.op_type        = blc_stat[0].ae_para.op_type;
    exp_attr.manual_attr.exp_time_op_type = blc_stat[0].ae_para.exp_time_op_type;
    exp_attr.manual_attr.a_gain_op_type = blc_stat[0].ae_para.a_gain_op_type;
    exp_attr.manual_attr.d_gain_op_type = blc_stat[0].ae_para.d_gain_op_type;
    exp_attr.manual_attr.ispd_gain_op_type = blc_stat[0].ae_para.ispd_gain_op_type;
    exp_attr.manual_attr.exp_time = blc_stat[0].ae_para.exp_time;
    exp_attr.manual_attr.a_gain = blc_stat[0].ae_para.a_gain;
    exp_attr.manual_attr.d_gain = blc_stat[0].ae_para.d_gain;
    exp_attr.manual_attr.isp_d_gain = blc_stat[0].ae_para.isp_d_gain;
    ret = ss_mpi_isp_set_exposure_attr(vi_pipe, &exp_attr);
    if (ret != TD_SUCCESS) {
        printf("set exp_attr failed!!!\n");
    }
}

static td_s32 recovery_attr(ot_vi_pipe vi_pipe,
    ot_vi_frame_dump_attr *raw_dump_attr, ot_vi_pipe_attr *ast_back_up_pipe_attr, blc_res *blc_stat)
{
    td_u32 ret;
    ot_vi_pipe_attr pipe_attr;

    ret = ss_mpi_vi_set_pipe_attr(vi_pipe, ast_back_up_pipe_attr);
    if (ret != TD_SUCCESS) {
        printf("set pipe %d attr failed!\n", vi_pipe);
        return ret;
    }

    ret = ss_mpi_vi_set_pipe_frame_dump_attr(vi_pipe, raw_dump_attr);
    if (ret != TD_SUCCESS) {
        printf("set pipe %d dump attr failed!\n", vi_pipe);
        return ret;
    }

    (td_void)memcpy_s(&pipe_attr, sizeof(ot_vi_pipe_attr), ast_back_up_pipe_attr, sizeof(ot_vi_pipe_attr));
    pipe_attr.pixel_format = blc_stat[0].pre_bit_width;
    ret = ss_mpi_vi_set_pipe_attr(vi_pipe, &pipe_attr);
    if (ret != TD_SUCCESS) {
        printf("set pipe %d attr failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    recovery_ae_and_pub_attr(vi_pipe, blc_stat);
    return TD_SUCCESS;
}

static td_s32 sample_dynamic_blc_run(ot_vi_pipe vi_pipe, td_u32 frame_cnt, td_u32 iso_num, ot_rect *light_area)
{
    td_s32 ret;
    ot_isp_black_level_attr black_level_attr;
    ot_vi_frame_dump_attr raw_dump_attr;
    ot_vi_pipe_attr ast_back_up_pipe_attr;
    blc_res *blc_stat = TD_NULL;

    printf("setting parameter ==> frame_cnt = %u\n", frame_cnt);
    printf("setting parameter ==> iso_num = %u\n", iso_num);
    blc_stat = (blc_res*)malloc(sizeof(blc_res) * DYNABLC_ISO_NUM_CALI);
    if (blc_stat == TD_NULL) {
        return TD_FAILURE;
    }

    blc_stat_init(&blc_stat[0], iso_num, frame_cnt, light_area);

    ret = sample_dynamic_blc_prepare_vi(vi_pipe, &raw_dump_attr, &ast_back_up_pipe_attr, blc_stat);
    if (ret != TD_SUCCESS) {
        goto end;
    }

    ret = dynamic_blc_online_cali_proc(vi_pipe, blc_stat);
    if (ret != TD_SUCCESS) {
        printf("%s %d blc cali err\n", __FUNCTION__, __LINE__);
        goto end;
    }

    ret = pass_res_to_mpi(vi_pipe, &black_level_attr, blc_stat);
    if (ret != TD_SUCCESS) {
        printf("%s %d set mpi para err\n", __FUNCTION__, __LINE__);
        goto end;
    }

    ret = recovery_attr(vi_pipe, &raw_dump_attr, &ast_back_up_pipe_attr, blc_stat);
    if (ret != TD_SUCCESS) {
        goto end;
    }

end:
    free(blc_stat);
    return ret;
}

static td_void sample_dynablc_get_char(td_void)
{
    if (g_sig_flag == 1) {
        return;
    }

    sample_pause();
}

static td_void sample_dynablc_vi_get_default_vb_config(ot_size *size, ot_vb_cfg *vb_cfg,
    td_u32 yuv_cnt, td_u32 raw_cnt)
{
    ot_vb_calc_cfg calc_cfg;
    ot_pic_buf_attr buf_attr;

    (td_void)memset_s(vb_cfg, sizeof(ot_vb_cfg), 0, sizeof(ot_vb_cfg));
    vb_cfg->max_pool_cnt = 128; /* 128 blks */

    /* default YUV pool: SP420 + compress_seg */
    buf_attr.width         = size->width;
    buf_attr.height        = size->height;
    buf_attr.align         = OT_DEFAULT_ALIGN;
    buf_attr.bit_width     = OT_DATA_BIT_WIDTH_8;
    buf_attr.pixel_format  = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    buf_attr.compress_mode = OT_COMPRESS_MODE_SEG;
    ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);
    vb_cfg->common_pool[0].blk_size = calc_cfg.vb_size;
    vb_cfg->common_pool[0].blk_cnt  = yuv_cnt;

    /* default raw pool: raw16bpp + compress_none */
    buf_attr.pixel_format  = OT_PIXEL_FORMAT_RGB_BAYER_16BPP;
    buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);
    vb_cfg->common_pool[1].blk_size = calc_cfg.vb_size;
    vb_cfg->common_pool[1].blk_cnt  = raw_cnt;

    isp_err_trace("raw_cnt = %u\n", raw_cnt);
    isp_err_trace("yuv_cnt = %u\n", yuv_cnt);
}

static td_s32 sample_dynablc_vio_sys_init(ot_vi_vpss_mode_type mode_type, ot_vi_aiisp_mode aiisp_mode,
    td_u32 yuv_cnt, td_u32 raw_cnt)
{
    td_s32 ret;
    ot_size size;
    ot_vb_cfg vb_cfg;
    td_u32 supplement_config;
    sample_sns_type sns_type = SENSOR0_TYPE;

    sample_comm_vi_get_size_by_sns_type(sns_type, &size);
    sample_dynablc_vi_get_default_vb_config(&size, &vb_cfg, yuv_cnt, raw_cnt);

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

static td_s32 sample_dynablc_vio_start_vpss(ot_vpss_grp grp, ot_size *in_size)
{
    td_s32 ret;
    ot_low_delay_info low_delay_info;
    ot_vpss_grp_attr grp_attr;
    sample_vpss_chn_attr vpss_chn_attr = {0};

    sample_comm_vpss_get_default_grp_attr(&grp_attr);
    grp_attr.max_width  = in_size->width;
    grp_attr.max_height = in_size->height;
    sample_comm_vpss_get_default_chn_attr(&vpss_chn_attr.chn_attr[0]);
    vpss_chn_attr.chn_attr[0].width  = in_size->width;
    vpss_chn_attr.chn_attr[0].height = in_size->height;
    vpss_chn_attr.chn_enable[0] = TD_TRUE;
    vpss_chn_attr.chn_array_size = OT_VPSS_MAX_PHYS_CHN_NUM;

    ret = sample_common_vpss_start(grp, &grp_attr, &vpss_chn_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    low_delay_info.enable     = TD_TRUE;
    low_delay_info.line_cnt   = 200; /* 200: lowdelay line */
    low_delay_info.one_buf_en = TD_FALSE;
    ret = ss_mpi_vpss_set_chn_low_delay(grp, 0, &low_delay_info);
    if (ret != TD_SUCCESS) {
        sample_common_vpss_stop(grp, vpss_chn_attr.chn_enable, OT_VPSS_MAX_PHYS_CHN_NUM);
        return ret;
    }

    return TD_SUCCESS;
}


static td_s32 sample_dynablc_vio_start_vo(sample_vo_mode vo_mode)
{
    g_dynablc_vo_cfg.vo_mode = vo_mode;

    return sample_comm_vo_start_vo(&g_dynablc_vo_cfg);
}

static td_s32 sample_dynablc_vio_start_venc(ot_venc_chn venc_chn[], size_t size, td_u32 chn_num)
{
    td_s32 i, ret;
    ot_size in_size;
    sample_sns_type sns_type = SENSOR0_TYPE;
    if (chn_num > size) {
        return TD_FAILURE;
    }

    sample_comm_vi_get_size_by_sns_type(sns_type, &in_size);

    g_dynablc_venc_chn_param.venc_size.width  = in_size.width;
    g_dynablc_venc_chn_param.venc_size.height = in_size.height;
    g_dynablc_venc_chn_param.size = sample_comm_sys_get_pic_enum(&in_size);

    for (i = 0; i < (td_s32)chn_num; i++) {
        ret = sample_comm_venc_start(venc_chn[i], &g_dynablc_venc_chn_param);
        if (ret != TD_SUCCESS) {
            goto exit;
        }
    }

    ret = sample_comm_venc_start_get_stream(venc_chn, chn_num);
    if (ret != TD_SUCCESS) {
        goto exit;
    }

    return TD_SUCCESS;

exit:
    for (i = i - 1; i >= 0; i--) {
        sample_comm_venc_stop(venc_chn[i]);
    }
    return TD_FAILURE;
}

static td_void sample_dynablc_vio_stop_vo(td_void)
{
    sample_comm_vo_stop_vo(&g_dynablc_vo_cfg);
}

static td_s32 sample_dynablc_vio_start_venc_and_vo(ot_vpss_grp vpss_grp[], td_u32 grp_num)
{
    td_u32 i;
    td_s32 ret;
    sample_vo_mode vo_mode = VO_MODE_1MUX;
    const ot_vpss_chn vpss_chn = 0;
    const ot_vo_layer vo_layer = 0;
    ot_vo_chn vo_chn[4] = {0, 1, 2, 3};     /* 4: max chn num, 0/1/2/3 chn id */
    ot_venc_chn venc_chn[4] = {0, 1, 2, 3}; /* 4: max chn num, 0/1/2/3 chn id */

    if (grp_num > 1) {
        vo_mode = VO_MODE_4MUX;
    }

    ret = sample_dynablc_vio_start_vo(vo_mode);
    if (ret != TD_SUCCESS) {
        goto start_vo_failed;
    }

    ret = sample_dynablc_vio_start_venc(venc_chn, sizeof(venc_chn) / sizeof(venc_chn[0]), grp_num);
    if (ret != TD_SUCCESS) {
        goto start_venc_failed;
    }

    for (i = 0; i < grp_num; i++) {
        sample_comm_vpss_bind_vo(vpss_grp[i], vpss_chn, vo_layer, vo_chn[i]);
        sample_comm_vpss_bind_venc(vpss_grp[i], vpss_chn, venc_chn[i]);
    }
    return TD_SUCCESS;

start_venc_failed:
    sample_dynablc_vio_stop_vo();
start_vo_failed:
    return TD_FAILURE;
}

static td_void sample_dynablc_vio_stop_venc(ot_venc_chn venc_chn[], td_u32 chn_num)
{
    td_u32 i;

    sample_comm_venc_stop_get_stream(chn_num);

    for (i = 0; i < chn_num; i++) {
        sample_comm_venc_stop(venc_chn[i]);
    }
}

static td_void sample_dynablc_vio_stop_venc_and_vo(ot_vpss_grp vpss_grp[], td_u32 grp_num)
{
    td_u32 i;
    const ot_vpss_chn vpss_chn = 0;
    const ot_vo_layer vo_layer = 0;
    ot_vo_chn vo_chn[4] = {0, 1, 2, 3};     /* 4: max chn num, 0/1/2/3 chn id */
    ot_venc_chn venc_chn[4] = {0, 1, 2, 3}; /* 4: max chn num, 0/1/2/3 chn id */

    for (i = 0; i < grp_num; i++) {
        sample_comm_vpss_un_bind_vo(vpss_grp[i], vpss_chn, vo_layer, vo_chn[i]);
        sample_comm_vpss_un_bind_venc(vpss_grp[i], vpss_chn, venc_chn[i]);
    }

    sample_dynablc_vio_stop_venc(venc_chn, grp_num);
    sample_dynablc_vio_stop_vo();
}

static td_void sample_dynablc_vio_stop_vpss(ot_vpss_grp grp)
{
    td_bool chn_enable[OT_VPSS_MAX_PHYS_CHN_NUM] = {TD_TRUE, TD_FALSE, TD_FALSE, TD_FALSE};

    sample_common_vpss_stop(grp, chn_enable, OT_VPSS_MAX_PHYS_CHN_NUM);
}

static td_s32 check_light_stt_area(ot_vi_pipe vi_pipe, ot_rect *light_area)
{
    ot_isp_black_level_attr black_level_attr;
    td_s32 ret;

    ret = ss_mpi_isp_get_black_level_attr(vi_pipe, &black_level_attr);
    if (ret != TD_SUCCESS) {
        printf("get ob_attr failed!!!\n");
    }

    sample_sns_type sns_type = SENSOR0_TYPE;
    ot_size in_size;
    sample_comm_vi_get_size_by_sns_type(sns_type, &in_size);
    if (light_area->x > (td_s32)in_size.width) {
        printf("set x coordinates of the light statistical region failed !!!\n");
        return TD_FAILURE;
    }

    if ((light_area->y > (td_s32)in_size.height) ||
        ((td_u32)light_area->y < black_level_attr.dynamic_attr.ob_area.height)) {
        printf("set y of the light region failed, y:(ob_heigt, img_height), light_area->y:%d, ob_area.height:%u\n",
            light_area->y, black_level_attr.dynamic_attr.ob_area.height);
        return TD_FAILURE;
    }

    if ((light_area->x + light_area->width) > in_size.width) {
        printf("set width of the light statistical region failed !!!\n");
        return TD_FAILURE;
    }

    if ((light_area->y + light_area->height) > in_size.height) {
        printf("set height of the light statistical region failed !!!\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 sample_dynablc_cali(ot_vi_pipe vi_pipe, td_u32 frame_cnt, td_u32 iso_num, ot_rect *light_area)
{
    td_s32 ret = TD_SUCCESS;

    if (g_sig_flag == 0) {
        printf("\033[0;32mprogram cannot be terminated using Ctrl + C!\033[0;39m\n");
    }

    ret = check_light_stt_area(vi_pipe, light_area);
    if (ret != TD_SUCCESS) {
        printf("set attr of the light statistical region failed !!!\n");
        return ret;
    }

    ret = sample_dynamic_blc_run(vi_pipe, frame_cnt, iso_num, light_area);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return ret;
}

static td_s32 sample_dynablc_vio_all_mode(ot_vi_pipe vi_pipe, td_u32 frame_cnt, td_u32 iso_num, ot_rect *light_area)
{
    td_s32 ret;
    td_u32 yuv_cnt, raw_cnt;
    ot_vi_vpss_mode_type mode_type = OT_VI_OFFLINE_VPSS_OFFLINE;
    ot_vi_aiisp_mode aiisp_mode = OT_VI_AIISP_MODE_DEFAULT;
    const ot_vi_chn vi_chn = 0;
    ot_vpss_grp vpss_grp[1] = {0};
    const td_u32 grp_num = 1;
    const ot_vpss_chn vpss_chn = 0;
    sample_vi_cfg vi_cfg;
    sample_sns_type sns_type;
    ot_size in_size;

    yuv_cnt = VB_YUV_ROUTE_CNT;
    raw_cnt = VB_LINEAR_RAW_CNT;

    ret = sample_dynablc_vio_sys_init(mode_type, aiisp_mode, yuv_cnt, raw_cnt);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    sns_type = SENSOR0_TYPE;
    sample_comm_vi_get_size_by_sns_type(sns_type, &in_size);
    sample_comm_vi_get_default_vi_cfg(sns_type, &vi_cfg);
    ret = sample_comm_vi_start_vi(&vi_cfg);
    if (ret != TD_SUCCESS) {
        goto start_vi_failed;
    }

    sample_comm_vi_bind_vpss(vi_pipe, vi_chn, vpss_grp[0], vpss_chn);

    ret = sample_dynablc_vio_start_vpss(vpss_grp[0], &in_size);
    if (ret != TD_SUCCESS) {
        goto start_vpss_failed;
    }

    ret = sample_dynablc_vio_start_venc_and_vo(vpss_grp, grp_num);
    if (ret != TD_SUCCESS) {
        goto start_venc_and_vo_failed;
    }

    ret = sample_dynablc_cali(vi_pipe, frame_cnt, iso_num, light_area);
    if (ret != TD_SUCCESS) {
        goto dyncblc_cali_failed;
    }

    sample_dynablc_get_char();

dyncblc_cali_failed:
    sample_dynablc_vio_stop_venc_and_vo(vpss_grp, grp_num);
start_venc_and_vo_failed:
    sample_dynablc_vio_stop_vpss(vpss_grp[0]);
start_vpss_failed:
    sample_comm_vi_un_bind_vpss(vi_pipe, vi_chn, vpss_grp[0], vpss_chn);
    sample_comm_vi_stop_vi(&vi_cfg);
start_vi_failed:
    sample_comm_sys_exit();
    return ret;
}

static td_s32 get_arg_value(char *argv[], ot_vi_pipe *vi_pipe, td_u32 *frame_cnt,
    td_u32 *iso_num, ot_rect *light_stt_area)
{
    const td_s32 base = 10; /* 10:Decimal */
    td_char *end_ptr = TD_NULL;

    errno = 0;
    *vi_pipe = (td_s32)strtol(argv[1], &end_ptr, base);
    if ((errno != 0) || (*end_ptr != '\0')) {
        return TD_FAILURE;
    }

    errno = 0;
    *frame_cnt = (td_u32)strtol(argv[2], &end_ptr, base); /* 2: frame_cnt value of dynamic blc offset calibration */
    if ((errno != 0) || (*end_ptr != '\0')) {
        return TD_FAILURE;
    }

    errno = 0;
    *iso_num = (td_u32)strtol(argv[3], &end_ptr, base); /* 3: iso_num value of dynamic blc offset calibration */
    if ((errno != 0) || (*end_ptr != '\0')) {
        return TD_FAILURE;
    }

    errno = 0;
    light_stt_area->x = (td_s32)strtol(argv[4], &end_ptr, base); /* 4: X coordinates of the light statistical region */
    if ((errno != 0) || (*end_ptr != '\0')) {
        return TD_FAILURE;
    }

    errno = 0;
    light_stt_area->y = (td_s32)strtol(argv[5], &end_ptr, base); /* 5: Y coordinates of the light statistical region */
    if ((errno != 0) || (*end_ptr != '\0')) {
        return TD_FAILURE;
    }

    errno = 0;
    light_stt_area->width = (td_u32)strtol(argv[6], &end_ptr, base); /* 6: width of the light statistical region */
    if ((errno != 0) || (*end_ptr != '\0')) {
        return TD_FAILURE;
    }

    errno = 0;
    light_stt_area->height = (td_u32)strtol(argv[7], &end_ptr, base); /* 7: height of the light statistical region */
    if ((errno != 0) || (*end_ptr != '\0')) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 check_input_value(td_u32 frame_cnt,
    td_u32 iso_num, ot_rect *light_stt_area)
{
    sample_sns_type sns_type = SENSOR0_TYPE;
    ot_size in_size;
    sample_comm_vi_get_size_by_sns_type(sns_type, &in_size);

    if ((frame_cnt > OT_ISP_DYNAMIC_BLC_CALI_MAX_FRAME) || (frame_cnt == 0)) {
        printf("can't not support frame_cnt %u, can choose only from 1~30!\n", frame_cnt);
        return TD_FAILURE;
    }

    if ((iso_num > OT_ISP_DYNAMIC_BLC_CALI_MAX_ISO_NUM) || (iso_num == 0)) {
        printf("can't not support iso_num %u, can choose only from 1~16!\n", iso_num);
        return TD_FAILURE;
    }

    if (light_stt_area->x < 0 || (td_u32)light_stt_area->x > in_size.width) {
        printf("can't not support x %d, can choose only from 0~sensor_width!\n", light_stt_area->x);
        return TD_FAILURE;
    }

    if (light_stt_area->y < 0 || (td_u32)light_stt_area->y > in_size.height) {
        printf("can't not support y %d, can choose only from 0~sensor_height!\n", light_stt_area->y);
        return TD_FAILURE;
    }

    if ((light_stt_area->width > (in_size.width - light_stt_area->x)) || (light_stt_area->width <= 0)) {
        printf("can't not support width %u, can choose only from 1~(image_width - x)!\n", light_stt_area->width);
        return TD_FAILURE;
    }

    if ((light_stt_area->height > (in_size.height - light_stt_area->y)) || (light_stt_area->height <= 0)) {
        printf("can't not support height %u, can choose only from 1~(image_height - y)!\n", light_stt_area->height);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void sample_vio_handle_sig(td_s32 signo)
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

td_s32 main(int argc, char *argv[])
{
    ot_vi_pipe vi_pipe;
    td_s32 ret;
    td_u32 iso_num;
    td_u32 frame_cnt;
    ot_rect light_stt_area;

    printf("\n_notice: this tool can only be used for TEST, to see more usage,"
           "enter: ./sample_dynamic_blc_online_cali -h\n");

    if (argc != 8) { /* 8 args */
        goto print_usage;
    }

    ret = get_arg_value(argv, &vi_pipe, &frame_cnt, &iso_num, &light_stt_area);
    if (ret != TD_SUCCESS) {
        goto print_usage;
    }

    if ((vi_pipe < 0) || (vi_pipe >= OT_ISP_MAX_PHY_PIPE_NUM)) {
        isp_err_trace("err vi_pipe %d!\n", vi_pipe);
        return TD_FAILURE;
    }

    ret = check_input_value(frame_cnt, iso_num, &light_stt_area);
    if (ret != TD_SUCCESS) {
        goto print_usage;
    }

    sample_register_sig_handler(sample_vio_handle_sig);

    ret = sample_dynablc_vio_all_mode(vi_pipe, frame_cnt, iso_num, &light_stt_area);
    if ((ret == TD_SUCCESS) && (g_sig_flag == 0)) {
        printf("\033[0;32mprogram exit normally!\033[0;39m\n");
    } else {
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
        return ret;
    }

    return ret;

print_usage:
    usage();
    return TD_FAILURE;
}

