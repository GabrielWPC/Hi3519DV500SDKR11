/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_AIDRC_H
#define SAMPLE_AIDRC_H

#include "sample_aiisp_common.h"
#include "ot_common_aidrc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
    td_bool is_wdr_mode;
    ot_aidrc_mode mode;
} sample_aidrc_param;

td_s32 sample_aidrc(sample_aidrc_param *aidrc_param);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
