/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_AE_ADP_H
#define SAMPLE_AE_ADP_H

#include <string.h>
#include "ot_type.h"
#include "ot_common_3a.h"
#include "ot_common_ae.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct {
    /* usr var */
    td_u8                   ae_mode;

    /* communicate with isp */
    ot_isp_ae_param         ae_param;
    td_u32                  frame_cnt;
    ot_isp_ae_info          ae_info;
    ot_isp_ae_result        ae_result;
    ot_vi_pipe              isp_bind_dev;

    /* communicate with sensor, defined by user. */
    td_bool                   b_sns_register;
    ot_isp_sns_attr_info      sns_attr_info;
    ot_isp_ae_sensor_default  sns_def;
    ot_isp_ae_sensor_register sns_register;

    /* global variables of ae algorithm */
} sample_ae_ctx_s;

#define MAX_AE_REGISTER_SNS_NUM 1
#define GAIN_USER_LINEAR_SHIFT  10

/* we assumed that the different lib instance have different id,
 * use the id 0 & 1.
 */
#define ae_get_extreg_id(handle)   (((handle) == 0) ? 0x4 : 0x5)

#define sample_ae_check_handle_id_return(handle) \
    do { \
        if (((handle) < 0) || ((handle) >= OT_ISP_MAX_AE_LIB_NUM)) { \
            printf("Illegal handle id %d in %s!\n", (handle), __FUNCTION__); \
            return TD_FAILURE; \
        } \
    } while (0)

#define sample_ae_check_lib_name_return(acName) \
    do { \
        if (strcmp((acName), OT_AE_LIB_NAME) != 0) { \
            printf("Illegal lib name %s in %s!\n", (acName), __FUNCTION__); \
            return TD_FAILURE; \
        } \
    } while (0)

#define sample_ae_check_pointer_return(ptr) \
    do { \
        if ((ptr) == TD_NULL) { \
            printf("Null Pointer in %s!\n", __FUNCTION__); \
            return TD_FAILURE; \
        } \
    } while (0)

#define sample_ae_check_pipe_return(pipe) \
    do { \
        if (((pipe) < 0) || ((pipe) >= OT_ISP_MAX_PIPE_NUM)) { \
            isp_err_trace("Err AE pipe %d in %s!\n", pipe, __FUNCTION__); \
            return OT_ERR_ISP_ILLEGAL_PARAM; \
        } \
    } while (0)

#define sample_ae_emerg_trace(fmt, ...) \
    OT_EMERG_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_ae_alert_trace(fmt, ...) \
    OT_ALERT_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_ae_crit_trace(fmt, ...) \
    OT_CRIT_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_ae_err_trace(fmt, ...) \
    OT_ERR_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_ae_warn_trace(fmt, ...) \
    OT_WARN_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_ae_notice_trace(fmt, ...) \
    OT_NOTICE_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_ae_info_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_ae_debug_trace(fmt, ...) \
    OT_DEBUG_TRACE(OT_ID_ISP, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define sample_ae_free(ptr) \
    do { \
        if ((ptr) != TD_NULL) { \
            free(ptr); \
            (ptr) = TD_NULL; \
        } \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
