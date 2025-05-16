/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <math.h>
#include "isp_config.h"
#include "isp_math_utils.h"
#include "isp_intf.h"
#include "ot_mpi_sys_mem.h"
#include "ot_math.h"

#define LSC_MEASURE_Q       1024
#define WINDOWSIZEX         20
#define WINDOWSIZEY         20

#define LSC_MAX_PIXEL_VALUE ((1<<20) - 1)
#define LSC_ARR_LEN         400

#define BPRG    0
#define BPGR    1
#define BPGB    2
#define BPBG    3

typedef struct {
    td_u32 grid_width_step[OT_ISP_LSC_GRID_COL];
    td_u32 grid_height_step[OT_ISP_LSC_GRID_ROW];
} isp_lsc_grid_step;

typedef struct {
    td_u32 r_data[OT_ISP_LSC_GRID_POINTS];
    td_u32 gr_data[OT_ISP_LSC_GRID_POINTS];
    td_u32 gb_data[OT_ISP_LSC_GRID_POINTS];
    td_u32 b_data[OT_ISP_LSC_GRID_POINTS];
} ls_data;

typedef struct {
    td_s32 x;
    td_s32 y;
} ls_location;

static const td_float g_max_gain_array[OT_ISP_LSC_MESHSCALE_NUM] = {
    (td_float)OT_ISP_LSC_MAX_GAIN / (td_float)OT_ISP_LSC_MESHSCALE0_DEF_GAIN, /* 1.9 */
    (td_float)OT_ISP_LSC_MAX_GAIN / (td_float)OT_ISP_LSC_MESHSCALE1_DEF_GAIN, /* 2.8 */
    (td_float)OT_ISP_LSC_MAX_GAIN / (td_float)OT_ISP_LSC_MESHSCALE2_DEF_GAIN, /* 3.7 */
    (td_float)OT_ISP_LSC_MAX_GAIN / (td_float)OT_ISP_LSC_MESHSCALE3_DEF_GAIN, /* 4.6 */
    (td_float)OT_ISP_LSC_MAX_GAIN / (td_float)1024 + 1,                       /* 0.10, 1024 is base gain */
    (td_float)OT_ISP_LSC_MAX_GAIN / (td_float)512 + 1,                        /* 1.9, 512 is base gain */
    (td_float)OT_ISP_LSC_MAX_GAIN / (td_float)256 + 1,                        /* 2.8, 256 is base gain */
    (td_float)OT_ISP_LSC_MAX_GAIN / (td_float)128 + 1,                        /* 3.7, 128 is base gain */
};

static td_u32 get_max_data(const td_u32 *data, int length)
{
    int i;

    td_u32 max_data = 0;
    for (i = 0; i < length; i++) {
        if (data[i] > max_data) {
            max_data = data[i];
        }
    }
    return max_data;
}

static td_u32 get_min_data(const td_u32 *data, int length)
{
    int i;

    td_u32 min_data = LSC_MAX_PIXEL_VALUE;
    for (i = 0; i < length; i++) {
        if (data[i] < min_data) {
            min_data = data[i];
        }
    }
    return min_data;
}

/*
 * [data_enlarge description]
 * @param data        [input] input data
 * @param max_data     [input] input target value
 * @param mesh_scale   [input] input scale value
 * generate gain value at each grid point, uniformed by input mesh_scale
 */
static td_u32 data_enlarge(td_u32 data, td_u32 max_data, td_u32 mesh_scale)
{
    td_u32 ratio;
    td_u32 lsc_q_value;
    if (mesh_scale < 4) { /* calculate mesh_scale under 4 separately */
        lsc_q_value = 1U << (9 - mesh_scale); /* 9 to get base gain for each meshscale */
        ratio = (td_u32)(((td_float)max_data / (td_float)div_0_to_1(data)) * lsc_q_value);
    } else {
        lsc_q_value = 1U << (14 - mesh_scale); /* 14 to get base gain for each meshscale */
        ratio = (td_u32)(((td_float)max_data / (td_float)div_0_to_1(data) - 1) * lsc_q_value);
    }

    return MIN2(ratio, OT_ISP_LSC_MAX_GAIN);
}

