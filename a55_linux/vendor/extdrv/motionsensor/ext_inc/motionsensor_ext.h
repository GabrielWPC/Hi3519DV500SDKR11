/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: motionsensor_ext.h
 * Author: ISP SW
 * Create: 2018-7-30
 * Version: Initial Draft
 */

#ifndef MOTIONSENSOR_EXT_H
#define MOTIONSENSOR_EXT_H

#include "ot_debug.h"
#include "ot_common_motionsensor.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct {
    td_s32  (*pfn_get_config_from_chip) (ot_msensor_param *param);
    td_s32  (*pfn_write_data_to_buf) (td_void);
} msensor_mng_callback;

typedef td_s32 fn_msensor_mng_add_user(td_s32 *id);
typedef td_s32 fn_msensor_mng_delete_user(const td_s32 *id);
typedef td_s32 fn_msensor_mng_get_data(ot_msensor_data_info *data);
typedef td_s32 fn_msensor_mng_release_data(ot_msensor_data_info *data);
typedef td_s32 fn_msensor_mng_get_m_sensor_config(ot_msensor_param *param);

typedef td_s32 fn_msensor_mng_write_data_to_buff(ot_msensor_data *chip_data);
typedef td_s32 fn_msensor_mng_init(const ot_msensor_attr *attr, const ot_msensor_buf_attr *buf_attr,
    const ot_msensor_config *config);

typedef td_s32 fn_msensor_mng_deinit(td_void);

typedef td_s32 fn_msensor_mng_register_call_back(const msensor_mng_callback *callback);
typedef td_void fn_msensor_mng_un_register_call_back(td_void);

typedef struct {
    /* call_back to dis/avs */
    fn_msensor_mng_add_user              *pfn_add_msensor_user;
    fn_msensor_mng_delete_user           *pfn_delete_msensor_user;
    fn_msensor_mng_get_data              *pfn_get_data;
    fn_msensor_mng_release_data          *pfn_release_data;
    fn_msensor_mng_get_m_sensor_config   *pfn_get_msensor_config;

    /* call_back to motionsensor_chip */
    fn_msensor_mng_write_data_to_buff    *pfn_chip_write_data_to_mng_buff;
    fn_msensor_mng_init                  *pfn_init;
    fn_msensor_mng_deinit                *pfn_deinit;

    /* register to motionsensor_chip */
    fn_msensor_mng_register_call_back    *pfn_register_call_back;
    fn_msensor_mng_un_register_call_back *pfn_unregister_call_back;
} msensor_mng_export_func;

#define ckfn_msensor_mng_register_call_back() \
    (func_entry(msensor_mng_export_func, OT_ID_MOTIONSENSOR)->pfn_register_call_back != TD_NULL)
#define call_msensor_mng_register_call_back(mng_callback) \
    func_entry(msensor_mng_export_func, OT_ID_MOTIONSENSOR)->pfn_register_call_back(mng_callback)

#define ckfn_msensor_mng_unregister_call_back() \
    (func_entry(msensor_mng_export_func, OT_ID_MOTIONSENSOR)->pfn_unregister_call_back != TD_NULL)
#define call_msensor_mng_unregister_call_back(td_void) \
    func_entry(msensor_mng_export_func, OT_ID_MOTIONSENSOR)->pfn_unregister_call_back(td_void)

/* debug print format  */
#define msensor_emerg_trace(fmt, ...) \
    OT_EMERG_TRACE(OT_ID_MOTIONSENSOR, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define msensor_alert_trace(fmt, ...) \
    OT_ALERT_TRACE(OT_ID_MOTIONSENSOR, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define msensor_crit_trace(fmt, ...) \
    OT_CRIT_TRACE(OT_ID_MOTIONSENSOR, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define msensor_err_trace(fmt, ...) \
    OT_ERR_TRACE(OT_ID_MOTIONSENSOR, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define msensor_warn_trace(fmt, ...) \
    OT_WARN_TRACE(OT_ID_MOTIONSENSOR, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define msensor_notice_trace(fmt, ...) \
    OT_NOTICE_TRACE(OT_ID_MOTIONSENSOR, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define msensor_info_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_MOTIONSENSOR, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define msensor_debug_trace(fmt, ...) \
    OT_DEBUG_TRACE(OT_ID_MOTIONSENSOR, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif
