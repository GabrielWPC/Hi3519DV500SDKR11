/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#ifndef OT_RATE_AUTO
#define OT_RATE_AUTO

#include <stdio.h>
#include "ot_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifndef rate_auto_prt
#define rate_auto_prt(fmt...)   \
    do { \
        printf("[%s]-%d: ", __FUNCTION__, __LINE__); \
        printf(fmt); \
    } while (0)
#endif

#define SVC_RECT_TYPE_NUM 5
#define FG_TYPE_0     0
#define FG_TYPE_1     1
#define FG_TYPE_2     2
#define FG_TYPE_3     3
#define FG_TYPE_4     4

typedef struct {
    td_u8 avbr_rate_control;
    td_u8 svc_fg_qpmap_val_p[SVC_RECT_TYPE_NUM];
    td_u8 svc_fg_qpmap_val_i[SVC_RECT_TYPE_NUM];
    td_u8 svc_fg_skipmap_val[SVC_RECT_TYPE_NUM];
    td_u8 svc_bg_qpmap_val_p;
    td_u8 svc_bg_qpmap_val_i;
    td_u8 svc_bg_skipmap_val;
    td_u8 svc_roi_qpmap_val_p;
    td_u8 svc_roi_qpmap_val_i;
    td_u8 svc_roi_skipmap_val;
    td_u32 max_bg_qp;
    td_u32 min_fg_qp;
    td_u32 max_fg_qp;
    td_u8 fg_protect_adjust;
} rate_auto_param;

td_s32 ot_rate_auto_init(const rate_auto_param *init_param);
td_s32 ot_rate_auto_deinit(td_void);
td_s32 ot_rate_auto_load_param(td_char *module_name, rate_auto_param *rate_auto_para);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* End of #ifndef OT_RATE_AUTO */
