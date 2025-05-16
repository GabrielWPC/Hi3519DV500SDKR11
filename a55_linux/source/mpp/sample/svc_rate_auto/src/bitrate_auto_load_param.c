/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include "ini_parser.h"
#include "ot_type.h"
#include "ot_bitrate_auto.h"
#include "securec.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define ITEM_LEN  64

typedef enum {
    RATE_AUTO_MODE_BITRATE_FIRST = 0,
    RATE_AUTO_MODE_QUALITY_FIRST,
    RATE_AUTO_MODE_BUTT
}rate_auto_mode;

#define bitrate_assert_return(condition, value) \
        do { \
            if (!(condition)) { \
                printf("bitrate load param assert '%s' error, func:%s, line:%d\n", #condition, __func__, __LINE__); \
                return value; \
            } \
        } while (0)

static td_s32 bitrate_auto_load_fg_qpmap_val_p(const ini_dictionary *dict, rate_auto_mode mode,
    rate_auto_param *rate_auto_para, td_char item[], td_u32 type)
{
    td_s32 value;

    value = ini_get_int(dict, item, TD_FAILURE);
    if (value == TD_FAILURE) {
        printf("rate_auto_base.%d:svc_fg%u_qpmap_val_p failed\n", mode, type);
        return TD_FAILURE;
    }
    if (value < 0 || value > 115) { /* max value: 115, 64+51=115 */
        printf("rate_auto_base.%d:svc_fg%u_qpmap_val_p over[0, 115]\n", mode, type);
        return TD_FAILURE;
    }
    rate_auto_para->svc_fg_qpmap_val_p[type] = (td_u8)value;

    return TD_SUCCESS;
}

static td_s32 load_fg_qpmap_val_p(const ini_dictionary *dict, rate_auto_mode mode, rate_auto_param *rate_auto_para)
{
    td_s32 ret;
    td_char item[ITEM_LEN] = {0};
    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg0_qpmap_val_p", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_fg_qpmap_val_p(dict, mode, rate_auto_para, item, FG_TYPE_0);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg1_qpmap_val_p", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_fg_qpmap_val_p(dict, mode, rate_auto_para, item, FG_TYPE_1);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg2_qpmap_val_p", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_fg_qpmap_val_p(dict, mode, rate_auto_para, item, FG_TYPE_2);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg3_qpmap_val_p", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_fg_qpmap_val_p(dict, mode, rate_auto_para, item, FG_TYPE_3);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg4_qpmap_val_p", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_fg_qpmap_val_p(dict, mode, rate_auto_para, item, FG_TYPE_4);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    return TD_SUCCESS;
}

static td_s32 bitrate_auto_svc_fg_qpmap_val_i(const ini_dictionary *dict, rate_auto_mode mode,
    rate_auto_param *rate_auto_para, td_char item[], td_u32 type)
{
    td_s32 value;

    value = ini_get_int(dict, item, TD_FAILURE);
    if (value == TD_FAILURE) {
        printf("rate_auto_base.%d:svc_fg%u_qpmap_val_i failed\n", mode, type);
        return TD_FAILURE;
    }

    if (value < 0 || value > 115) { /* max value: 115, 64+51=115 */
        printf("rate_auto_base.%d:svc_fg%u_qpmap_val_i over[0, 115]\n", mode, type);
        return TD_FAILURE;
    }

    rate_auto_para->svc_fg_qpmap_val_i[type] = (td_u8)value;

    return TD_SUCCESS;
}

static td_s32 load_fg_qpmap_val_i(const ini_dictionary *dict, rate_auto_mode mode, rate_auto_param *rate_auto_para)
{
    td_s32 ret;
    td_char item[ITEM_LEN] = {0};
    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg0_qpmap_val_i", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_svc_fg_qpmap_val_i(dict, mode, rate_auto_para, item, FG_TYPE_0);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg1_qpmap_val_i", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_svc_fg_qpmap_val_i(dict, mode, rate_auto_para, item, FG_TYPE_1);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg2_qpmap_val_i", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_svc_fg_qpmap_val_i(dict, mode, rate_auto_para, item, FG_TYPE_2);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg3_qpmap_val_i", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_svc_fg_qpmap_val_i(dict, mode, rate_auto_para, item, FG_TYPE_3);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg4_qpmap_val_i", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_svc_fg_qpmap_val_i(dict, mode, rate_auto_para, item, FG_TYPE_4);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    return TD_SUCCESS;
}

static td_s32 bitrate_auto_load_fg_skipmap_val(const ini_dictionary *dict, rate_auto_mode mode,
    rate_auto_param *rate_auto_para, td_char item[], td_u32 type)
{
    td_s32 value;

    value = ini_get_int(dict, item, TD_FAILURE);
    if (value == TD_FAILURE) {
        printf("rate_auto_base.%d:svc_fg%u_skipmap_val failed\n", mode, type);
        return TD_FAILURE;
    }

    if (value < 0 || value > 115) { /* max value: 115, 64+51=115 */
        printf("rate_auto_base.%d:svc_fg%u_skipmap_val over[0, 115]\n", mode, type);
        return TD_FAILURE;
    }

    rate_auto_para->svc_fg_skipmap_val[type] = (td_u8)value;

    return TD_SUCCESS;
}

static td_s32 load_fg_skipmap_val(const ini_dictionary *dict, rate_auto_mode mode, rate_auto_param *rate_auto_para)
{
    td_s32 ret;
    td_char item[ITEM_LEN] = {0};
    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg0_skipmap_val", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_fg_skipmap_val(dict, mode, rate_auto_para, item, FG_TYPE_0);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg1_skipmap_val", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_fg_skipmap_val(dict, mode, rate_auto_para, item, FG_TYPE_1);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg2_skipmap_val", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_fg_skipmap_val(dict, mode, rate_auto_para, item, FG_TYPE_2);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg3_skipmap_val", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_fg_skipmap_val(dict, mode, rate_auto_para, item, FG_TYPE_3);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_fg4_skipmap_val", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_fg_skipmap_val(dict, mode, rate_auto_para, item, FG_TYPE_4);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    return TD_SUCCESS;
}

static td_s32 bitrate_auto_load_param(const ini_dictionary *dict, rate_auto_mode mode,
    rate_auto_param *rate_auto_para, td_char item[], td_s32 *val)
{
    td_s32 value;

    value = ini_get_int(dict, item, TD_FAILURE);
    if (value == TD_FAILURE) {
        printf("rate_auto_base.%d:load param failed\n", mode);
        return TD_FAILURE;
    }

    if (value < 0 || value > 115) { /* max value: 115, 64+51=115 */
        printf("rate_auto_base.%d:load param failed over[0, 115]\n", mode);
        return TD_FAILURE;
    }

    *val = value;

    return TD_SUCCESS;
}

static td_s32 load_bg_val(const ini_dictionary *dict, rate_auto_mode mode, rate_auto_param *rate_auto_para)
{
    td_s32 value, ret;
    td_char item[ITEM_LEN] = {0};

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_bg_qpmap_val_p", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_param(dict, mode, rate_auto_para, item, &value);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);
    rate_auto_para->svc_bg_qpmap_val_p = (td_u8)value;

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_bg_qpmap_val_i", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_param(dict, mode, rate_auto_para, item, &value);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    rate_auto_para->svc_bg_qpmap_val_i = (td_u8)value;

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_bg_skipmap_val", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_param(dict, mode, rate_auto_para, item, &value);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    rate_auto_para->svc_bg_skipmap_val = (td_u8)value;
    return TD_SUCCESS;
}

static td_s32 load_roi_val(const ini_dictionary *dict, rate_auto_mode mode, rate_auto_param *rate_auto_para)
{
    td_s32 value, ret;
    td_char item[ITEM_LEN] = {0};

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_roi_qpmap_val_p", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_param(dict, mode, rate_auto_para, item, &value);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    rate_auto_para->svc_roi_qpmap_val_p = (td_u8)value;

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_roi_qpmap_val_i", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_param(dict, mode, rate_auto_para, item, &value);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    rate_auto_para->svc_roi_qpmap_val_i = (td_u8)value;

    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:svc_roi_skipmap_val", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_param(dict, mode, rate_auto_para, item, &value);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    rate_auto_para->svc_roi_skipmap_val = (td_u8)value;
    return TD_SUCCESS;
}

static td_s32 bitrate_auto_load_avbr_rate_control(const ini_dictionary *dict, rate_auto_mode mode,
    rate_auto_param *rate_auto_para, td_char item[], td_s32 *val)
{
    td_s32 value;

    value = ini_get_int(dict, item, TD_FAILURE);
    if (value == TD_FAILURE) {
        printf("rate_auto_base.%d:load param failed\n", mode);
        return TD_FAILURE;
    }

    if (value < 0 || value > 2) { /* avbr rate control value: 1, 2 */
        printf("rate_auto_base.%d:load param failed over[0,2]\n", mode);
        return TD_FAILURE;
    }

    *val = value;

    return TD_SUCCESS;
}

static td_s32 bitrate_auto_load_fg_protest_adjust(const ini_dictionary *dict, rate_auto_mode mode,
    rate_auto_param *rate_auto_para, td_char item[], td_s32 *val)
{
    td_s32 value;

    value = ini_get_int(dict, item, TD_FAILURE);
    if (value == TD_FAILURE) {
        printf("rate_auto_base.%d:load param failed\n", mode);
        return TD_FAILURE;
    }

    if (value < 0 || value > 1) {
        printf("rate_auto_base.%d:load param failed over[0,1]\n", mode);
        return TD_FAILURE;
    }

    *val = value;

    return TD_SUCCESS;
}

static td_s32 bitrate_auto_load_qp(const ini_dictionary *dict, rate_auto_mode mode,
    rate_auto_param *rate_auto_para, td_char item[], td_s32 *val)
{
    td_s32 value;

    value = ini_get_int(dict, item, TD_FAILURE);
    if (value == TD_FAILURE) {
        printf("rate_auto_base.%d:load param failed\n", mode);
        return TD_FAILURE;
    }

    if (value < 0 || value > 51) { /* 8bit max qp: 51 */
        printf("rate_auto_base.%d:load param failed over[0, 51]\n", mode);
        return TD_FAILURE;
    }

    *val = value;

    return TD_SUCCESS;
}

static td_s32 rate_auto_load_param(const ini_dictionary *dict, rate_auto_mode mode, rate_auto_param *rate_auto_para)
{
    td_s32 value, ret;
    td_char item[ITEM_LEN] = {0};
    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:avbr_rate_control", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_avbr_rate_control(dict, mode, rate_auto_para, item, &value);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    rate_auto_para->avbr_rate_control = value;
    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:max_bg_qp", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_qp(dict, mode, rate_auto_para, item, &value);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    rate_auto_para->max_bg_qp = value;
    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:min_fg_qp", mode) < 0) {
        return TD_FAILURE;
    }
    ret = bitrate_auto_load_qp(dict, mode, rate_auto_para, item, &value);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    rate_auto_para->min_fg_qp = value;
    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:max_fg_qp", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_qp(dict, mode, rate_auto_para, item, &value);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    rate_auto_para->max_fg_qp = value;
    if (snprintf_s(item, ITEM_LEN, ITEM_LEN - 1, "rate_auto_base.%d:fg_protect_adjust", mode) < 0) {
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_fg_protest_adjust(dict, mode, rate_auto_para, item, &value);
    bitrate_assert_return(ret == TD_SUCCESS, TD_FAILURE);

    rate_auto_para->fg_protect_adjust = value;
    return TD_SUCCESS;
}

static td_s32 bitrate_auto_check_value(td_s32 value)
{
    if (value < RATE_AUTO_MODE_BITRATE_FIRST || value > RATE_AUTO_MODE_QUALITY_FIRST) {
        printf("rate auto mode:%d invalid\n", value);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 bitrate_auto_load_all_param(const ini_dictionary *dic, td_s32 value, rate_auto_param *rate_auto_para)
{
    td_s32 ret, ret1, ret2, ret3, ret4, ret5;

    ret = rate_auto_load_param(dic, value, rate_auto_para);
    ret1 = load_fg_qpmap_val_p(dic, value, rate_auto_para);
    ret2 = load_fg_qpmap_val_i(dic, value, rate_auto_para);
    ret3 = load_fg_skipmap_val(dic, value, rate_auto_para);
    ret4 = load_bg_val(dic, value, rate_auto_para);
    ret5 = load_roi_val(dic, value, rate_auto_para);
    ret = ret || ret1 || ret2 || ret3 || ret4 || ret5;

    return ret;
}
td_s32 ot_rate_auto_load_param(td_char *module_name, rate_auto_param *rate_auto_para)
{
    ini_dictionary *dic = NULL;
    td_s32 value, ret;

    dic = ini_process_file(module_name);
    if (dic == NULL) {
        printf("ini_process_file fail\n");
        free(dic);
        return TD_FAILURE;
    }

    value = ini_get_int(dic, "rate_auto_mode:mode", TD_FAILURE);
    if (value == TD_FAILURE && dic != NULL) {
        printf("rate_auto_base:rate_auto_mode failed\n");
        free_ini_info_dict(dic);
        return TD_FAILURE;
    }

    printf("rate auto mode:%d\n", value);

    ret = bitrate_auto_check_value(value);
    if (ret == TD_FAILURE && dic != NULL) {
        free_ini_info_dict(dic);
        return TD_FAILURE;
    }

    ret = bitrate_auto_load_all_param(dic, value, rate_auto_para);
    if (ret != TD_SUCCESS) {
        printf("rate auto load param failed\n");
        ret = TD_FAILURE;
    }

    if (dic != NULL) {
        free_ini_info_dict(dic);
    }
    return ret;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
