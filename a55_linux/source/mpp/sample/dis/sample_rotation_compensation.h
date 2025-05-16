/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_ROTATION_COMPENSATION_H
#define SAMPLE_ROTATION_COMPENSATION_H

#include "ot_type.h"
#include "ot_common_motionsensor.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

td_s32 sample_motionfuson_start_rotation_compensation(ot_msensor_param *msensor_param);
td_s32 sample_motionfuson_stop_rotation_compensation(td_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of __cplusplus */

#endif /* SAMPLE_ROTATION_COMPENSATION_H */

