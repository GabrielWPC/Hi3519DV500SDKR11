/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_AWB_ADP_H
#define SAMPLE_AWB_ADP_H

#include <string.h>

#include "ot_common_3a.h"
#include "ot_common_awb.h"
#include "ot_type.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct ot_sample_awb_ctx_s {
    /* usr var */
    td_u16                  detect_temp;
    td_u8                   wb_type;

    /* communicate with isp */
    ot_isp_awb_param        awb_param;
    td_u32                  frame_cnt;
    ot_isp_awb_info         awb_info;
    ot_isp_awb_result       awb_result;
    ot_vi_pipe              isp_bind_dev;

    /* communicate with sensor, defined by user. */
    td_bool                 sns_register_flag;
    ot_isp_sns_attr_info    sns_attr_info;
    ot_isp_awb_sensor_default sns_dft;
    ot_isp_awb_sensor_register sns_register;

    /* global variables of awb algorithm */
} sample_awb_ctx_s;

#define MAX_AWB_REGISTER_SNS_NUM 1

#define sample_awb_check_handle_id_return(handle) \
    do { \
        if (((handle) < 0) || ((handle) >= OT_ISP_MAX_AWB_LIB_NUM)) { \
            printf("Illegal handle id %d in %s!\n", (handle), __FUNCTION__); \
            return TD_FAILURE; \
        } \
    }while (0)

#define sample_awb_check_lib_name_return(acName) \
    do { \
        if (strncmp((acName), OT_AWB_LIB_NAME, ALG_LIB_NAME_SIZE_MAX) != 0) { \
            printf("Illegal lib name %s in %s!\n", (acName), __FUNCTION__); \
            return TD_FAILURE; \
        } \
    }while (0)

#define sample_awb_check_pointer_return(ptr) \
    do { \
        if ((ptr) == TD_NULL) { \
            printf("Null Pointer in %s!\n", __FUNCTION__); \
            return TD_FAILURE; \
        } \
    }while (0)

#define sample_awb_check_pipe_return(pipe) \
    do { \
        if (((pipe) < 0) || ((pipe) >= OT_ISP_MAX_PIPE_NUM)) { \
            isp_err_trace("Err AWB pipe %d in %s!\n", pipe, __FUNCTION__); \
            return OT_ERR_ISP_ILLEGAL_PARAM; \
        } \
    }while (0)

#define sample_awb_emerg_trace(fmt, ...) \
    OT_EMERG_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_awb_alert_trace(fmt, ...) \
    OT_ALERT_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_awb_crit_trace(fmt, ...) \
    OT_CRIT_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_awb_err_trace(fmt, ...) \
    OT_ERR_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_awb_warn_trace(fmt, ...) \
    OT_WARN_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_awb_notice_trace(fmt, ...) \
    OT_NOTICE_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_awb_info_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_awb_debug_trace(fmt, ...) \
    OT_DEBUG_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