/*
 * [sort description]
 * @param array  [input] input array
 * @param num     [input] array length
 * sort the input array from min to max
 */
static td_void sort(td_u32 *array, td_u32 num)
{
    td_u32 i, j;
    td_u32 temp;
    for (i = 0; i < num; i++) {
        for (j = 0; j < (num - 1 - i); j++) {
            if (array[j] > array[j + 1]) {
                temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

static td_u32 get_point_average(const td_u16 *bayer_img, const ot_isp_mlsc_calibration_cfg *lsc_cali_cfg,
    td_s32 blc_offset, td_u8 value, const ls_location *location)
{
    td_s32 height   = (td_s32)lsc_cali_cfg->dst_img_height / 2; /* process by channel, height is divided by 2 */
    td_s32 width    = (td_s32)lsc_cali_cfg->dst_img_width / 2; /* process by channel, width is divided by 2 */
    td_u32 stride = lsc_cali_cfg->dst_img_width;
    td_u8  bayer = lsc_cali_cfg->bayer;
    td_s32 x_start, x_end, y_start, y_end, x, y;

    td_u32 chn_index = (bayer & 0x3) ^ value;
    td_s32 w = chn_index & 0x1;
    td_s32 h = (chn_index >> 1) & 0x1;
    td_u32 coor_index, count;
    td_u32 num = 0;
    td_u32 num_temp = 0;
    td_u32 sum = 0;
    td_u32 data[LSC_ARR_LEN] = {0};

    x_start = location->x - WINDOWSIZEX / 2; /* process by channel, need to divided by 2 */
    x_end   = location->x + WINDOWSIZEX / 2; /* process by channel, need to divided by 2 */
    y_start = location->y - WINDOWSIZEY / 2; /* process by channel, need to divided by 2 */
    y_end   = location->y + WINDOWSIZEY / 2; /* process by channel, need to divided by 2 */

    if (x_start < 0) {
        x_start = 0;
    }
    if (x_end >= width) {
        x_end = width - 1;
    }
    if (y_start < 0) {
        y_start = 0;
    }
    if (y_end >= height) {
        y_end = height - 1;
    }

    for (y = y_start; y < y_end; y++) {
        for (x = x_start; x < x_end; x++) {
            coor_index = 2 * x + 2 * y * stride; /* search in the origin img, multiply by 2 */
            data[num++] = (td_u32)MAX2(0, ((td_s32)((td_u32)(bayer_img[coor_index + w + h * stride]) <<
                (20 - (td_u16)lsc_cali_cfg->raw_bit)) - blc_offset)); /* process in 20 bit */
        }
    }
    sort(data, num);

    for (count = num / 10; count < num * 9 / 10; count++) { /* drop 10% of the bottom data, 9 for 90% of the data */
        sum  += data[count];
        num_temp++;
    }

    if (num_temp) {
        return sum / num_temp;
    } else {
        return 0;
    }
}

/*
 * [get_lsc_data_channel description]
 * @param bayer_img        [input]  input image data
 * @param grid_data        [output] mesh_lsc grid data generated
 * @param lsc_cali_cfg     [input]  mesh LSC calibration configure
 * @param lsc_grid_step_xy [input]  input grid width&height information
 * @param blc_offset       [input]  input BLC value
 * @param value            [input]  indicate channel number: 0-R, 1-gr, 2-gb, 3-B
 */
static td_s32 get_lsc_data_channel(const td_u16 *bayer_img, td_u32 *grid_data,
    const ot_isp_mlsc_calibration_cfg *lsc_cali_cfg, const isp_lsc_grid_step *lsc_grid_step_xy, td_u8 value)
{
    td_s32 true_blc_offset;
    td_u32 i, j;

    ls_location location;
    location.y = 0;

    if (bayer_img == TD_NULL || grid_data == TD_NULL) {
        isp_err_trace("%s: LINE: %d bayer_img / grid_data is TD_NULL pointer !\n", __FUNCTION__, __LINE__);
        return TD_FAILURE;
    }

    switch (value) { /* input BLC is 12 bit, move to 20 bit */
        case BPRG:
            true_blc_offset = (td_s32)(((td_u32)lsc_cali_cfg->blc_offset_r) << 0x8);
            break;

        case BPGR:
            true_blc_offset = (td_s32)(((td_u32)lsc_cali_cfg->blc_offset_gr) << 0x8);
            break;

        case BPGB:
            true_blc_offset = (td_s32)(((td_u32)lsc_cali_cfg->blc_offset_gb) << 0x8);
            break;

        case BPBG:
            true_blc_offset = (td_s32)(((td_u32)lsc_cali_cfg->blc_offset_b) << 0x8);
            break;

        default:
            true_blc_offset = (td_s32)(((td_u32)lsc_cali_cfg->blc_offset_r) << 0x8);
            break;
    }

    for (i = 0; i < OT_ISP_LSC_GRID_ROW; i++) {
        location.y = location.y + (td_s32)lsc_grid_step_xy->grid_height_step[i];
        location.x = 0;
        for (j = 0; j < OT_ISP_LSC_GRID_COL; j++) {
            location.x = location.x + (td_s32)lsc_grid_step_xy->grid_width_step[j];

            grid_data[i * OT_ISP_LSC_GRID_COL + j] = get_point_average(bayer_img, lsc_cali_cfg, true_blc_offset,
                                                                       value, &location);
        }
    }

    return TD_SUCCESS;
}

/*
 * [get_ls_data description]
 * @param bayer_img        [input]  input image data
 * @param lsc_grid_data    [output] mesh_lsc grid data generated
 * @param lsc_grid_step_xy [input]  input grid width&height information
 * @param lsc_cali_cfg     [input]  mesh_lsc calibration results
 */
static td_s32 get_ls_data(const td_u16 *bayer_img, ls_data *lsc_grid_data, const isp_lsc_grid_step *lsc_grid_step_xy,
                          const ot_isp_mlsc_calibration_cfg *lsc_cali_cfg)
{
    td_s32 ret;

    ret = get_lsc_data_channel(bayer_img, lsc_grid_data->r_data, lsc_cali_cfg, lsc_grid_step_xy, BPRG);
    if (ret != TD_SUCCESS) {
        isp_err_trace("%s: LINE: %d get_lsc_data of R channel failure !\n", __FUNCTION__, __LINE__);
        return TD_FAILURE;
    }

    ret = get_lsc_data_channel(bayer_img, lsc_grid_data->gr_data, lsc_cali_cfg, lsc_grid_step_xy, BPGR);
    if (ret != TD_SUCCESS) {
        isp_err_trace("%s: LINE: %d get_lsc_data of gr channel failure !\n", __FUNCTION__, __LINE__);
        return TD_FAILURE;
    }

    ret = get_lsc_data_channel(bayer_img, lsc_grid_data->gb_data, lsc_cali_cfg, lsc_grid_step_xy, BPGB);
    if (ret != TD_SUCCESS) {
        isp_err_trace("%s: LINE: %d get_lsc_data of gb channel failure !\n", __FUNCTION__, __LINE__);
        return TD_FAILURE;
    }

    ret = get_lsc_data_channel(bayer_img, lsc_grid_data->b_data, lsc_cali_cfg, lsc_grid_step_xy, BPBG);
    if (ret != TD_SUCCESS) {
        isp_err_trace("%s: LINE: %d get_lsc_data of B channel failure !\n", __FUNCTION__, __LINE__);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

/*
 * [lc_normalization description]
 * @param ls_data          [input]  input grid data
 * @param lsc_grid_points  [input]  mesh_lsc calibration results
 * @param mesh_scale       [input]  input mesh_lsc calibration config
 * this function is used for testing if the input mesh scale is suitable for the current condition
 */
static td_void mesh_scale_test(const ls_data *ls_grid_data, td_u32 lsc_grid_points, td_u32 mesh_scale)
{
    td_u32 b_max_data, gb_max_data, gr_max_data, r_max_data;
    td_u32 b_min_data, gb_min_data, gr_min_data, r_min_data;
    td_float b_max_gain, gb_max_gain, gr_max_gain, r_max_gain;
    td_float max_gain;

    /* for b channel */
    b_max_data  = get_max_data(ls_grid_data->b_data, lsc_grid_points);
    /* for gb channel */
    gb_max_data = get_max_data(ls_grid_data->gb_data, lsc_grid_points);
    /* for gr channel */
    gr_max_data = get_max_data(ls_grid_data->gr_data, lsc_grid_points);
    /* for r channel */
    r_max_data  = get_max_data(ls_grid_data->r_data, lsc_grid_points);

    /* for b channel */
    b_min_data  = get_min_data(ls_grid_data->b_data, lsc_grid_points);
    /* for gb channel */
    gb_min_data = get_min_data(ls_grid_data->gb_data, lsc_grid_points);
    /* for gr channel */
    gr_min_data = get_min_data(ls_grid_data->gr_data, lsc_grid_points);
    /* for r channel */
    r_min_data  = get_min_data(ls_grid_data->r_data, lsc_grid_points);

    b_max_gain  = (td_float)b_max_data  / (td_float)b_min_data;
    gb_max_gain = (td_float)gb_max_data / (td_float)gb_min_data;
    gr_max_gain = (td_float)gr_max_data / (td_float)gr_min_data;
    r_max_gain  = (td_float)r_max_data  / (td_float)r_min_data;

    max_gain = MAX2(MAX3(b_max_gain, gb_max_gain, gr_max_gain), r_max_gain);
    if (max_gain > g_max_gain_array[mesh_scale]) {
        printf("WARNING:\n");
        printf("max gain = %f\n", max_gain);

        if (mesh_scale < 4) { /* under mesh_scale 4, min gain's value is 0 */
            if ((max_gain > g_max_gain_array[0]) && (max_gain <= g_max_gain_array[1])) { /* (2,4] */
                printf("please set mesh scale to %d\n", 1);
            } else if ((max_gain > g_max_gain_array[1]) && (max_gain <= g_max_gain_array[2])) { /* 2: (4,8] */
                printf("please set mesh scale to %d\n", 2); /* 2 */
            } else if ((max_gain > g_max_gain_array[2]) && (max_gain <= g_max_gain_array[3])) { /* 2, 3: (8,16] */
                printf("please set mesh scale to %d\n", 3); /* 3 */
            } else { /* >16 */
                printf("max gain is too large, please set meshscale to %d to reduce loss\n", 3); /* 3: >16 */
            }
        } else { /* min gain's value is 1 */
            if ((max_gain > g_max_gain_array[4]) && (max_gain <= g_max_gain_array[5])) { /* 4, 5: (2,3] */
                printf("please set mesh scale to %d\n", 5); /* 5 */
            } else if ((max_gain > g_max_gain_array[5]) && (max_gain <= g_max_gain_array[6])) { /* 5, 6: (3,5] */
                printf("please set mesh scale to %d\n", 6); /* 6 */
            } else if ((max_gain > g_max_gain_array[6]) && (max_gain <= g_max_gain_array[7])) { /* 6, 7: (5,9] */
                printf("please set mesh scale to %d\n", 7); /* 7 */
            } else if ((max_gain > g_max_gain_array[7]) && (max_gain <= g_max_gain_array[3])) { /* 3, 7: (9,16] */
                printf("please set mesh scale to %d\n", 3); /* 3 */
            } else {
                printf("max gain is too large, please set meshscale to %d to reduce loss\n", 3); /* 3: >16 */
            }
        }
    }

    return;
}

/*
 * [lc_normalization description]
 * @param lsc_grid_data     [input]  input grid data
 * @param lsc_cali_result   [output] mesh_lsc calibration results
 * @param lsc_cali_cfg      [input]  input mesh_lsc calibration config
 */
static td_void lc_normalization(const ls_data *lsc_grid_data, ot_isp_mesh_shading_table *lsc_cali_result,
                                const ot_isp_mlsc_calibration_cfg *lsc_cali_cfg)
{
    td_u32 b_max_data, gb_max_data, gr_max_data, r_max_data;
    td_u32 lsc_grid_points;
    td_u32 i;

    lsc_grid_points = OT_ISP_LSC_GRID_POINTS;

    mesh_scale_test(lsc_grid_data, OT_ISP_LSC_GRID_POINTS, lsc_cali_cfg->mesh_scale);

    /* find the max data of each channel */
    /* for b channel */
    b_max_data  = get_max_data(lsc_grid_data->b_data, lsc_grid_points);
    /* for gb channel */
    gb_max_data = get_max_data(lsc_grid_data->gb_data, lsc_grid_points);
    /* for gr channel */
    gr_max_data = get_max_data(lsc_grid_data->gr_data, lsc_grid_points);
    /* for r channel */
    r_max_data  = get_max_data(lsc_grid_data->r_data, lsc_grid_points);

    for (i = 0; i < lsc_grid_points; i++) {
        /* normalization process */
        lsc_cali_result->lsc_gain_lut.b_gain[i]  = (td_u16)(MIN2(data_enlarge(lsc_grid_data->b_data[i],
            b_max_data, lsc_cali_cfg->mesh_scale), LSC_MAX_PIXEL_VALUE));
        lsc_cali_result->lsc_gain_lut.gb_gain[i] = (td_u16)(MIN2(data_enlarge(lsc_grid_data->gb_data[i],
            gb_max_data, lsc_cali_cfg->mesh_scale), LSC_MAX_PIXEL_VALUE));
        lsc_cali_result->lsc_gain_lut.gr_gain[i] = (td_u16)(MIN2(data_enlarge(lsc_grid_data->gr_data[i],
            gr_max_data, lsc_cali_cfg->mesh_scale), LSC_MAX_PIXEL_VALUE));
        lsc_cali_result->lsc_gain_lut.r_gain[i]  = (td_u16)(MIN2(data_enlarge(lsc_grid_data->r_data[i],
            r_max_data, lsc_cali_cfg->mesh_scale), LSC_MAX_PIXEL_VALUE));
    }
    lsc_cali_result->mesh_scale = lsc_cali_cfg->mesh_scale;

    return;
}

/*
 * [geometric_grid_size_y description]
 * @param lsc_cali_cfg      [input]  input mesh_lsc calibration config
 * @param lsc_grid_step_xy  [output] grid width&height info
 * @param lsc_cali_result   [output] mesh_lsc calibration results
 */
static td_void geometric_grid_size_x(const ot_isp_mlsc_calibration_cfg *lsc_cali_cfg,
    isp_lsc_grid_step *lsc_grid_step_xy, ot_isp_mesh_shading_table *lsc_cali_result)
{
    td_u32   tmp_step_x[OT_ISP_MLSC_X_HALF_GRID_NUM];
    td_float rx[OT_ISP_MLSC_X_HALF_GRID_NUM];
    td_float sum_r;

    td_u32 i, sum;
    td_u32 half_grid_size_x = OT_ISP_MLSC_X_HALF_GRID_NUM;
    td_u32 width, diff;

    const td_float r1 = 1.0f;
    width = lsc_cali_cfg->dst_img_width / 2; /* process by channel, need to divided by 2 */

    rx[0] = 1.0f;
    for (i = 1; i < half_grid_size_x; i++) {
        rx[i] = rx[i - 1] * r1;
    }
    sum_r = 0;
    for (i = 0; i < half_grid_size_x; i++) {
        sum_r = sum_r + rx[i];
    }
    for (i = 0; i < half_grid_size_x; i++) {
        tmp_step_x[i] = (int)((((width >> 1) * 0x400 / div_0_to_1_float(sum_r)) * rx[i] + 0x200) / 0x400);
    }

    sum = 0;
    for (i = 0; i < half_grid_size_x; i++) {
        sum = sum + tmp_step_x[i];
    }
    if (sum != (width / 2)) { /* calculate only 1/2 of the width */
        if (sum > (width / 2)) { /* calculate only 1/2 of the width */
            diff = sum - width / 2; /* calculate only 1/2 of the width */
            for (i = 0; i < diff; i++) {
                tmp_step_x[i] = tmp_step_x[i] - 1;
            }
        } else {
            diff = width / 2 - sum; /* calculate only 1/2 of the width */
            for (i = 0; i < diff; i++) {
                tmp_step_x[half_grid_size_x - i - 1] = tmp_step_x[half_grid_size_x - i - 1] + 1;
            }
        }
    }

    /* return the step length value to grid_step_width and grid_step_height */
    lsc_grid_step_xy->grid_width_step[0] = 0;
    for (i = 1; i <= half_grid_size_x; i++) {
        lsc_grid_step_xy->grid_width_step[i] = tmp_step_x[i - 1];
        lsc_grid_step_xy->grid_width_step[OT_ISP_LSC_GRID_COL - i] = tmp_step_x[i - 1];
        lsc_cali_result->x_grid_width[i - 1] = lsc_grid_step_xy->grid_width_step[i];
    }

    return;
}

/*
 * [geometric_grid_size_y description]
 * @param lsc_cali_cfg      [input]  input mesh_lsc calibration config
 * @param lsc_grid_step_xy   [output]  grid width&height info
 * @param lsc_cali_result   [output] mesh_lsc calibration results
 */
static td_void geometric_grid_size_y(const ot_isp_mlsc_calibration_cfg *lsc_cali_cfg,
    isp_lsc_grid_step *lsc_grid_step_xy, ot_isp_mesh_shading_table *lsc_cali_result)
{
    td_float ry[OT_ISP_MLSC_Y_HALF_GRID_NUM];
    td_float sum_r;
    td_u32   tmp_step_y[OT_ISP_MLSC_Y_HALF_GRID_NUM];

    td_u32 j, sum;
    td_u32 half_grid_size_y = OT_ISP_MLSC_Y_HALF_GRID_NUM;
    td_u32 height, diff;

    height = lsc_cali_cfg->dst_img_height / 2; /* process by channel, need to divided by 2 */

    const td_float r1 = 1.0f;
    ry[0] = 1.0f;

    for (j = 1; j < half_grid_size_y; j++) {
        ry[j] = ry[j - 1] * r1;
    }
    sum_r = 0;
    for (j = 0; j < half_grid_size_y; j++) {
        sum_r = sum_r + ry[j];
    }
    for (j = 0; j < half_grid_size_y; j++) {
        tmp_step_y[j] = (int)((((height >> 1) * 0x400 / div_0_to_1_float(sum_r)) * ry[j] + 0x200) / 0x400);
    }

    sum = 0;
    for (j = 0; j < half_grid_size_y; j++) {
        sum = sum + tmp_step_y[j];
    }
    if (sum != (height / 2)) { /* calculate only 1/2 of the height */
        if (sum > (height / 2)) { /* calculate only 1/2 of the height */
            diff = sum - height / 2; /* calculate only 1/2 of the height */
            for (j = 0; j < diff; j++) {
                tmp_step_y[j] = tmp_step_y[j] - 1;
            }
        } else {
            diff = height / 2 - sum; /* calculate only 1/2 of the height */
            for (j = 0; j < diff; j++) {
                tmp_step_y[half_grid_size_y - j - 1] = tmp_step_y[half_grid_size_y - j - 1] + 1;
            }
        }
    }

    /* return the step length value to grid_step_width and grid_step_height */
    lsc_grid_step_xy->grid_height_step[0] = 0;
    for (j = 1; j <= half_grid_size_y; j++) {
        lsc_grid_step_xy->grid_height_step[j] = tmp_step_y[j - 1];
        lsc_grid_step_xy->grid_height_step[OT_ISP_LSC_GRID_ROW - j] = tmp_step_y[j - 1];
        lsc_cali_result->y_grid_width[j - 1] = lsc_grid_step_xy->grid_height_step[j];
    }

    return;
}

/*
 * [lsc_generate_grid_info description]
 * @param data             [input]  input raw data that used for calibration
 * @param lsc_cali_cfg     [input]  input mesh_lsc calibration config
 * @param lsc_grid_step_xy [input]  grid width&height info
 * @param lsc_cali_result  [output] mesh_lsc calibration results
 */
static td_s32 lsc_generate_grid_info(const td_u16 *data, const ot_isp_mlsc_calibration_cfg *lsc_cali_cfg,
    const isp_lsc_grid_step *lsc_grid_step_xy, ot_isp_mesh_shading_table *lsc_cali_result)
{
    /* pass all the input params to the function */
    ls_data *ls_grid_data = TD_NULL;
    td_s32 ret;

    /* memory allocation */
    ls_grid_data = (ls_data *)isp_malloc(sizeof(ls_data));
    if (ls_grid_data == TD_NULL) {
        isp_err_trace("ls_gird_data allocation fail!\n");
        return TD_FAILURE;
    }

    (td_void)memset_s(ls_grid_data, sizeof(ls_data), 0, sizeof(ls_data));

    /* get lens correction coefficients */
    ret = get_ls_data(data, ls_grid_data, lsc_grid_step_xy, lsc_cali_cfg);
    if (ret != TD_SUCCESS) {
        isp_err_trace("there are some errors in get_ls_data()!\n");
        isp_free(ls_grid_data);
        return TD_FAILURE;
    }

    lc_normalization(ls_grid_data, lsc_cali_result, lsc_cali_cfg);

    isp_free(ls_grid_data);
    return ret;
}

/*
 * [isp_mesh_shading_calibration description]
 * @param src_raw       [input]  input raw data that used for calibration
 * @param mlsc_cali_cfg [input]  input mesh_lsc calibration config
 * @param mlsc_table    [output] mesh_lsc calibration results
 */
td_s32 isp_mesh_shading_calibration(const td_u16 *src_raw,
                                    ot_isp_mlsc_calibration_cfg *mlsc_cali_cfg,
                                    ot_isp_mesh_shading_table *mlsc_table)
{
    td_s32 ret, ret1;
    td_s32 i, j;
    td_phys_addr_t phy_addr = 0;
    size_t size;
    td_u16 *crop_raw = TD_NULL;

    isp_lsc_grid_step lsc_grid_step_xy;

    size = sizeof(td_u16) * mlsc_cali_cfg->dst_img_width * mlsc_cali_cfg->dst_img_height;
    ret = ot_mpi_sys_mmz_alloc_cached(&phy_addr, (td_void **)(&crop_raw), "mlsc_calib", TD_NULL, size);
    if (ret != TD_SUCCESS) {
        isp_err_trace("malloc memory failed!\n");
        return TD_FAILURE;
    }
    (td_void)memset_s(crop_raw, sizeof(td_u16) * mlsc_cali_cfg->dst_img_width * mlsc_cali_cfg->dst_img_height,
                      0, sizeof(td_u16) * mlsc_cali_cfg->dst_img_width * mlsc_cali_cfg->dst_img_height);

    for (j = 0; j < mlsc_cali_cfg->dst_img_height; j++) {
        for (i = 0; i < mlsc_cali_cfg->dst_img_width; i++) {
            *(crop_raw + j * mlsc_cali_cfg->dst_img_width + i) = *(src_raw +
                (j + mlsc_cali_cfg->offset_y) * mlsc_cali_cfg->img_width + i + mlsc_cali_cfg->offset_x);
        }
    }

    /* get grid info */
    geometric_grid_size_x(mlsc_cali_cfg, &lsc_grid_step_xy, mlsc_table);

    geometric_grid_size_y(mlsc_cali_cfg, &lsc_grid_step_xy, mlsc_table);

    /* malloc memory */
    ret = lsc_generate_grid_info(crop_raw, mlsc_cali_cfg, &lsc_grid_step_xy, mlsc_table);
    if (ret == TD_FAILURE) {
        isp_err_trace("there are some errors in lsc_generate_grid_info()!\n");
        goto end;
    }

end:
    ret1 = ot_mpi_sys_mmz_free(phy_addr, (td_void *)crop_raw);
    if (ret1 != TD_SUCCESS) {
        isp_err_trace("ot_mpi_sys_mmz_free err!\n");
        return ret1;
    }

    return ret;
}

