/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#ifndef __SAMPLE_AI3DNR_H__
#define __SAMPLE_AI3DNR_H__

#include "sample_aiisp_common.h"
#include "ot_common_ai3dnr.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
    ot_ai3dnr_net_type net_type;
} sample_ai3dnr_param;

td_s32 sample_ai3dnr(sample_ai3dnr_param *ai3dnr_param);
td_s32 sample_ai3dnr_and_mcf(td_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
