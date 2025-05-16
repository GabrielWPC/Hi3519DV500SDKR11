/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#ifndef __SAMPLE_AIBNR_H__
#define __SAMPLE_AIBNR_H__

#include "sample_aiisp_common.h"
#include "ot_common_aibnr.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
    td_bool is_wdr_mode;
    ot_aibnr_ref_mode ref_mode;
    td_bool is_blend;
} sample_aibnr_param;

td_s32 sample_aibnr(sample_aibnr_param *aibnr_param);
td_s32 sample_aibnr_snap(sample_aibnr_param *aibnr_param);
td_s32 sample_aibnr_line_switch_to_wdr(td_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
